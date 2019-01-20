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
uint8_t 					WS2812_TC;												//global scope: used in the main routine
/* private variables */
static uint8_t				stop_flag = 0;
static uint8_t 				TIM2_overflows = 0;
static uint8_t				init = 0;
static uint8_t				clock_background_framebuffer[BACKGROUND_BUFFERSIZE];	//11 rows * 11 cols * 3 (RGB) = 363 --- separate frame buffer for background fx --- 1 array entry contents a color component information in 8 bit. 3 entries together = 1 RGB Information
static uint16_t 			WS2812_IO_High = 0xFFFF;
static uint16_t 			WS2812_IO_Low = 0x0000;
static uint16_t 			WS2812_IO_framedata[GPIO_BUFFERSIZE];					// 11 cols * 24 bits (R(8bit), G(8bit), B(8bit)) = 266 --- output array transferred to GPIO output --- 1 array entry contents 16 bits parallel to GPIO output
/* private numbers and letters */
static Number 				zero;
static Number 				one;
static Number 				two;
static Number 				three;
static Number 				four;
static Number 				five;
static Number 				six;
static Number 				seven;
static Number 				eight;
static Number 				nine;
static Number 				doublepoint;
static Letter 				m;
static Letter 				w;
static Letter 				a;
static Letter 				d;
static Letter 				e;
static Letter				f;
static Letter				h;
static Letter 				i;
static Letter				l;
static Letter				r;
static Letter				s;
static Letter 				t;
static Letter 				u;
static Letter 				p;
static Letter				c;
static Letter				o;
static Letter				k;
static Letter				n;
static Letter				_;
static Letter				z;
static Letter				x;
static Letter 				nzero;
static Letter 				none;
static Letter 				ntwo;
static Letter 				nthree;
static Letter 				nfour;
static Letter 				nfive;
static Letter 				nsix;
static Letter 				nseven;
static Letter 				neight;
static Letter 				nnine;

/* typedefs */
TIM_HandleTypeDef 			TIM2_Handle;
DMA_HandleTypeDef 			DMA_HandleStruct_UEV;
DMA_HandleTypeDef 			DMA_HandleStruct_CC1;
DMA_HandleTypeDef 			DMA_HandleStruct_CC2;

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
	init_font();

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

/**
  * @brief  initialization of letter and number data
  * @note   None
  * @retval None
  */
