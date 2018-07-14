/*
 * Autor: Nico Korn
 * Date: 15.05.2018
 * Firmware for a alarmlcock with custom made STM32F103 microcontroller board.
 *  *
 * Copyright (c) 2018 Nico Korn
 *
 * ws2812.c this module contents the initialization of the ws2812b leds including
 * several functions.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// ----------------------------------------------------------------------------
#include <lightsensor.h>
#include "ws2812.h"
#include "stm32f1xx.h"
#include <Math.h>
#include <stdio.h>
#include <stdlib.h>

/* defines */
/* this define sets the number of TIM2 overflows
 * to append to the data frame for the LEDs to
 * load the received data into their registers */
#define WS2812_DEADPERIOD 		19
/* WS2812 GPIO output buffer size */
#define GPIO_BUFFERSIZE 		COL*24
/* background RGB color buffer size */
#define BACKGROUND_BUFFERSIZE 	ROW*COL*3
/* global variables */
uint8_t 				WS2812_TC;												//global scope: used in the main routine
/* private variables */
static uint8_t 			TIM2_overflows = 0;
static uint8_t			init = 0;
static uint8_t			clock_background_framebuffer[BACKGROUND_BUFFERSIZE];	//11 rows * 11 cols * 3 (RGB) = 363 --- separate frame buffer for background fx --- 1 array entry contents a color component information in 8 bit. 3 entries together = 1 RGB Information
static uint16_t 		WS2812_IO_High = 0xFFFF;
static uint16_t 		WS2812_IO_Low = 0x0000;
static uint16_t 		WS2812_IO_framedata[GPIO_BUFFERSIZE];					// 11 cols * 24 bits (R(8bit), G(8bit), B(8bit)) = 266 --- output array transferred to GPIO output --- 1 array entry contents 16 bits parallel to GPIO output
/* typedefs */
TIM_HandleTypeDef 		TIM2_Handle;
DMA_HandleTypeDef 		DMA_HandleStruct_UEV;
DMA_HandleTypeDef 		DMA_HandleStruct_CC1;
DMA_HandleTypeDef 		DMA_HandleStruct_CC2;

/**
  * @brief  initialization of peripherals used for ws2812 leds
  * @note   None
  * @retval None
  */
void init_ws2812(void){
	/* init peripherals */
	init_gpio();
	init_dma();
	init_timer();

	/* set transmission flag to 1 */
	WS2812_TC = 1;
}

/**
  * @brief  initialization of hw timer for the serial data output
  * @note   None
  * @retval None
  */
void init_timer(void){
	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_OC_InitTypeDef TIM_OC1Struct;
	TIM_OC_InitTypeDef TIM_OC2Struct;
	uint16_t PrescalerValue;

	/* TIM2 Periph clock enable */
	__HAL_RCC_TIM2_CLK_ENABLE();

	/* set prescaler to get a 24 MHz clock signal */
	PrescalerValue = (uint16_t) (SystemCoreClock / 24000000) - 1;

	/* Time base configuration */
	TIM2_Handle.Instance = TIM2;
	TIM2_Handle.Init.Period = 29;	// set the period to get 29 to get a 800kHz timer
	TIM2_Handle.Init.Prescaler = PrescalerValue;
	TIM2_Handle.Init.ClockDivision = 0;
	TIM2_Handle.Init.CounterMode = TIM_COUNTERMODE_UP;
	HAL_TIM_Base_Init(&TIM2_Handle);
    /* Reset the ARR Preload Bit */
	TIM2->CR1 &= (uint16_t)~TIM_CR1_ARPE;

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&TIM2_Handle, &sClockSourceConfig);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&TIM2_Handle, &sMasterConfig);

	/* Timing Mode configuration: Channel 1 */
	TIM_OC1Struct.OCMode = TIM_OCMODE_TIMING;
	TIM_OC1Struct.OCPolarity = TIM_OCPOLARITY_HIGH;
	TIM_OC1Struct.Pulse = 8;
	/* Configure the channel */
	HAL_TIM_OC_ConfigChannel(&TIM2_Handle, &TIM_OC1Struct, TIM_CHANNEL_1);

	/* Timing Mode configuration: Channel 2 */
	TIM_OC2Struct.OCMode = TIM_OCMODE_PWM1;
	TIM_OC2Struct.Pulse = 17;
	TIM_OC2Struct.OCPolarity = TIM_OCPOLARITY_HIGH;
	TIM_OC2Struct.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	TIM_OC2Struct.OCIdleState = TIM_OCIDLESTATE_SET;
	TIM_OC2Struct.OCFastMode = TIM_OCFAST_ENABLE;
	TIM_OC2Struct.OCIdleState = TIM_CCx_ENABLE;
	/* Configure the channel */
	HAL_TIM_PWM_ConfigChannel(&TIM2_Handle, &TIM_OC2Struct, TIM_CHANNEL_2);

	/* configure TIM2 interrupt */
	HAL_NVIC_SetPriority(TIM2_IRQn, 0, 2);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