void init_font(void){
	/* construct the numbers
	 * 1 = set pixel
	 * 0 = empty pixel
	 */
	/* number zero */
	zero.number_construction[0][0] = 1;
	zero.number_construction[1][0] = 1;
	zero.number_construction[2][0] = 1;

	zero.number_construction[0][1] = 1;
	zero.number_construction[1][1] = 0;
	zero.number_construction[2][1] = 1;

	zero.number_construction[0][2] = 1;
	zero.number_construction[1][2] = 0;
	zero.number_construction[2][2] = 1;

	zero.number_construction[0][3] = 1;
	zero.number_construction[1][3] = 0;
	zero.number_construction[2][3] = 1;

	zero.number_construction[0][4] = 1;
	zero.number_construction[1][4] = 0;
	zero.number_construction[2][4] = 1;

	zero.number_construction[0][5] = 1;
	zero.number_construction[1][5] = 0;
	zero.number_construction[2][5] = 1;

	zero.number_construction[0][6] = 1;
	zero.number_construction[1][6] = 1;
	zero.number_construction[2][6] = 1;

	/* number one */
	one.number_construction[0][0] = 0;
	one.number_construction[1][0] = 0;
	one.number_construction[2][0] = 1;

	one.number_construction[0][1] = 0;
	one.number_construction[1][1] = 0;
	one.number_construction[2][1] = 1;

	one.number_construction[0][2] = 0;
	one.number_construction[1][2] = 0;
	one.number_construction[2][2] = 1;

	one.number_construction[0][3] = 0;
	one.number_construction[1][3] = 0;
	one.number_construction[2][3] = 1;

	one.number_construction[0][4] = 0;
	one.number_construction[1][4] = 0;
	one.number_construction[2][4] = 1;

	one.number_construction[0][5] = 0;
	one.number_construction[1][5] = 0;
	one.number_construction[2][5] = 1;

	one.number_construction[0][6] = 0;
	one.number_construction[1][6] = 0;
	one.number_construction[2][6] = 1;

	/* number two */
	two.number_construction[0][0] = 1;
	two.number_construction[1][0] = 1;
	two.number_construction[2][0] = 1;

	two.number_construction[0][1] = 0;
	two.number_construction[1][1] = 0;
	two.number_construction[2][1] = 1;

	two.number_construction[0][2] = 0;
	two.number_construction[1][2] = 0;
	two.number_construction[2][2] = 1;

	two.number_construction[0][3] = 1;
	two.number_construction[1][3] = 1;
	two.number_construction[2][3] = 1;

	two.number_construction[0][4] = 1;
	two.number_construction[1][4] = 0;
	two.number_construction[2][4] = 0;

	two.number_construction[0][5] = 1;
	two.number_construction[1][5] = 0;
	two.number_construction[2][5] = 0;

	two.number_construction[0][6] = 1;
	two.number_construction[1][6] = 1;
	two.number_construction[2][6] = 1;

	/* number three */
	three.number_construction[0][0] = 1;
	three.number_construction[1][0] = 1;
	three.number_construction[2][0] = 1;

	three.number_construction[0][1] = 0;
	three.number_construction[1][1] = 0;
	three.number_construction[2][1] = 1;

	three.number_construction[0][2] = 0;
	three.number_construction[1][2] = 0;
	three.number_construction[2][2] = 1;

	three.number_construction[0][3] = 1;
	three.number_construction[1][3] = 1;
	three.number_construction[2][3] = 1;

	three.number_construction[0][4] = 0;
	three.number_construction[1][4] = 0;
	three.number_construction[2][4] = 1;

	three.number_construction[0][5] = 0;
	three.number_construction[1][5] = 0;
	three.number_construction[2][5] = 1;

	three.number_construction[0][6] = 1;
	three.number_construction[1][6] = 1;
	three.number_construction[2][6] = 1;

	/* number four */
	four.number_construction[0][0] = 1;
	four.number_construction[1][0] = 0;
	four.number_construction[2][0] = 1;

	four.number_construction[0][1] = 1;
	four.number_construction[1][1] = 0;
	four.number_construction[2][1] = 1;

	four.number_construction[0][2] = 1;
	four.number_construction[1][2] = 0;
	four.number_construction[2][2] = 1;

	four.number_construction[0][3] = 1;
	four.number_construction[1][3] = 1;
	four.number_construction[2][3] = 1;

	four.number_construction[0][4] = 0;
	four.number_construction[1][4] = 0;
	four.number_construction[2][4] = 1;

	four.number_construction[0][5] = 0;
	four.number_construction[1][5] = 0;
	four.number_construction[2][5] = 1;

	four.number_construction[0][6] = 0;
	four.number_construction[1][6] = 0;
	four.number_construction[2][6] = 1;

	/* number five */
	five.number_construction[0][0] = 1;
	five.number_construction[1][0] = 1;
	five.number_construction[2][0] = 1;

	five.number_construction[0][1] = 1;
	five.number_construction[1][1] = 0;
	five.number_construction[2][1] = 0;

	five.number_construction[0][2] = 1;
	five.number_construction[1][2] = 0;
	five.number_construction[2][2] = 0;

	five.number_construction[0][3] = 1;
	five.number_construction[1][3] = 1;
	five.number_construction[2][3] = 1;

	five.number_construction[0][4] = 0;
	five.number_construction[1][4] = 0;
	five.number_construction[2][4] = 1;

	five.number_construction[0][5] = 0;
	five.number_construction[1][5] = 0;
	five.number_construction[2][5] = 1;

	five.number_construction[0][6] = 1;
	five.number_construction[1][6] = 1;
	five.number_construction[2][6] = 1;

	/* number six */
	six.number_construction[0][0] = 1;
	six.number_construction[1][0] = 1;
	six.number_construction[2][0] = 1;

	six.number_construction[0][1] = 1;
	six.number_construction[1][1] = 0;
	six.number_construction[2][1] = 0;

	six.number_construction[0][2] = 1;
	six.number_construction[1][2] = 0;
	six.number_construction[2][2] = 0;

	six.number_construction[0][3] = 1;
	six.number_construction[1][3] = 1;
	six.number_construction[2][3] = 1;

	six.number_construction[0][4] = 1;
	six.number_construction[1][4] = 0;
	six.number_construction[2][4] = 1;

	six.number_construction[0][5] = 1;
	six.number_construction[1][5] = 0;
	six.number_construction[2][5] = 1;

	six.number_construction[0][6] = 1;
	six.number_construction[1][6] = 1;
	six.number_construction[2][6] = 1;

	/* number seven */
	seven.number_construction[0][0] = 1;
	seven.number_construction[1][0] = 1;
	seven.number_construction[2][0] = 1;

	seven.number_construction[0][1] = 0;
	seven.number_construction[1][1] = 0;
	seven.number_construction[2][1] = 1;

	seven.number_construction[0][2] = 0;
	seven.number_construction[1][2] = 0;
	seven.number_construction[2][2] = 1;

	seven.number_construction[0][3] = 0;
	seven.number_construction[1][3] = 0;
	seven.number_construction[2][3] = 1;

	seven.number_construction[0][4] = 0;
	seven.number_construction[1][4] = 0;
	seven.number_construction[2][4] = 1;

	seven.number_construction[0][5] = 0;
	seven.number_construction[1][5] = 0;
	seven.number_construction[2][5] = 1;

	seven.number_construction[0][6] = 0;
	seven.number_construction[1][6] = 0;
	seven.number_construction[2][6] = 1;

	/* number eight */
	eight.number_construction[0][0] = 1;
	eight.number_construction[1][0] = 1;
	eight.number_construction[2][0] = 1;

	eight.number_construction[0][1] = 1;
	eight.number_construction[1][1] = 0;
	eight.number_construction[2][1] = 1;

	eight.number_construction[0][2] = 1;
	eight.number_construction[1][2] = 0;
	eight.number_construction[2][2] = 1;

	eight.number_construction[0][3] = 1;
	eight.number_construction[1][3] = 1;
	eight.number_construction[2][3] = 1;

	eight.number_construction[0][4] = 1;
	eight.number_construction[1][4] = 0;
	eight.number_construction[2][4] = 1;

	eight.number_construction[0][5] = 1;
	eight.number_construction[1][5] = 0;
	eight.number_construction[2][5] = 1;

	eight.number_construction[0][6] = 1;
	eight.number_construction[1][6] = 1;
	eight.number_construction[2][6] = 1;

	/* number nine */
	nine.number_construction[0][0] = 1;
	nine.number_construction[1][0] = 1;
	nine.number_construction[2][0] = 1;

	nine.number_construction[0][1] = 1;
	nine.number_construction[1][1] = 0;
	nine.number_construction[2][1] = 1;

	nine.number_construction[0][2] = 1;
	nine.number_construction[1][2] = 0;
	nine.number_construction[2][2] = 1;

	nine.number_construction[0][3] = 1;
	nine.number_construction[1][3] = 1;
	nine.number_construction[2][3] = 1;

	nine.number_construction[0][4] = 0;
	nine.number_construction[1][4] = 0;
	nine.number_construction[2][4] = 1;

	nine.number_construction[0][5] = 0;
	nine.number_construction[1][5] = 0;
	nine.number_construction[2][5] = 1;

	nine.number_construction[0][6] = 1;
	nine.number_construction[1][6] = 1;
	nine.number_construction[2][6] = 1;

	/* doublepoint */
	doublepoint.number_construction[0][0] = 0;
	doublepoint.number_construction[1][0] = 0;
	doublepoint.number_construction[2][0] = 0;

	doublepoint.number_construction[0][1] = 0;
	doublepoint.number_construction[1][1] = 0;
	doublepoint.number_construction[2][1] = 0;

	doublepoint.number_construction[0][2] = 0;
	doublepoint.number_construction[1][2] = 1;
	doublepoint.number_construction[2][2] = 0;

	doublepoint.number_construction[0][3] = 0;
	doublepoint.number_construction[1][3] = 0;
	doublepoint.number_construction[2][3] = 0;

	doublepoint.number_construction[0][4] = 0;
	doublepoint.number_construction[1][4] = 1;
	doublepoint.number_construction[2][4] = 0;

	doublepoint.number_construction[0][5] = 0;
	doublepoint.number_construction[1][5] = 0;
	doublepoint.number_construction[2][5] = 0;

	doublepoint.number_construction[0][6] = 0;
	doublepoint.number_construction[1][6] = 0;
	doublepoint.number_construction[2][6] = 0;

	/* construct the letters
	 * 1 = set pixel
	 * 0 = empty pixel
	 */
	/* letter A */
	a.letter_construction[0][0] = 0;
	a.letter_construction[1][0] = 0;
	a.letter_construction[2][0] = 0;
	a.letter_construction[3][0] = 0;
	a.letter_construction[4][0] = 0;

	a.letter_construction[0][1] = 0;
	a.letter_construction[1][1] = 1;
	a.letter_construction[2][1] = 0;
	a.letter_construction[3][1] = 0;
	a.letter_construction[4][1] = 0;

	a.letter_construction[0][2] = 1;
	a.letter_construction[1][2] = 0;
	a.letter_construction[2][2] = 1;
	a.letter_construction[3][2] = 0;
	a.letter_construction[4][2] = 0;

	a.letter_construction[0][3] = 1;
	a.letter_construction[1][3] = 1;
	a.letter_construction[2][3] = 1;
	a.letter_construction[3][3] = 0;
	a.letter_construction[4][3] = 0;

	a.letter_construction[0][4] = 1;
	a.letter_construction[1][4] = 0;
	a.letter_construction[2][4] = 1;
	a.letter_construction[3][4] = 0;
	a.letter_construction[4][4] = 0;

	a.letter_construction[0][5] = 1;
	a.letter_construction[1][5] = 0;
	a.letter_construction[2][5] = 1;
	a.letter_construction[3][5] = 0;
	a.letter_construction[4][5] = 0;

	a.letter_construction[0][6] = 0;
	a.letter_construction[1][6] = 0;
	a.letter_construction[2][6] = 0;
	a.letter_construction[3][6] = 0;
	a.letter_construction[4][6] = 0;

	/* letter E */
	e.letter_construction[0][0] = 0;
	e.letter_construction[1][0] = 0;
	e.letter_construction[2][0] = 0;
	e.letter_construction[3][0] = 0;
	e.letter_construction[4][0] = 0;

	e.letter_construction[0][1] = 1;
	e.letter_construction[1][1] = 1;
	e.letter_construction[2][1] = 1;
	e.letter_construction[3][1] = 0;
	e.letter_construction[4][1] = 0;

	e.letter_construction[0][2] = 1;
	e.letter_construction[1][2] = 0;
	e.letter_construction[2][2] = 0;
	e.letter_construction[3][2] = 0;
	e.letter_construction[4][2] = 0;

	e.letter_construction[0][3] = 1;
	e.letter_construction[1][3] = 1;
	e.letter_construction[2][3] = 0;
	e.letter_construction[3][3] = 0;
	e.letter_construction[4][3] = 0;

	e.letter_construction[0][4] = 1;
	e.letter_construction[1][4] = 0;
	e.letter_construction[2][4] = 0;
	e.letter_construction[3][4] = 0;
	e.letter_construction[4][4] = 0;

	e.letter_construction[0][5] = 1;
	e.letter_construction[1][5] = 1;
	e.letter_construction[2][5] = 1;
	e.letter_construction[3][5] = 0;
	e.letter_construction[4][5] = 0;

	e.letter_construction[0][6] = 0;
	e.letter_construction[1][6] = 0;
	e.letter_construction[2][6] = 0;
	e.letter_construction[3][6] = 0;
	e.letter_construction[4][6] = 0;

	/* letter I */
	i.letter_construction[0][0] = 0;
	i.letter_construction[1][0] = 0;
	i.letter_construction[2][0] = 0;
	i.letter_construction[3][0] = 0;
	i.letter_construction[4][0] = 0;

	i.letter_construction[0][1] = 1;
	i.letter_construction[1][1] = 1;
	i.letter_construction[2][1] = 1;
	i.letter_construction[3][1] = 0;
	i.letter_construction[4][1] = 0;

	i.letter_construction[0][2] = 0;
	i.letter_construction[1][2] = 1;
	i.letter_construction[2][2] = 0;
	i.letter_construction[3][2] = 0;
	i.letter_construction[4][2] = 0;

	i.letter_construction[0][3] = 0;
	i.letter_construction[1][3] = 1;
	i.letter_construction[2][3] = 0;
	i.letter_construction[3][3] = 0;
	i.letter_construction[4][3] = 0;

	i.letter_construction[0][4] = 0;
	i.letter_construction[1][4] = 1;
	i.letter_construction[2][4] = 0;
	i.letter_construction[3][4] = 0;
	i.letter_construction[4][4] = 0;

	i.letter_construction[0][5] = 1;
	i.letter_construction[1][5] = 1;
	i.letter_construction[2][5] = 1;
	i.letter_construction[3][5] = 0;
	i.letter_construction[4][5] = 0;

	i.letter_construction[0][6] = 0;
	i.letter_construction[1][6] = 0;
	i.letter_construction[2][6] = 0;
	i.letter_construction[3][6] = 0;
	i.letter_construction[4][6] = 0;

	/* letter L */
	l.letter_construction[0][0] = 0;
	l.letter_construction[1][0] = 0;
	l.letter_construction[2][0] = 0;
	l.letter_construction[3][0] = 0;
	l.letter_construction[4][0] = 0;

	l.letter_construction[0][1] = 1;
	l.letter_construction[1][1] = 0;
	l.letter_construction[2][1] = 0;
	l.letter_construction[3][1] = 0;
	l.letter_construction[4][1] = 0;

	l.letter_construction[0][2] = 1;
	l.letter_construction[1][2] = 0;
	l.letter_construction[2][2] = 0;
	l.letter_construction[3][2] = 0;
	l.letter_construction[4][2] = 0;

	l.letter_construction[0][3] = 1;
	l.letter_construction[1][3] = 0;
	l.letter_construction[2][3] = 0;
	l.letter_construction[3][3] = 0;
	l.letter_construction[4][3] = 0;

	l.letter_construction[0][4] = 1;
	l.letter_construction[1][4] = 0;
	l.letter_construction[2][4] = 0;
	l.letter_construction[3][4] = 0;
	l.letter_construction[4][4] = 0;

	l.letter_construction[0][5] = 1;
	l.letter_construction[1][5] = 1;
	l.letter_construction[2][5] = 1;
	l.letter_construction[3][5] = 0;
	l.letter_construction[4][5] = 0;

	l.letter_construction[0][6] = 0;
	l.letter_construction[1][6] = 0;
	l.letter_construction[2][6] = 0;
	l.letter_construction[3][6] = 0;
	l.letter_construction[4][6] = 0;

	/* letter R */
	r.letter_construction[0][0] = 0;
	r.letter_construction[1][0] = 0;
	r.letter_construction[2][0] = 0;
	r.letter_construction[3][0] = 0;
	r.letter_construction[4][0] = 0;

	r.letter_construction[0][1] = 1;
	r.letter_construction[1][1] = 1;
	r.letter_construction[2][1] = 0;
	r.letter_construction[3][1] = 0;
	r.letter_construction[4][1] = 0;

	r.letter_construction[0][2] = 1;
	r.letter_construction[1][2] = 0;
	r.letter_construction[2][2] = 1;
	r.letter_construction[3][2] = 0;
	r.letter_construction[4][2] = 0;

	r.letter_construction[0][3] = 1;
	r.letter_construction[1][3] = 1;
	r.letter_construction[2][3] = 0;
	r.letter_construction[3][3] = 0;
	r.letter_construction[4][3] = 0;

	r.letter_construction[0][4] = 1;
	r.letter_construction[1][4] = 0;
	r.letter_construction[2][4] = 1;
	r.letter_construction[3][4] = 0;
	r.letter_construction[4][4] = 0;

	r.letter_construction[0][5] = 1;
	r.letter_construction[1][5] = 0;
	r.letter_construction[2][5] = 1;
	r.letter_construction[3][5] = 0;
	r.letter_construction[4][5] = 0;

	r.letter_construction[0][6] = 0;
	r.letter_construction[1][6] = 0;
	r.letter_construction[2][6] = 0;
	r.letter_construction[3][6] = 0;
	r.letter_construction[4][6] = 0;

	/* letter S */
	s.letter_construction[0][0] = 0;
	s.letter_construction[1][0] = 0;
	s.letter_construction[2][0] = 0;
	s.letter_construction[3][0] = 0;
	s.letter_construction[4][0] = 0;

	s.letter_construction[0][1] = 0;
	s.letter_construction[1][1] = 1;
	s.letter_construction[2][1] = 1;
	s.letter_construction[3][1] = 0;
	s.letter_construction[4][1] = 0;

	s.letter_construction[0][2] = 1;
	s.letter_construction[1][2] = 0;
	s.letter_construction[2][2] = 0;
	s.letter_construction[3][2] = 0;
	s.letter_construction[4][2] = 0;

	s.letter_construction[0][3] = 0;
	s.letter_construction[1][3] = 1;
	s.letter_construction[2][3] = 0;
	s.letter_construction[3][3] = 0;
	s.letter_construction[4][3] = 0;

	s.letter_construction[0][4] = 0;
	s.letter_construction[1][4] = 0;
	s.letter_construction[2][4] = 1;
	s.letter_construction[3][4] = 0;
	s.letter_construction[4][4] = 0;

	s.letter_construction[0][5] = 1;
	s.letter_construction[1][5] = 1;
	s.letter_construction[2][5] = 0;
	s.letter_construction[3][5] = 0;
	s.letter_construction[4][5] = 0;

	s.letter_construction[0][6] = 0;
	s.letter_construction[1][6] = 0;
	s.letter_construction[2][6] = 0;
	s.letter_construction[3][6] = 0;
	s.letter_construction[4][6] = 0;

	/* letter T */
	t.letter_construction[0][0] = 0;
	t.letter_construction[1][0] = 0;
	t.letter_construction[2][0] = 0;
	t.letter_construction[3][0] = 0;
	t.letter_construction[4][0] = 0;

	t.letter_construction[0][1] = 1;
	t.letter_construction[1][1] = 1;
	t.letter_construction[2][1] = 1;
	t.letter_construction[3][1] = 0;
	t.letter_construction[4][1] = 0;

	t.letter_construction[0][2] = 0;
	t.letter_construction[1][2] = 1;
	t.letter_construction[2][2] = 0;
	t.letter_construction[3][2] = 0;
	t.letter_construction[4][2] = 0;

	t.letter_construction[0][3] = 0;
	t.letter_construction[1][3] = 1;
	t.letter_construction[2][3] = 0;
	t.letter_construction[3][3] = 0;
	t.letter_construction[4][3] = 0;

	t.letter_construction[0][4] = 0;
	t.letter_construction[1][4] = 1;
	t.letter_construction[2][4] = 0;
	t.letter_construction[3][4] = 0;
	t.letter_construction[4][4] = 0;

	t.letter_construction[0][5] = 0;
	t.letter_construction[1][5] = 1;
	t.letter_construction[2][5] = 0;
	t.letter_construction[3][5] = 0;
	t.letter_construction[4][5] = 0;

	t.letter_construction[0][6] = 0;
	t.letter_construction[1][6] = 0;
	t.letter_construction[2][6] = 0;
	t.letter_construction[3][6] = 0;
	t.letter_construction[4][6] = 0;

	/* letter U */
	u.letter_construction[0][0] = 0;
	u.letter_construction[1][0] = 0;
	u.letter_construction[2][0] = 0;
	u.letter_construction[3][0] = 0;
	u.letter_construction[4][0] = 0;

	u.letter_construction[0][1] = 1;
	u.letter_construction[1][1] = 0;
	u.letter_construction[2][1] = 1;
	u.letter_construction[3][1] = 0;
	u.letter_construction[4][1] = 0;

	u.letter_construction[0][2] = 1;
	u.letter_construction[1][2] = 0;
	u.letter_construction[2][2] = 1;
	u.letter_construction[3][2] = 0;
	u.letter_construction[4][2] = 0;

	u.letter_construction[0][3] = 1;
	u.letter_construction[1][3] = 0;
	u.letter_construction[2][3] = 1;
	u.letter_construction[3][3] = 0;
	u.letter_construction[4][3] = 0;

	u.letter_construction[0][4] = 1;
	u.letter_construction[1][4] = 0;
	u.letter_construction[2][4] = 1;
	u.letter_construction[3][4] = 0;
	u.letter_construction[4][4] = 0;

	u.letter_construction[0][5] = 0;
	u.letter_construction[1][5] = 1;
	u.letter_construction[2][5] = 1;
	u.letter_construction[3][5] = 0;
	u.letter_construction[4][5] = 0;

	u.letter_construction[0][6] = 0;
	u.letter_construction[1][6] = 0;
	u.letter_construction[2][6] = 0;
	u.letter_construction[3][6] = 0;
	u.letter_construction[4][6] = 0;

	/* letter P */
	p.letter_construction[0][0] = 0;
	p.letter_construction[1][0] = 0;
	p.letter_construction[2][0] = 0;
	p.letter_construction[3][0] = 0;
	p.letter_construction[4][0] = 0;

	p.letter_construction[0][1] = 1;
	p.letter_construction[1][1] = 1;
	p.letter_construction[2][1] = 0;
	p.letter_construction[3][1] = 0;
	p.letter_construction[4][1] = 0;

	p.letter_construction[0][2] = 1;
	p.letter_construction[1][2] = 0;
	p.letter_construction[2][2] = 1;
	p.letter_construction[3][2] = 0;
	p.letter_construction[4][2] = 0;

	p.letter_construction[0][3] = 1;
	p.letter_construction[1][3] = 1;
	p.letter_construction[2][3] = 0;
	p.letter_construction[3][3] = 0;
	p.letter_construction[4][3] = 0;

	p.letter_construction[0][4] = 1;
	p.letter_construction[1][4] = 0;
	p.letter_construction[2][4] = 0;
	p.letter_construction[3][4] = 0;
	p.letter_construction[4][4] = 0;

	p.letter_construction[0][5] = 1;
	p.letter_construction[1][5] = 0;
	p.letter_construction[2][5] = 0;
	p.letter_construction[3][5] = 0;
	p.letter_construction[4][5] = 0;

	p.letter_construction[0][6] = 0;
	p.letter_construction[1][6] = 0;
	p.letter_construction[2][6] = 0;
	p.letter_construction[3][6] = 0;
	p.letter_construction[4][6] = 0;

	/* letter C */
	c.letter_construction[0][0] = 0;
	c.letter_construction[1][0] = 0;
	c.letter_construction[2][0] = 0;
	c.letter_construction[3][0] = 0;
	c.letter_construction[4][0] = 0;

	c.letter_construction[0][1] = 0;
	c.letter_construction[1][1] = 1;
	c.letter_construction[2][1] = 1;
	c.letter_construction[3][1] = 0;
	c.letter_construction[4][1] = 0;

	c.letter_construction[0][2] = 1;
	c.letter_construction[1][2] = 0;
	c.letter_construction[2][2] = 0;
	c.letter_construction[3][2] = 0;
	c.letter_construction[4][2] = 0;

	c.letter_construction[0][3] = 1;
	c.letter_construction[1][3] = 0;
	c.letter_construction[2][3] = 0;
	c.letter_construction[3][3] = 0;
	c.letter_construction[4][3] = 0;

	c.letter_construction[0][4] = 1;
	c.letter_construction[1][4] = 0;
	c.letter_construction[2][4] = 0;
	c.letter_construction[3][4] = 0;
	c.letter_construction[4][4] = 0;

	c.letter_construction[0][5] = 0;
	c.letter_construction[1][5] = 1;
	c.letter_construction[2][5] = 1;
	c.letter_construction[3][5] = 0;
	c.letter_construction[4][5] = 0;

	c.letter_construction[0][6] = 0;
	c.letter_construction[1][6] = 0;
	c.letter_construction[2][6] = 0;
	c.letter_construction[3][6] = 0;
	c.letter_construction[4][6] = 0;


	/* letter O */
	o.letter_construction[0][0] = 0;
	o.letter_construction[1][0] = 0;
	o.letter_construction[2][0] = 0;
	o.letter_construction[3][0] = 0;
	o.letter_construction[4][0] = 0;

	o.letter_construction[0][1] = 0;
	o.letter_construction[1][1] = 1;
	o.letter_construction[2][1] = 0;
	o.letter_construction[3][1] = 0;
	o.letter_construction[4][1] = 0;

	o.letter_construction[0][2] = 1;
	o.letter_construction[1][2] = 0;
	o.letter_construction[2][2] = 1;
	o.letter_construction[3][2] = 0;
	o.letter_construction[4][2] = 0;

	o.letter_construction[0][3] = 1;
	o.letter_construction[1][3] = 0;
	o.letter_construction[2][3] = 1;
	o.letter_construction[3][3] = 0;
	o.letter_construction[4][3] = 0;

	o.letter_construction[0][4] = 1;
	o.letter_construction[1][4] = 0;
	o.letter_construction[2][4] = 1;
	o.letter_construction[3][4] = 0;
	o.letter_construction[4][4] = 0;

	o.letter_construction[0][5] = 0;
	o.letter_construction[1][5] = 1;
	o.letter_construction[2][5] = 1;
	o.letter_construction[3][5] = 0;
	o.letter_construction[4][5] = 0;

	o.letter_construction[0][6] = 0;
	o.letter_construction[1][6] = 0;
	o.letter_construction[2][6] = 0;
	o.letter_construction[3][6] = 0;
	o.letter_construction[4][6] = 0;

	/* letter K */
	k.letter_construction[0][0] = 0;
	k.letter_construction[1][0] = 0;
	k.letter_construction[2][0] = 0;
	k.letter_construction[3][0] = 0;
	k.letter_construction[4][0] = 0;

	k.letter_construction[0][1] = 1;
	k.letter_construction[1][1] = 0;
	k.letter_construction[2][1] = 1;
	k.letter_construction[3][1] = 0;
	k.letter_construction[4][1] = 0;

	k.letter_construction[0][2] = 1;
	k.letter_construction[1][2] = 0;
	k.letter_construction[2][2] = 1;
	k.letter_construction[3][2] = 0;
	k.letter_construction[4][2] = 0;

	k.letter_construction[0][3] = 1;
	k.letter_construction[1][3] = 1;
	k.letter_construction[2][3] = 0;
	k.letter_construction[3][3] = 0;
	k.letter_construction[4][3] = 0;

	k.letter_construction[0][4] = 1;
	k.letter_construction[1][4] = 0;
	k.letter_construction[2][4] = 1;
	k.letter_construction[3][4] = 0;
	k.letter_construction[4][4] = 0;

	k.letter_construction[0][5] = 1;
	k.letter_construction[1][5] = 0;
	k.letter_construction[2][5] = 1;
	k.letter_construction[3][5] = 0;
	k.letter_construction[4][5] = 0;

	k.letter_construction[0][6] = 0;
	k.letter_construction[1][6] = 0;
	k.letter_construction[2][6] = 0;
	k.letter_construction[3][6] = 0;
	k.letter_construction[4][6] = 0;

	/* letter M */
	m.letter_construction[0][0] = 0;
	m.letter_construction[1][0] = 0;
	m.letter_construction[2][0] = 0;
	m.letter_construction[3][0] = 0;
	m.letter_construction[4][0] = 0;

	m.letter_construction[0][1] = 1;
	m.letter_construction[1][1] = 1;
	m.letter_construction[2][1] = 0;
	m.letter_construction[3][1] = 1;
	m.letter_construction[4][1] = 0;

	m.letter_construction[0][2] = 1;
	m.letter_construction[1][2] = 0;
	m.letter_construction[2][2] = 1;
	m.letter_construction[3][2] = 0;
	m.letter_construction[4][2] = 1;

	m.letter_construction[0][3] = 1;
	m.letter_construction[1][3] = 0;
	m.letter_construction[2][3] = 1;
	m.letter_construction[3][3] = 0;
	m.letter_construction[4][3] = 1;

	m.letter_construction[0][4] = 1;
	m.letter_construction[1][4] = 0;
	m.letter_construction[2][4] = 1;
	m.letter_construction[3][4] = 0;
	m.letter_construction[4][4] = 1;

	m.letter_construction[0][5] = 1;
	m.letter_construction[1][5] = 0;
	m.letter_construction[2][5] = 1;
	m.letter_construction[3][5] = 0;
	m.letter_construction[4][5] = 1;

	m.letter_construction[0][6] = 0;
	m.letter_construction[1][6] = 0;
	m.letter_construction[2][6] = 0;
	m.letter_construction[3][6] = 0;
	m.letter_construction[4][6] = 0;

	/* letter W */
	w.letter_construction[0][0] = 0;
	w.letter_construction[1][0] = 0;
	w.letter_construction[2][0] = 0;
	w.letter_construction[3][0] = 0;
	w.letter_construction[4][0] = 0;

	w.letter_construction[0][1] = 1;
	w.letter_construction[1][1] = 0;
	w.letter_construction[2][1] = 1;
	w.letter_construction[3][1] = 0;
	w.letter_construction[4][1] = 1;

	w.letter_construction[0][2] = 1;
	w.letter_construction[1][2] = 0;
	w.letter_construction[2][2] = 1;
	w.letter_construction[3][2] = 0;
	w.letter_construction[4][2] = 1;

	w.letter_construction[0][3] = 1;
	w.letter_construction[1][3] = 0;
	w.letter_construction[2][3] = 1;
	w.letter_construction[3][3] = 0;
	w.letter_construction[4][3] = 1;

	w.letter_construction[0][4] = 1;
	w.letter_construction[1][4] = 0;
	w.letter_construction[2][4] = 1;
	w.letter_construction[3][4] = 0;
	w.letter_construction[4][4] = 1;

	w.letter_construction[0][5] = 0;
	w.letter_construction[1][5] = 1;
	w.letter_construction[2][5] = 0;
	w.letter_construction[3][5] = 1;
	w.letter_construction[4][5] = 0;

	w.letter_construction[0][6] = 0;
	w.letter_construction[1][6] = 0;
	w.letter_construction[2][6] = 0;
	w.letter_construction[3][6] = 0;
	w.letter_construction[4][6] = 0;

	/* letter F */
	f.letter_construction[0][0] = 0;
	f.letter_construction[1][0] = 0;
	f.letter_construction[2][0] = 0;
	f.letter_construction[3][0] = 0;
	f.letter_construction[4][0] = 0;

	f.letter_construction[0][1] = 1;
	f.letter_construction[1][1] = 1;
	f.letter_construction[2][1] = 1;
	f.letter_construction[3][1] = 0;
	f.letter_construction[4][1] = 0;

	f.letter_construction[0][2] = 1;
	f.letter_construction[1][2] = 0;
	f.letter_construction[2][2] = 0;
	f.letter_construction[3][2] = 0;
	f.letter_construction[4][2] = 0;

	f.letter_construction[0][3] = 1;
	f.letter_construction[1][3] = 1;
	f.letter_construction[2][3] = 1;
	f.letter_construction[3][3] = 0;
	f.letter_construction[4][3] = 0;

	f.letter_construction[0][4] = 1;
	f.letter_construction[1][4] = 0;
	f.letter_construction[2][4] = 0;
	f.letter_construction[3][4] = 0;
	f.letter_construction[4][4] = 0;

	f.letter_construction[0][5] = 1;
	f.letter_construction[1][5] = 0;
	f.letter_construction[2][5] = 0;
	f.letter_construction[3][5] = 0;
	f.letter_construction[4][5] = 0;

	f.letter_construction[0][6] = 0;
	f.letter_construction[1][6] = 0;
	f.letter_construction[2][6] = 0;
	f.letter_construction[3][6] = 0;
	f.letter_construction[4][6] = 0;

	/* letter N */
	n.letter_construction[0][0] = 0;
	n.letter_construction[1][0] = 0;
	n.letter_construction[2][0] = 0;
	n.letter_construction[3][0] = 0;
	n.letter_construction[4][0] = 0;

	n.letter_construction[0][1] = 1;
	n.letter_construction[1][1] = 1;
	n.letter_construction[2][1] = 0;
	n.letter_construction[3][1] = 0;
	n.letter_construction[4][1] = 0;

	n.letter_construction[0][2] = 1;
	n.letter_construction[1][2] = 0;
	n.letter_construction[2][2] = 1;
	n.letter_construction[3][2] = 0;
	n.letter_construction[4][2] = 0;

	n.letter_construction[0][3] = 1;
	n.letter_construction[1][3] = 0;
	n.letter_construction[2][3] = 1;
	n.letter_construction[3][3] = 0;
	n.letter_construction[4][3] = 0;

	n.letter_construction[0][4] = 1;
	n.letter_construction[1][4] = 0;
	n.letter_construction[2][4] = 1;
	n.letter_construction[3][4] = 0;
	n.letter_construction[4][4] = 0;

	n.letter_construction[0][5] = 1;
	n.letter_construction[1][5] = 0;
	n.letter_construction[2][5] = 1;
	n.letter_construction[3][5] = 0;
	n.letter_construction[4][5] = 0;

	n.letter_construction[0][6] = 0;
	n.letter_construction[1][6] = 0;
	n.letter_construction[2][6] = 0;
	n.letter_construction[3][6] = 0;
	n.letter_construction[4][6] = 0;

	/* letter z */
	z.letter_construction[0][0] = 0;
	z.letter_construction[1][0] = 0;
	z.letter_construction[2][0] = 0;
	z.letter_construction[3][0] = 0;
	z.letter_construction[4][0] = 0;

	z.letter_construction[0][1] = 1;
	z.letter_construction[1][1] = 1;
	z.letter_construction[2][1] = 1;
	z.letter_construction[3][1] = 0;
	z.letter_construction[4][1] = 0;

	z.letter_construction[0][2] = 0;
	z.letter_construction[1][2] = 0;
	z.letter_construction[2][2] = 1;
	z.letter_construction[3][2] = 0;
	z.letter_construction[4][2] = 0;

	z.letter_construction[0][3] = 0;
	z.letter_construction[1][3] = 1;
	z.letter_construction[2][3] = 0;
	z.letter_construction[3][3] = 0;
	z.letter_construction[4][3] = 0;

	z.letter_construction[0][4] = 1;
	z.letter_construction[1][4] = 0;
	z.letter_construction[2][4] = 0;
	z.letter_construction[3][4] = 0;
	z.letter_construction[4][4] = 0;

	z.letter_construction[0][5] = 1;
	z.letter_construction[1][5] = 1;
	z.letter_construction[2][5] = 1;
	z.letter_construction[3][5] = 0;
	z.letter_construction[4][5] = 0;

	z.letter_construction[0][6] = 0;
	z.letter_construction[1][6] = 0;
	z.letter_construction[2][6] = 0;
	z.letter_construction[3][6] = 0;
	z.letter_construction[4][6] = 0;

	/* letter _ (space) */
	_.letter_construction[0][0] = 0;
	_.letter_construction[1][0] = 0;
	_.letter_construction[2][0] = 0;
	_.letter_construction[3][0] = 0;
	_.letter_construction[4][0] = 0;

	_.letter_construction[0][1] = 0;
	_.letter_construction[1][1] = 0;
	_.letter_construction[2][1] = 0;
	_.letter_construction[3][1] = 0;
	_.letter_construction[4][1] = 0;

	_.letter_construction[0][2] = 0;
	_.letter_construction[1][2] = 0;
	_.letter_construction[2][2] = 0;
	_.letter_construction[3][2] = 0;
	_.letter_construction[4][2] = 0;

	_.letter_construction[0][3] = 0;
	_.letter_construction[1][3] = 0;
	_.letter_construction[2][3] = 0;
	_.letter_construction[3][3] = 0;
	_.letter_construction[4][3] = 0;

	_.letter_construction[0][4] = 0;
	_.letter_construction[1][4] = 0;
	_.letter_construction[2][4] = 0;
	_.letter_construction[3][4] = 0;
	_.letter_construction[4][4] = 0;

	_.letter_construction[0][5] = 0;
	_.letter_construction[1][5] = 0;
	_.letter_construction[2][5] = 0;
	_.letter_construction[3][5] = 0;
	_.letter_construction[4][5] = 0;

	_.letter_construction[0][6] = 0;
	_.letter_construction[1][6] = 0;
	_.letter_construction[2][6] = 0;
	_.letter_construction[3][6] = 0;
	_.letter_construction[4][6] = 0;

	/* letter x */
	x.letter_construction[0][0] = 0;
	x.letter_construction[1][0] = 0;
	x.letter_construction[2][0] = 0;
	x.letter_construction[3][0] = 0;
	x.letter_construction[4][0] = 0;

	x.letter_construction[0][1] = 1;
	x.letter_construction[1][1] = 0;
	x.letter_construction[2][1] = 1;
	x.letter_construction[3][1] = 0;
	x.letter_construction[4][1] = 0;

	x.letter_construction[0][2] = 1;
	x.letter_construction[1][2] = 0;
	x.letter_construction[2][2] = 1;
	x.letter_construction[3][2] = 0;
	x.letter_construction[4][2] = 0;

	x.letter_construction[0][3] = 0;
	x.letter_construction[1][3] = 1;
	x.letter_construction[2][3] = 0;
	x.letter_construction[3][3] = 0;
	x.letter_construction[4][3] = 0;

	x.letter_construction[0][4] = 1;
	x.letter_construction[1][4] = 0;
	x.letter_construction[2][4] = 1;
	x.letter_construction[3][4] = 0;
	x.letter_construction[4][4] = 0;

	x.letter_construction[0][5] = 1;
	x.letter_construction[1][5] = 0;
	x.letter_construction[2][5] = 1;
	x.letter_construction[3][5] = 0;
	x.letter_construction[4][5] = 0;

	x.letter_construction[0][6] = 0;
	x.letter_construction[1][6] = 0;
	x.letter_construction[2][6] = 0;
	x.letter_construction[3][6] = 0;
	x.letter_construction[4][6] = 0;

	/* letter h */
	h.letter_construction[0][0] = 0;
	h.letter_construction[1][0] = 0;
	h.letter_construction[2][0] = 0;
	h.letter_construction[3][0] = 0;
	h.letter_construction[4][0] = 0;

	h.letter_construction[0][1] = 1;
	h.letter_construction[1][1] = 0;
	h.letter_construction[2][1] = 1;
	h.letter_construction[3][1] = 0;
	h.letter_construction[4][1] = 0;

	h.letter_construction[0][2] = 1;
	h.letter_construction[1][2] = 0;
	h.letter_construction[2][2] = 1;
	h.letter_construction[3][2] = 0;
	h.letter_construction[4][2] = 0;

	h.letter_construction[0][3] = 1;
	h.letter_construction[1][3] = 1;
	h.letter_construction[2][3] = 1;
	h.letter_construction[3][3] = 0;
	h.letter_construction[4][3] = 0;

	h.letter_construction[0][4] = 1;
	h.letter_construction[1][4] = 0;
	h.letter_construction[2][4] = 1;
	h.letter_construction[3][4] = 0;
	h.letter_construction[4][4] = 0;

	h.letter_construction[0][5] = 1;
	h.letter_construction[1][5] = 0;
	h.letter_construction[2][5] = 1;
	h.letter_construction[3][5] = 0;
	h.letter_construction[4][5] = 0;

	h.letter_construction[0][6] = 0;
	h.letter_construction[1][6] = 0;
	h.letter_construction[2][6] = 0;
	h.letter_construction[3][6] = 0;
	h.letter_construction[4][6] = 0;

	/* letter d */
	d.letter_construction[0][0] = 0;
	d.letter_construction[1][0] = 0;
	d.letter_construction[2][0] = 0;
	d.letter_construction[3][0] = 0;
	d.letter_construction[4][0] = 0;

	d.letter_construction[0][1] = 1;
	d.letter_construction[1][1] = 1;
	d.letter_construction[2][1] = 0;
	d.letter_construction[3][1] = 0;
	d.letter_construction[4][1] = 0;

	d.letter_construction[0][2] = 1;
	d.letter_construction[1][2] = 0;
	d.letter_construction[2][2] = 1;
	d.letter_construction[3][2] = 0;
	d.letter_construction[4][2] = 0;

	d.letter_construction[0][3] = 1;
	d.letter_construction[1][3] = 0;
	d.letter_construction[2][3] = 1;
	d.letter_construction[3][3] = 0;
	d.letter_construction[4][3] = 0;

	d.letter_construction[0][4] = 1;
	d.letter_construction[1][4] = 0;
	d.letter_construction[2][4] = 1;
	d.letter_construction[3][4] = 0;
	d.letter_construction[4][4] = 0;

	d.letter_construction[0][5] = 1;
	d.letter_construction[1][5] = 1;
	d.letter_construction[2][5] = 0;
	d.letter_construction[3][5] = 0;
	d.letter_construction[4][5] = 0;

	d.letter_construction[0][6] = 0;
	d.letter_construction[1][6] = 0;
	d.letter_construction[2][6] = 0;
	d.letter_construction[3][6] = 0;
	d.letter_construction[4][6] = 0;

	/* number zero */
	nzero.letter_construction[0][0] = 0;
	nzero.letter_construction[1][0] = 0;
	nzero.letter_construction[2][0] = 0;
	nzero.letter_construction[3][0] = 0;
	nzero.letter_construction[4][0] = 0;

	nzero.letter_construction[0][1] = 1;
	nzero.letter_construction[1][1] = 1;
	nzero.letter_construction[2][1] = 1;
	nzero.letter_construction[3][1] = 0;
	nzero.letter_construction[4][1] = 0;

	nzero.letter_construction[0][2] = 1;
	nzero.letter_construction[1][2] = 0;
	nzero.letter_construction[2][2] = 1;
	nzero.letter_construction[3][2] = 0;
	nzero.letter_construction[4][2] = 0;

	nzero.letter_construction[0][3] = 1;
	nzero.letter_construction[1][3] = 0;
	nzero.letter_construction[2][3] = 1;
	nzero.letter_construction[3][3] = 0;
	nzero.letter_construction[4][3] = 0;

	nzero.letter_construction[0][4] = 1;
	nzero.letter_construction[1][4] = 0;
	nzero.letter_construction[2][4] = 1;
	nzero.letter_construction[3][4] = 0;
	nzero.letter_construction[4][4] = 0;

	nzero.letter_construction[0][5] = 1;
	nzero.letter_construction[1][5] = 1;
	nzero.letter_construction[2][5] = 1;
	nzero.letter_construction[3][5] = 0;
	nzero.letter_construction[4][5] = 0;

	nzero.letter_construction[0][6] = 0;
	nzero.letter_construction[1][6] = 0;
	nzero.letter_construction[2][6] = 0;
	nzero.letter_construction[3][6] = 0;
	nzero.letter_construction[4][6] = 0;

	/* number one */
	none.letter_construction[0][0] = 0;
	none.letter_construction[1][0] = 0;
	none.letter_construction[2][0] = 0;
	none.letter_construction[3][0] = 0;
	none.letter_construction[4][0] = 0;

	none.letter_construction[0][1] = 0;
	none.letter_construction[1][1] = 0;
	none.letter_construction[2][1] = 1;
	none.letter_construction[3][1] = 0;
	none.letter_construction[4][1] = 0;

	none.letter_construction[0][2] = 0;
	none.letter_construction[1][2] = 0;
	none.letter_construction[2][2] = 1;
	none.letter_construction[3][2] = 0;
	none.letter_construction[4][2] = 0;

	none.letter_construction[0][3] = 0;
	none.letter_construction[1][3] = 0;
	none.letter_construction[2][3] = 1;
	none.letter_construction[3][3] = 0;
	none.letter_construction[4][3] = 0;

	none.letter_construction[0][4] = 0;
	none.letter_construction[1][4] = 0;
	none.letter_construction[2][4] = 1;
	none.letter_construction[3][4] = 0;
	none.letter_construction[4][4] = 0;

	none.letter_construction[0][5] = 0;
	none.letter_construction[1][5] = 0;
	none.letter_construction[2][5] = 1;
	none.letter_construction[3][5] = 0;
	none.letter_construction[4][5] = 0;

	none.letter_construction[0][6] = 0;
	none.letter_construction[1][6] = 0;
	none.letter_construction[2][6] = 0;
	none.letter_construction[3][6] = 0;
	none.letter_construction[4][6] = 0;

	/* number two */
	ntwo.letter_construction[0][0] = 0;
	ntwo.letter_construction[1][0] = 0;
	ntwo.letter_construction[2][0] = 0;
	ntwo.letter_construction[3][0] = 0;
	ntwo.letter_construction[4][0] = 0;

	ntwo.letter_construction[0][1] = 1;
	ntwo.letter_construction[1][1] = 1;
	ntwo.letter_construction[2][1] = 1;
	ntwo.letter_construction[3][1] = 0;
	ntwo.letter_construction[4][1] = 0;

	ntwo.letter_construction[0][2] = 0;
	ntwo.letter_construction[1][2] = 0;
	ntwo.letter_construction[2][2] = 1;
	ntwo.letter_construction[3][2] = 0;
	ntwo.letter_construction[4][2] = 0;

	ntwo.letter_construction[0][3] = 1;
	ntwo.letter_construction[1][3] = 1;
	ntwo.letter_construction[2][3] = 1;
	ntwo.letter_construction[3][3] = 0;
	ntwo.letter_construction[4][3] = 0;

	ntwo.letter_construction[0][4] = 1;
	ntwo.letter_construction[1][4] = 0;
	ntwo.letter_construction[2][4] = 0;
	ntwo.letter_construction[3][4] = 0;
	ntwo.letter_construction[4][4] = 0;

	ntwo.letter_construction[0][5] = 1;
	ntwo.letter_construction[1][5] = 1;
	ntwo.letter_construction[2][5] = 1;
	ntwo.letter_construction[3][5] = 0;
	ntwo.letter_construction[4][5] = 0;

	ntwo.letter_construction[0][6] = 0;
	ntwo.letter_construction[1][6] = 0;
	ntwo.letter_construction[2][6] = 0;
	ntwo.letter_construction[3][6] = 0;
	ntwo.letter_construction[4][6] = 0;

	/* number three */
	nthree.letter_construction[0][0] = 0;
	nthree.letter_construction[1][0] = 0;
	nthree.letter_construction[2][0] = 0;
	nthree.letter_construction[3][0] = 0;
	nthree.letter_construction[4][0] = 0;

	nthree.letter_construction[0][1] = 1;
	nthree.letter_construction[1][1] = 1;
	nthree.letter_construction[2][1] = 1;
	nthree.letter_construction[3][1] = 0;
	nthree.letter_construction[4][1] = 0;

	nthree.letter_construction[0][2] = 0;
	nthree.letter_construction[1][2] = 0;
	nthree.letter_construction[2][2] = 1;
	nthree.letter_construction[3][2] = 0;
	nthree.letter_construction[4][2] = 0;

	nthree.letter_construction[0][3] = 1;
	nthree.letter_construction[1][3] = 1;
	nthree.letter_construction[2][3] = 1;
	nthree.letter_construction[3][3] = 0;
	nthree.letter_construction[4][3] = 0;

	nthree.letter_construction[0][4] = 0;
	nthree.letter_construction[1][4] = 0;
	nthree.letter_construction[2][4] = 1;
	nthree.letter_construction[3][4] = 0;
	nthree.letter_construction[4][4] = 0;

	nthree.letter_construction[0][5] = 1;
	nthree.letter_construction[1][5] = 1;
	nthree.letter_construction[2][5] = 1;
	nthree.letter_construction[3][5] = 0;
	nthree.letter_construction[4][5] = 0;

	nthree.letter_construction[0][6] = 0;
	nthree.letter_construction[1][6] = 0;
	nthree.letter_construction[2][6] = 0;
	nthree.letter_construction[3][6] = 0;
	nthree.letter_construction[4][6] = 0;

	/* number four */
	nfour.letter_construction[0][0] = 0;
	nfour.letter_construction[1][0] = 0;
	nfour.letter_construction[2][0] = 0;
	nfour.letter_construction[3][0] = 0;
	nfour.letter_construction[4][0] = 0;

	nfour.letter_construction[0][1] = 1;
	nfour.letter_construction[1][1] = 0;
	nfour.letter_construction[2][1] = 1;
	nfour.letter_construction[3][1] = 0;
	nfour.letter_construction[4][1] = 0;

	nfour.letter_construction[0][2] = 1;
	nfour.letter_construction[1][2] = 0;
	nfour.letter_construction[2][2] = 1;
	nfour.letter_construction[3][2] = 0;
	nfour.letter_construction[4][2] = 0;

	nfour.letter_construction[0][3] = 1;
	nfour.letter_construction[1][3] = 1;
	nfour.letter_construction[2][3] = 1;
	nfour.letter_construction[3][3] = 0;
	nfour.letter_construction[4][3] = 0;

	nfour.letter_construction[0][4] = 0;
	nfour.letter_construction[1][4] = 0;
	nfour.letter_construction[2][4] = 1;
	nfour.letter_construction[3][4] = 0;
	nfour.letter_construction[4][4] = 0;

	nfour.letter_construction[0][5] = 0;
	nfour.letter_construction[1][5] = 0;
	nfour.letter_construction[2][5] = 1;
	nfour.letter_construction[3][5] = 0;
	nfour.letter_construction[4][5] = 0;

	nfour.letter_construction[0][6] = 0;
	nfour.letter_construction[1][6] = 0;
	nfour.letter_construction[2][6] = 0;
	nfour.letter_construction[3][6] = 0;
	nfour.letter_construction[4][6] = 0;

	/* number five */
	nfive.letter_construction[0][0] = 0;
	nfive.letter_construction[1][0] = 0;
	nfive.letter_construction[2][0] = 0;
	nfive.letter_construction[3][0] = 0;
	nfive.letter_construction[4][0] = 0;

	nfive.letter_construction[0][1] = 1;
	nfive.letter_construction[1][1] = 1;
	nfive.letter_construction[2][1] = 1;
	nfive.letter_construction[3][1] = 0;
	nfive.letter_construction[4][1] = 0;

	nfive.letter_construction[0][2] = 1;
	nfive.letter_construction[1][2] = 0;
	nfive.letter_construction[2][2] = 0;
	nfive.letter_construction[3][2] = 0;
	nfive.letter_construction[4][2] = 0;

	nfive.letter_construction[0][3] = 1;
	nfive.letter_construction[1][3] = 1;
	nfive.letter_construction[2][3] = 1;
	nfive.letter_construction[3][3] = 0;
	nfive.letter_construction[4][3] = 0;

	nfive.letter_construction[0][4] = 0;
	nfive.letter_construction[1][4] = 0;
	nfive.letter_construction[2][4] = 1;
	nfive.letter_construction[3][4] = 0;
	nfive.letter_construction[4][4] = 0;

	nfive.letter_construction[0][5] = 1;
	nfive.letter_construction[1][5] = 1;
	nfive.letter_construction[2][5] = 1;
	nfive.letter_construction[3][5] = 0;
	nfive.letter_construction[4][5] = 0;

	nfive.letter_construction[0][6] = 0;
	nfive.letter_construction[1][6] = 0;
	nfive.letter_construction[2][6] = 0;
	nfive.letter_construction[3][6] = 0;
	nfive.letter_construction[4][6] = 0;

	/* number six */
	nsix.letter_construction[0][0] = 0;
	nsix.letter_construction[1][0] = 0;
	nsix.letter_construction[2][0] = 0;
	nsix.letter_construction[3][0] = 0;
	nsix.letter_construction[4][0] = 0;

	nsix.letter_construction[0][1] = 1;
	nsix.letter_construction[1][1] = 1;
	nsix.letter_construction[2][1] = 1;
	nsix.letter_construction[3][1] = 0;
	nsix.letter_construction[4][1] = 0;

	nsix.letter_construction[0][2] = 1;
	nsix.letter_construction[1][2] = 0;
	nsix.letter_construction[2][2] = 0;
	nsix.letter_construction[3][2] = 0;
	nsix.letter_construction[4][2] = 0;

	nsix.letter_construction[0][3] = 1;
	nsix.letter_construction[1][3] = 1;
	nsix.letter_construction[2][3] = 1;
	nsix.letter_construction[3][3] = 0;
	nsix.letter_construction[4][3] = 0;

	nsix.letter_construction[0][4] = 1;
	nsix.letter_construction[1][4] = 0;
	nsix.letter_construction[2][4] = 1;
	nsix.letter_construction[3][4] = 0;
	nsix.letter_construction[4][4] = 0;

	nsix.letter_construction[0][5] = 1;
	nsix.letter_construction[1][5] = 1;
	nsix.letter_construction[2][5] = 1;
	nsix.letter_construction[3][5] = 0;
	nsix.letter_construction[4][5] = 0;

	nsix.letter_construction[0][6] = 0;
	nsix.letter_construction[1][6] = 0;
	nsix.letter_construction[2][6] = 0;
	nsix.letter_construction[3][6] = 0;
	nsix.letter_construction[4][6] = 0;

	/* number seven */
	nseven.letter_construction[0][0] = 0;
	nseven.letter_construction[1][0] = 0;
	nseven.letter_construction[2][0] = 0;
	nseven.letter_construction[3][0] = 0;
	nseven.letter_construction[4][0] = 0;

	nseven.letter_construction[0][1] = 1;
	nseven.letter_construction[1][1] = 1;
	nseven.letter_construction[2][1] = 1;
	nseven.letter_construction[3][1] = 0;
	nseven.letter_construction[4][1] = 0;

	nseven.letter_construction[0][2] = 0;
	nseven.letter_construction[1][2] = 0;
	nseven.letter_construction[2][2] = 1;
	nseven.letter_construction[3][2] = 0;
	nseven.letter_construction[4][2] = 0;

	nseven.letter_construction[0][3] = 0;
	nseven.letter_construction[1][3] = 0;
	nseven.letter_construction[2][3] = 1;
	nseven.letter_construction[3][3] = 0;
	nseven.letter_construction[4][3] = 0;

	nseven.letter_construction[0][4] = 0;
	nseven.letter_construction[1][4] = 0;
	nseven.letter_construction[2][4] = 1;
	nseven.letter_construction[3][4] = 0;
	nseven.letter_construction[4][4] = 0;

	nseven.letter_construction[0][5] = 0;
	nseven.letter_construction[1][5] = 0;
	nseven.letter_construction[2][5] = 1;
	nseven.letter_construction[3][5] = 0;
	nseven.letter_construction[4][5] = 0;

	nseven.letter_construction[0][6] = 0;
	nseven.letter_construction[1][6] = 0;
	nseven.letter_construction[2][6] = 0;
	nseven.letter_construction[3][6] = 0;
	nseven.letter_construction[4][6] = 0;

	/* number eight */
	neight.letter_construction[0][0] = 0;
	neight.letter_construction[1][0] = 0;
	neight.letter_construction[2][0] = 0;
	neight.letter_construction[3][0] = 0;
	neight.letter_construction[4][0] = 0;

	neight.letter_construction[0][1] = 1;
	neight.letter_construction[1][1] = 1;
	neight.letter_construction[2][1] = 1;
	neight.letter_construction[3][1] = 0;
	neight.letter_construction[4][1] = 0;

	neight.letter_construction[0][2] = 1;
	neight.letter_construction[1][2] = 0;
	neight.letter_construction[2][2] = 1;
	neight.letter_construction[3][2] = 0;
	neight.letter_construction[4][2] = 0;

	neight.letter_construction[0][3] = 1;
	neight.letter_construction[1][3] = 1;
	neight.letter_construction[2][3] = 1;
	neight.letter_construction[3][3] = 0;
	neight.letter_construction[4][3] = 0;

	neight.letter_construction[0][4] = 1;
	neight.letter_construction[1][4] = 0;
	neight.letter_construction[2][4] = 1;
	neight.letter_construction[3][4] = 0;
	neight.letter_construction[4][4] = 0;

	neight.letter_construction[0][5] = 1;
	neight.letter_construction[1][5] = 1;
	neight.letter_construction[2][5] = 1;
	neight.letter_construction[3][5] = 0;
	neight.letter_construction[4][5] = 0;

	neight.letter_construction[0][6] = 0;
	neight.letter_construction[1][6] = 0;
	neight.letter_construction[2][6] = 0;
	neight.letter_construction[3][6] = 0;
	neight.letter_construction[4][6] = 0;

	/* number nine */
	nnine.letter_construction[0][0] = 0;
	nnine.letter_construction[1][0] = 0;
	nnine.letter_construction[2][0] = 0;
	nnine.letter_construction[3][0] = 0;
	nnine.letter_construction[4][0] = 0;

	nnine.letter_construction[0][1] = 1;
	nnine.letter_construction[1][1] = 1;
	nnine.letter_construction[2][1] = 1;
	nnine.letter_construction[3][1] = 0;
	nnine.letter_construction[4][1] = 0;

	nnine.letter_construction[0][2] = 1;
	nnine.letter_construction[1][2] = 0;
	nnine.letter_construction[2][2] = 1;
	nnine.letter_construction[3][2] = 0;
	nnine.letter_construction[4][2] = 0;

	nnine.letter_construction[0][3] = 1;
	nnine.letter_construction[1][3] = 1;
	nnine.letter_construction[2][3] = 1;
	nnine.letter_construction[3][3] = 0;
	nnine.letter_construction[4][3] = 0;

	nnine.letter_construction[0][4] = 0;
	nnine.letter_construction[1][4] = 0;
	nnine.letter_construction[2][4] = 1;
	nnine.letter_construction[3][4] = 0;
	nnine.letter_construction[4][4] = 0;

	nnine.letter_construction[0][5] = 1;
	nnine.letter_construction[1][5] = 1;
	nnine.letter_construction[2][5] = 1;
	nnine.letter_construction[3][5] = 0;
	nnine.letter_construction[4][5] = 0;

	nnine.letter_construction[0][6] = 0;
	nnine.letter_construction[1][6] = 0;
	nnine.letter_construction[2][6] = 0;
	nnine.letter_construction[3][6] = 0;
	nnine.letter_construction[4][6] = 0;
}