/**
  * @brief  initialization of GPIO pins as outputs for the led stripes
  * @note   None
  * @retval None
  */
void init_gpio(void){
	__HAL_RCC_GPIOA_CLK_ENABLE();						//enable clock on the bus
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = (uint16_t)0x07FFU; 			// select all 11 Pins => 11 led rows 0x07FFU;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 		// configure pins for pp output
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;		// 50 MHz rate
	GPIO_InitStruct.Pull = GPIO_NOPULL;					// this activates the pullup resistors on the IO pins
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);				// setting GPIO registers
}

/**
  * @brief  initialization of DMA controller
  * @note   None
  * @retval None
  */
void init_dma(void){
	/* activate bus on which dma1 is connected */
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* TIM2 Update event, High Output */
	/* DMA1 Channel2 configuration ----------------------------------------------*/
	DMA_HandleStruct_UEV.Instance 					= DMA1_Channel2;
	DMA_HandleStruct_UEV.Init.Direction 			= DMA_MEMORY_TO_PERIPH;
	DMA_HandleStruct_UEV.Init.PeriphInc 			= DMA_PINC_DISABLE;
	DMA_HandleStruct_UEV.Init.MemInc 				= DMA_MINC_DISABLE;
	DMA_HandleStruct_UEV.Init.Mode 					= DMA_NORMAL;
	DMA_HandleStruct_UEV.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_HALFWORD;
	DMA_HandleStruct_UEV.Init.MemDataAlignment 		= DMA_MDATAALIGN_HALFWORD;
	DMA_HandleStruct_UEV.Init.Priority 				= DMA_PRIORITY_HIGH;
	HAL_DMA_DeInit(&DMA_HandleStruct_UEV);
	DMA_SetConfiguration(&DMA_HandleStruct_UEV, (uint32_t)WS2812_IO_High, (uint32_t)&GPIOA->ODR, GPIO_BUFFERSIZE);
	if(HAL_DMA_Init(&DMA_HandleStruct_UEV) != HAL_OK){
		while(1){
			//error
		}
	}

	/* TIM2 CC1 event, Dataframe Output, needs bit incrementation on memory */
	/* DMA1 Channel5 configuration ----------------------------------------------*/
	DMA_HandleStruct_CC1.Instance 					= DMA1_Channel5;
	DMA_HandleStruct_CC1.Init.Direction 			= DMA_MEMORY_TO_PERIPH;
	DMA_HandleStruct_CC1.Init.PeriphInc 			= DMA_PINC_DISABLE;
	DMA_HandleStruct_CC1.Init.MemInc 				= DMA_MINC_ENABLE;
	DMA_HandleStruct_CC1.Init.Mode 					= DMA_NORMAL;
	DMA_HandleStruct_CC1.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_HALFWORD;
	DMA_HandleStruct_CC1.Init.MemDataAlignment 		= DMA_MDATAALIGN_HALFWORD;
	DMA_HandleStruct_CC1.Init.Priority 				= DMA_PRIORITY_HIGH;
	HAL_DMA_DeInit(&DMA_HandleStruct_CC1);
	DMA_SetConfiguration(&DMA_HandleStruct_CC1, (uint32_t)WS2812_IO_framedata, (uint32_t)&GPIOA->ODR, GPIO_BUFFERSIZE);
	if(HAL_DMA_Init(&DMA_HandleStruct_CC1) != HAL_OK){
		while(1){
			//error
		}
	}

	/* TIM2 CC2 event, Low Output */
	/* DMA1 Channel7 configuration ----------------------------------------------*/
	DMA_HandleStruct_CC2.Instance 					= DMA1_Channel7;
	DMA_HandleStruct_CC2.Init.Direction 			= DMA_MEMORY_TO_PERIPH;
	DMA_HandleStruct_CC2.Init.PeriphInc 			= DMA_PINC_DISABLE;
	DMA_HandleStruct_CC2.Init.MemInc 				= DMA_MINC_DISABLE;
	DMA_HandleStruct_CC2.Init.Mode 					= DMA_NORMAL;
	DMA_HandleStruct_CC2.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_HALFWORD;
	DMA_HandleStruct_CC2.Init.MemDataAlignment 		= DMA_MDATAALIGN_HALFWORD;
	DMA_HandleStruct_CC2.Init.Priority 				= DMA_PRIORITY_HIGH;
	HAL_DMA_DeInit(&DMA_HandleStruct_CC2);
	DMA_SetConfiguration(&DMA_HandleStruct_CC2, (uint32_t)WS2812_IO_Low, (uint32_t)&GPIOA->ODR, GPIO_BUFFERSIZE);
	if(HAL_DMA_Init(&DMA_HandleStruct_CC2) != HAL_OK){
		while(1){
			//error
		}
	}

	/* register callbacks */
	HAL_DMA_RegisterCallback(&DMA_HandleStruct_CC2, HAL_DMA_XFER_CPLT_CB_ID, TransferComplete);
	HAL_DMA_RegisterCallback(&DMA_HandleStruct_CC2, HAL_DMA_XFER_ERROR_CB_ID, TransferError);

	/* NVIC configuration for DMA transfer complete interrupt */
	HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 1);
	/* Enable interrupt */
	HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);
}