/**
  * @brief  draws a a letter into the IO buffer
  * @note   None
  * @retval None
  */
Letter char_to_letter(char charistic){
	Letter letter;
	switch(charistic){
		case 'm':	return letter = m;
		break;
		case 'w':	return letter = w;
		break;
		case 'a':	return letter = a;
		break;
		case 'd':	return letter = d;
		break;
		case 'e':	return letter = e;
		break;
		case 'f':	return letter = f;
		break;
		case 'h':	return letter = h;
		break;
		case 'i':	return letter = i;
		break;
		case 'l':	return letter = l;
		break;
		case 'r':	return letter = r;
		break;
		case 's':	return letter = s;
		break;
		case 't':	return letter = t;
		break;
		case 'u':	return letter = u;
		break;
		case 'p':	return letter = p;
		break;
		case 'c':	return letter = c;
		break;
		case 'o':	return letter = o;
		break;
		case 'k':	return letter = k;
		break;
		case 'n':	return letter = n;
		break;
		case ' ':	return letter = _;
		break;
		case 'x':	return letter = x;
		break;
		case 'z':	return letter = z;
		break;
		case '0':	return letter = nzero;
		break;
		case '1':	return letter = none;
		break;
		case '2':	return letter = ntwo;
		break;
		case '3':	return letter = nthree;
		break;
		case '4':	return letter = nfour;
		break;
		case '5':	return letter = nfive;
		break;
		case '6':	return letter = nsix;
		break;
		case '7':	return letter = nseven;
		break;
		case '8':	return letter = neight;
		break;
		case '9':	return letter = nnine;
		break;
		default: return letter = a;

	}
}