/**
  * @brief  start serial data transmission to the led's
  * @note   None
  * @retval None
  */
void sendbuf_WS2812(){
	/* transmission complete flag, indicate that transmission is taking place */
	WS2812_TC = 0;

	/* set configuration */
	DMA_SetConfiguration(&DMA_HandleStruct_UEV, (uint32_t)WS2812_IO_High, (uint32_t)&GPIOA->ODR, GPIO_BUFFERSIZE);
	DMA_SetConfiguration(&DMA_HandleStruct_CC1, (uint32_t)WS2812_IO_framedata, (uint32_t)&GPIOA->ODR, GPIO_BUFFERSIZE);
	DMA_SetConfiguration(&DMA_HandleStruct_CC2, (uint32_t)WS2812_IO_Low, (uint32_t)&GPIOA->ODR, GPIO_BUFFERSIZE);

	/* clear all relevant DMA flags from the channels 2,5 and 7 */
	__HAL_DMA_CLEAR_FLAG(&DMA_HandleStruct_UEV, DMA_FLAG_TC2 | DMA_FLAG_HT2 | DMA_FLAG_TE2 | DMA_FLAG_GL2);
	__HAL_DMA_CLEAR_FLAG(&DMA_HandleStruct_CC1, DMA_FLAG_TC5 | DMA_FLAG_HT5 | DMA_FLAG_TE5 | DMA_FLAG_GL5);
	__HAL_DMA_CLEAR_FLAG(&DMA_HandleStruct_CC2, DMA_FLAG_TC7 | DMA_FLAG_HT7 | DMA_FLAG_TE7 | DMA_FLAG_GL7);

    /* Enable the selected DMA transfer interrupts */
	/*
	DMA_HandleStruct_CC2.Instance->CCR  |= DMA_IT_TC | DMA_IT_TE;
	//DMA_HandleStruct_CC2.Instance->FCR |= DMA_IT_FE;
    if(DMA_HandleStruct_CC2.XferHalfCpltCallback != NULL)
    {
    	DMA_HandleStruct_CC2.Instance->CCR  |= DMA_IT_HT;
    }
    */
	__HAL_DMA_ENABLE_IT(&DMA_HandleStruct_CC2, (DMA_IT_TC | DMA_IT_HT | DMA_IT_TE));

	/* enable dma channels */
	__HAL_DMA_ENABLE(&DMA_HandleStruct_UEV);
	__HAL_DMA_ENABLE(&DMA_HandleStruct_CC1);
	__HAL_DMA_ENABLE(&DMA_HandleStruct_CC2);

	/* clear all TIM2 flags */
	TIM2->SR = 0;

	/* IMPORTANT: enable the TIM2 DMA requests AFTER enabling the DMA channels! */
	__HAL_TIM_ENABLE_DMA(&TIM2_Handle, TIM_DMA_UPDATE);
	__HAL_TIM_ENABLE_DMA(&TIM2_Handle, TIM_DMA_CC1);
	__HAL_TIM_ENABLE_DMA(&TIM2_Handle, TIM_DMA_CC2);

	/* Enable the Output compare channel */
	TIM_CCxChannelCmd(TIM2, TIM_CHANNEL_1, TIM_CCx_ENABLE);
	TIM_CCxChannelCmd(TIM2, TIM_CHANNEL_2, TIM_CCx_ENABLE);

	/* preload counter with 29 so TIM2 generates UEV directly to start DMA transfer */
	__HAL_TIM_SET_COUNTER(&TIM2_Handle, 29);

	/* start TIM2 */
	__HAL_TIM_ENABLE(&TIM2_Handle);
}

/* DMA1 Channel2 Interrupt Handler gets executed once the complete frame buffer
 * has been transmitted to the LEDs */
void DMA1_Channel7_IRQHandler(void){
	/* set irq handler */
	HAL_DMA_IRQHandler(&DMA_HandleStruct_CC2);
}

/**
  * @brief  TIM2 Interrupt Callback Handler
  * @note   TIM2 Interrupt Handler gets executed on every TIM1 Update if enabled
  * @retval None
  */
void WS2812_TIM2_callback(void){
	/* Clear TIM2 Interrupt Flag */
	HAL_NVIC_ClearPendingIRQ(TIM2_IRQn);

	/* check if certain number of overflows has occured yet
	 * this ISR is used to guarantee a 19*1.25 us = 23.75 us dead time on the data lines (measured 28.8 us)
	 * before another frame is transmitted */
	if (TIM2_overflows < (uint8_t)WS2812_DEADPERIOD){
		/* count the number of occured overflows */
		TIM2_overflows++;
	}else{
		/* clear the number of overflows */
		TIM2_overflows = 0;

		/* stop TIM2 now because dead period has been reached */
		TIM_CCxChannelCmd(TIM2, TIM_CHANNEL_1, TIM_CCx_DISABLE);
		TIM_CCxChannelCmd(TIM2, TIM_CHANNEL_2, TIM_CCx_DISABLE);
		__HAL_TIM_DISABLE(&TIM2_Handle);

		/* disable the TIM2 Update interrupt again so it doesn't occur while transmitting data */
		__HAL_TIM_DISABLE_IT(&TIM2_Handle, TIM_IT_UPDATE);

		/* finally indicate that the data frame has been transmitted */
		WS2812_TC = 1;
	}
}

/**
  * @brief  Sets the DMA Transfer parameter.
  * @param  hdma:       pointer to a DMA_HandleTypeDef structure that contains
  *                     the configuration information for the specified DMA Stream.
  * @param  SrcAddress: The source memory Buffer address
  * @param  DstAddress: The destination memory Buffer address
  * @param  DataLength: The length of data to be transferred from source to destination
  * @retval HAL status
  */
void DMA_SetConfiguration(DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength){
	/* Clear all flags */
	hdma->DmaBaseAddress->IFCR = (DMA_ISR_GIF1 << hdma->ChannelIndex);

	/* Configure DMA Channel data length */
	hdma->Instance->CNDTR = DataLength;

	/* Memory to Peripheral */
	if((hdma->Init.Direction) == DMA_MEMORY_TO_PERIPH){
		/* Configure DMA Channel destination address */
		hdma->Instance->CPAR = DstAddress;

	    /* Configure DMA Channel source address */
	    hdma->Instance->CMAR = SrcAddress;
	}
	/* Peripheral to Memory */
	else
	{
	    /* Configure DMA Channel source address */
	    hdma->Instance->CPAR = SrcAddress;

	    /* Configure DMA Channel destination address */
	    hdma->Instance->CMAR = DstAddress;
	}
}

/**
  * @brief  DMA conversion complete callback
  * @note   This function is executed when the transfer complete interrupt
  *         is generated
  * @retval None
  */