/**
  * @brief  draws a a letter into the IO buffer
  * @note   None
  * @retval None
  */
Number char_to_number(char charistic){
	Number number;
	switch(charistic){
		case '0':	return number = zero;
		break;
		case '1':	return number = one;
		break;
		case '2':	return number = two;
		break;
		case '3':	return number = three;
		break;
		case '4':	return number = four;
		break;
		case '5':	return number = five;
		break;
		case '6':	return number = six;
		break;
		case '7':	return number = seven;
		break;
		case '8':	return number = eight;
		break;
		case '9':	return number = nine;
		break;
		case ':':	return number = doublepoint;
		break;
		default: return number = zero;
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
			for(uint16_t j=0; j<ROW-1; j++){
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
  * @brief  this function sets the foregroundcolor
  * @note   this function sets the foregroundcolor
  * @retval -
  */
void WS2812_foreground_colour(uint8_t red, uint8_t green, uint8_t blue){
	/* clear frame buffer */
	for(uint8_t y=0; y<ROW;y++){
		for(uint16_t x=0; x<COL; x++){
			WS2812_framedata_setPixel(y, x, red, green, blue);
		}
	}
	while(!WS2812_TC);
	/* send frame buffer to the leds */
	sendbuf_WS2812();
}

/**
  * @brief  this function lets the display flash
  * @note   this function lets the display flash
  * @retval -
  */
uint8_t WS2812_display_flash(uint32_t speed_ms, uint8_t flash_count){
	for(uint8_t i = 0; i<flash_count; i++){
		if(stop_flag){
			stop_flag = 0;
			return 1;
		}
		/* set foreground to white */
		WS2812_foreground_colour(0xFF, 0xFF , 0xFF);
		/* wait until peripherals are ready */
		while(!WS2812_TC);
		/* send frame buffer to the leds */
		sendbuf_WS2812();
		/* wait specified time */
		HAL_Delay(speed_ms>>2);
		/* clear display */
		WS2812_foreground_colour(0x00, 0x00, 0x00);
		/* wait until peripherals are ready */
		while(!WS2812_TC);
		/* send frame buffer to the leds */
		sendbuf_WS2812();
		/* wait specified time */
		HAL_Delay(speed_ms>>2);
	}
}

/**
  * @brief  led test
  * @note   this function shows a colorfall on the display
  * @retval -
  */
uint8_t WS2812_display_colorfall(void){
	uint8_t redtest = 0xff;
	uint8_t greentest = 0x00;
	uint8_t bluetest = 0x00;

	for(uint16_t i = 0; i<20; i++){
		if(stop_flag){
			stop_flag = 0;
			return 1;
		}
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
			for(uint16_t j=0; j<ROW-1; j++){
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
  * @brief  stops running animations
  * @note   None
  * @retval None
  */
void WS2812_stop_animation(void){
	stop_flag = 1;
}

/**
  * @brief  draws a a letter into the IO buffer
  * @note   None
  * @retval None
  */
void draw_letter(char character, int16_t x_offset, int8_t y_offset, uint8_t *red, uint8_t *green, uint8_t *blue, uint16_t *ambient_factor){
	for(int16_t x = 0; x < 5; x++){
		for(int8_t y = 0; y < 7; y++){
			if(char_to_letter(character).letter_construction[x][y] != 0 && (y_offset+y >= 0) && (y_offset+y < ROW) && (x_offset+x >= 0) && (x_offset+x < COL)){
				WS2812_framedata_setPixel((uint8_t)y_offset + (uint8_t)y, (uint16_t)x_offset + (uint16_t)x, (uint8_t)*red*(uint8_t)*ambient_factor, (uint8_t)*green*(uint8_t)*ambient_factor, (uint8_t)*blue*(uint8_t)*ambient_factor);
			}
		}
	}
}

/**
  * @brief  draws a a number into the IO buffer
  * @note   None
  * @retval None
  */
void draw_number(char character, int16_t x_offset, int8_t y_offset, uint8_t *red, uint8_t *green, uint8_t *blue, uint16_t *ambient_factor){
	for(int16_t x = 0; x < 3; x++){
		for(int8_t y = 0; y < 7; y++){
			if(char_to_number(character).number_construction[x][y] != 0 && (y_offset+y >= 0) && (y_offset+y < ROW) && (x_offset+x >= 0) && (x_offset+x < COL)){
				WS2812_framedata_setPixel((uint8_t)y_offset + (uint8_t)y, (uint16_t)x_offset + (uint16_t)x, (uint8_t)*red*(uint8_t)*ambient_factor, (uint8_t)*green*(uint8_t)*ambient_factor, (uint8_t)*blue*(uint8_t)*ambient_factor);
			}
		}
	}
}

/**
  * @brief  draws a a letter into the IO buffer
  * @note   None
  * @retval None
  */
void draw_string(char *string, int16_t x_offset, int8_t y_offset, uint8_t *red, uint8_t *green, uint8_t *blue, uint16_t *ambient_factor){
	int16_t length = strlen(string);
	int16_t x_offset_letter = x_offset;
	int16_t additional_frame = 0;
	/* calculate the amount of frames to roll the whole text through the display */
	for(uint8_t i = 0; i < length; i++){
		if(*(string+i+1) == 'm' || *(string+i+1) == 'w'){
			additional_frame+=2;
		}
	}
	/* if text length is longer than the display, let it run through it, else just write text on it*/
	if(4*length > COL){
		/* write and display */
		for(int16_t j = 0; j>((-4)*length)+(17)-additional_frame; j--){
			/* erase frame buffer */
			WS2812_clear_buffer();
			/* write letters into buffer for 1 frame */
			for(uint8_t i = 0; i < length; i++){
				draw_letter(*(string+i), j+x_offset_letter, y_offset, red, green, blue, ambient_factor);
				if(*(string+i+1) == 'm' || *(string+i+1) == 'w'){
					x_offset_letter += 4;
				}else if(*(string+i) == 'm' || *(string+i) == 'w'){
					x_offset_letter += 6;
				}else{
					x_offset_letter += 4;
				}
			}
			x_offset_letter = x_offset;
			/* wait for the data transmission to the led's to be ready */
			while(!WS2812_TC);
			/* send frame buffer to the leds */
			sendbuf_WS2812();
			/* delay that the user can read the message */
			/* roll through the text with XX ms period */
			if(j == 0){
				HAL_Delay(500);
			}else{
				HAL_Delay(30);
			}
		}
		/* wait a bit to show the letters */
		HAL_Delay(500);
	}else{
		/* erase frame buffer */
		WS2812_clear_buffer();
		/* write letters into buffer for 1 frame */
		for(uint8_t i = 0; i < length; i++){
			draw_letter(*(string+i), x_offset_letter, y_offset, red, green, blue, ambient_factor);
			if(*(string+i+1) == 'm' || *(string+i+1) == 'w'){
				x_offset_letter += 4;
			}else if(*(string+i) == 'm' || *(string+i) == 'w'){
				x_offset_letter += 6;
			}else{
				x_offset_letter += 4;
			}
		}
		x_offset_letter = x_offset;
		/* wait for the data transmission to the led's to be ready */
		while(!WS2812_TC);
		/* send frame buffer to the leds */
		sendbuf_WS2812();
		/* delay that the user can read the message */
	}
}