void TransferComplete(DMA_HandleTypeDef *DmaHandle){
	/* clear DMA7 transfer complete interrupt flag */
	HAL_NVIC_ClearPendingIRQ(DMA1_Channel7_IRQn);

	/* enable TIM2 Update interrupt to append 50us dead period */
	__HAL_TIM_ENABLE_IT(&TIM2_Handle, TIM_IT_UPDATE);

	/* disable the DMA channels */
	__HAL_DMA_DISABLE(&DMA_HandleStruct_UEV);
	__HAL_DMA_DISABLE(&DMA_HandleStruct_CC1);
	__HAL_DMA_DISABLE(&DMA_HandleStruct_CC2);

	/* IMPORTANT: disable the DMA requests, too! */
	__HAL_TIM_DISABLE_DMA(&TIM2_Handle, TIM_DMA_UPDATE);
	__HAL_TIM_DISABLE_DMA(&TIM2_Handle, TIM_DMA_CC1);
	__HAL_TIM_DISABLE_DMA(&TIM2_Handle, TIM_DMA_CC2);
}

/**
  * @brief  DMA conversion error callback
  * @note   This function is executed when the transfer error interrupt
  *         is generated during DMA transfer
  * @retval None
  */
void TransferError(DMA_HandleTypeDef *DmaHandle){
	while(1){
		//do, nothing stay here
	}
}


/* This function sets the color of a single pixel in the clock_background_framebuffer
 *
 * Arguments:
 * row = the channel number/LED strip the pixel is in from 0 to 15
 * column = the column/LED position in the LED string from 0 to number of LEDs per strip
 * red, green, blue = the RGB color triplet that the pixel should display
 */
void WS2812_framedata_setPixel(uint8_t row, uint16_t column, uint8_t red, uint8_t green, uint8_t blue){

	uint8_t i;

	for (i = 0; i < 8; i++){
		/* clear the data for pixel */
		WS2812_IO_framedata[((column*24)+i)] &= ~(0x01<<row);
		WS2812_IO_framedata[((column*24)+8+i)] &= ~(0x01<<row);
		WS2812_IO_framedata[((column*24)+16+i)] &= ~(0x01<<row);

		/* write new data for pixel */
		WS2812_IO_framedata[((column*24)+i)] |= ((((green<<i) & 0x80)>>7)<<row);
		WS2812_IO_framedata[((column*24)+8+i)] |= ((((red<<i) & 0x80)>>7)<<row);
		WS2812_IO_framedata[((column*24)+16+i)] |= ((((blue<<i) & 0x80)>>7)<<row);
	}
}

/* This function clears the ws2812 color buffer
 *
 * Arguments:
 * none
 */
void WS2812_clear_buffer(){
	/* clear frame buffer */
	for(uint8_t y=0; y<ROW;y++){
		for(uint16_t x=0; x<COL; x++){
			WS2812_framedata_setPixel(y, x, 0x00, 0x00, 0x00);
		}
	}
}

/* This function sets a line with start point and end point
 *
 * Arguments:
 * row_start, column_start = start point of the line
 * row_end, column_end = end point of the line
 */
void WS2812_set_line(int8_t row_start, int16_t column_start, int8_t row_end, int16_t column_end, uint8_t red, uint8_t green, uint8_t blue){
	uint16_t offset_x = (uint16_t)column_start;
	uint16_t offset_y = (uint8_t)row_start;
	double x_delta;
	double y_delta;
	double m;
	double q;
	double y_calc;
	double x_calc;

	/* the smaller coordination value shall be chosen as offset */
	if(column_start > column_end){
		offset_x = column_end;
	}
	if(row_start > row_end){
		offset_y = row_end;
	}

	/* calculate deltas */
	x_delta = column_end-column_start;
	y_delta = row_end-row_start;

	/* calculate function pitch f(x) */
	if(x_delta == 0 || y_delta == 0){
		m = 0;
	}else{
		m = y_delta / x_delta;
	}

	/* y value on x = 0: y = m*x + q */
	q = (double)row_start - (double)column_start*m;

	/* draw the line with the function f(x) */
	for(uint16_t x=0; x<abs(x_delta)+1; x++){
		y_calc = m*((double)offset_x+(double)x)+q;
		y_calc = round(y_calc);
		WS2812_framedata_setPixel((uint8_t)y_calc, offset_x+x, red, green, blue);
	}

	/* calculate function pitch f(y) */
	if(x_delta == 0 || y_delta == 0){
		m = 0;
	}else{
		m = x_delta / y_delta;
	}

	/* draw the line with the function f(y) */
	for(uint8_t y=0; y<abs(y_delta)+1; y++){
		x_calc = m*((double)y-q);
		x_calc = round(x_calc);
		WS2812_framedata_setPixel(offset_y+y, (uint16_t)x_calc, red, green, blue);
	}
}

/**
  * @brief  led color wheel function
  * @note   this function is used to switch properly through colors
  * @retval color in rgb
  */
void WS2812_color_wheel_plus(uint8_t *red, uint8_t *green, uint8_t *blue){
	//set next colors
	if(*green == 0x00 && *red < 0xff && *blue >= 0x00){
		*red += 0x01;
		*green = 0x00;
		*blue -= 0x01;
	}else if(*green < 0xff && *red >= 0x00 && *blue == 0x00){
		*red -= 0x01;
		*green += 0x01;
		*blue = 0x00;
	}else if(*green >= 0x00 && *red == 0x00 && *blue < 0xff){
		*red = 0x00;
		*green -= 0x01;
		*blue += 0x01;
	}
}

/**
  * @brief  led color wheel function
  * @note   this function is used to switch properly through colors
  * @retval color in rgb
  */
void WS2812_color_wheel_minus(uint8_t *red, uint8_t *green, uint8_t *blue){
	//set next colors
	if(*green == 0x00 && *red >= 0x00 && *blue < 0xff){
		*red -= 0x01;
		*green = 0x00;
		*blue += 0x01;
	}else if(*green >= 0x00 && *red < 0xff && *blue == 0x00){
		*red += 0x01;
		*green -= 0x01;
		*blue = 0x00;
	}else if(*green < 0xff && *red == 0x00 && *blue >= 0x00){
		*red = 0x00;
		*green += 0x01;
		*blue -= 0x01;
	}
}

/**
  * @brief  led test
  * @note   this function is used to test the rgb leds on all colors
  * @retval -
  */
void WS2812_led_test(){
	uint8_t redtest = 0xff;
	uint8_t greentest = 0x00;
	uint8_t bluetest = 0x00;

	for(uint16_t i = 0; i<380; i++){
		while(!WS2812_TC);
		/* fill the complete buffer at first round */
		if(i == 0){
			for(uint8_t y = 0; y<ROW; y++){
				/* set the color with the color wheel function */
				for(uint8_t s=0; s<20; s++){
					WS2812_color_wheel_plus(&redtest, &greentest, &bluetest);
				}
				for(uint16_t x = 0; x<COL; x++){
					/* fill buffer for first frame */
					clock_background_framebuffer[(y*COL*3)+(x*3)] = redtest;
					clock_background_framebuffer[(y*COL*3)+(x*3)+1] = greentest;
					clock_background_framebuffer[(y*COL*3)+(x*3)+2] = bluetest;
				}
			}
		}else{
			/* set the color with the color wheel function */
			for(uint8_t s=0; s<15; s++){
				WS2812_color_wheel_plus(&redtest, &greentest, &bluetest);
			}
			/* shift 1 row up */
			for(uint16_t j=0; j<ROW; j++){
				for(uint16_t s=0; s<3*COL; s++){
					clock_background_framebuffer[j*COL*3+s] = clock_background_framebuffer[(j+1)*COL*3+s];
				}
			}
			/* write new color in bottom row */
			for(uint16_t j=0; j<COL; j++){
				clock_background_framebuffer[((ROW-1)*COL*3)+(j*3)] = redtest;
				clock_background_framebuffer[((ROW-1)*COL*3)+(j*3)+1] = greentest;
				clock_background_framebuffer[((ROW-1)*COL*3)+(j*3)+2] = bluetest;
			}
		}
		/* write buffer into buffer... lol */
		for(uint8_t y = 0; y<ROW; y++){
			for(uint16_t x = 0; x<COL; x++){
				WS2812_framedata_setPixel(y, x, (uint8_t)clock_background_framebuffer[(y*COL*3)+(x*3)], (uint8_t)clock_background_framebuffer[(y*COL*3)+(x*3)+1], (uint8_t)clock_background_framebuffer[(y*COL*3)+(x*3)+2]);
			}
		}
		sendbuf_WS2812();
		HAL_Delay(10);
	}
}

/**
  * @brief  matrix effect background
  * @note   this function is used to test the rgb leds on all colors
  * @retval -
  */
void WS2812_background_matrix(){
	uint16_t drops[3];
	uint16_t row_width = 11;

	/* init clock_background_framebuffer */
	if(init == 0){
		init = 1;
		for(uint8_t y = 0; y<ROW-1; y++){
			for(uint16_t x = 0; x<COL; x++){
				/* fill complete buffer */
				clock_background_framebuffer[(y*11*3)+(x*3)] = 0x00;	//red
				clock_background_framebuffer[(y*11*3)+(x*3)+1] = 0x00;	//green
				clock_background_framebuffer[(y*11*3)+(x*3)+2] = 0x00;	//blue
			}
		}
	}

	/* frame buffer shift down */
	/* shift 1 row down */
	for(uint16_t row=9; row>0; row--){
		for(uint16_t s=0; s<33; s++){
			clock_background_framebuffer[row*row_width*3+s] = clock_background_framebuffer[(row-1)*row_width*3+s];
		}
	}

	/* set drops with random numbers */
	for(int i = 0; i<3; i++){
		drops[i] = (rand() % 11);
	}

	/* write top row into frame buffer with drops*/
	for(uint16_t x = 0; x<COL; x++){
		for(uint8_t i = 0; i<3; i++){
			if(x == drops[i]){
				clock_background_framebuffer[3*x] = 0x00;
				clock_background_framebuffer[3*x+1] = rand() & 0x33;
				clock_background_framebuffer[3*x+2] = 0x00;
			}
		}
	}
	/* write buffer into buffer... lol */
	for(uint8_t y = 0; y<ROW-1; y++){
		for(uint16_t x = 0; x<COL; x++){
			WS2812_framedata_setPixel(y, x, (uint8_t)clock_background_framebuffer[(y*11*3)+(x*3)], (uint8_t)clock_background_framebuffer[(y*11*3)+(x*3)+1], (uint8_t)clock_background_framebuffer[(y*11*3)+(x*3)+2]);
		}
	}
}

/**
  * @brief  equalizer effect background
  * @note   this function is used to test the rgb leds on all colors
  * @retval -
  */
void WS2812_background_equalizer(){
	/* variables */
	uint8_t hex_value;
	double hex_value_float;
	double m = 0.128; //0.128
	uint32_t adc_value;
	uint32_t adc_value_temp;

	/* start adc and read conversion */
	start_microphone_adc_conversion();
	//for(uint i=0; i<10;i++){
		get_microphone_adc_conversion(&adc_value_temp);
		//adc_value += adc_value_temp;
	//}
	//stop_microphone_adc_conversion();

	//adc_value /= 10;
	adc_value = adc_value_temp;

	/* convert conversion into hex value 0x00 - 0xFF */

	if(adc_value >= 2000 && adc_value < 4000){
		hex_value_float = m*(double)adc_value-256;
		hex_value = (uint8_t)hex_value_float;
	}else if(adc_value >= 4000){
		hex_value = 0xff;
	}else{
		hex_value = 0x00;
	}

	/* set background color according to adc conversion */
	for(uint8_t y = 0; y < 11; y++){
		for(uint16_t x = 0; x < 11; x++){
			WS2812_framedata_setPixel(y, x, hex_value, 0x00, 0x00);
		}
	}
}

/**
  * @brief  sets the background effect
  * @note   input is a number which is used to choose the background effect in a switch case
  * @retval -
  */
WS2812_set_clock_fx(uint8_t *fx_mode, uint32_t *counter, uint8_t *red, uint8_t *green, uint8_t *blue){
	/* set background/fx mode */
	switch(*fx_mode){
		case 0: WS2812_clear_buffer();
		break;
		case 1:	if(*counter%50 == 0 || *counter == 0){
					WS2812_clear_buffer();
					WS2812_background_matrix();
				}
		break;
		case 2:	WS2812_clear_buffer();
				WS2812_background_equalizer();
		break;
		case 3:	if(*counter%50 == 0){
					WS2812_clear_buffer();
					WS2812_color_wheel_plus(red,green,blue);
				}
		break;
	}
}
