/*
 * Autor: Nico Korn
 * Date: 15.05.2018
 * Firmware for a alarmlcock with custom made STM32F103 microcontroller board.
 *  *
 * Copyright (c) 2018 Nico Korn
 *
 * lightsensor.c this module contents lightsensor init and functions.
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

#include "lightsensor.h"

/* defines */
#define BUFFERSIZE			64

/* private variables */
static uint16_t 			adc_buffer[BUFFERSIZE];
static TIM_HandleTypeDef    TIM3_Handle;

/* global variables */
ADC_HandleTypeDef 			ADC_Handle;
DMA_HandleTypeDef 			DMA_Handle;
RCC_PeriphCLKInitTypeDef  	PeriphClkInit;
ADC_ChannelConfTypeDef 		ADCChConfig;
GPIO_InitTypeDef 			GPIO_InitStruct_Light;
uint16_t 					ambientlight_factor, adc_raw;

/**
  * @brief  initialization of lightsensor
  * @note   None
  * @retval None
  */
void init_lightsensor(Alarmclock *alarmclock_param){
	/* init buffer for adc filtering */
	init_filter();
	/* safe address into alarmclock object */
	alarmclock_param->ambient_light_factor = &ambientlight_factor;
	/* init peripherals for lightsensor */
	init_gpio_lightsensor();
	init_dma_lightsensor();
	init_adc_lightsensor();
	init_timer_lightsensor();
	/* start adc */
	HAL_TIM_Base_Start(&TIM3_Handle);
	HAL_ADC_Start_DMA(&ADC_Handle, &adc_raw, 1);
}

/**
  * @brief  initialization of gpio for adc
  * @note   None
  * @retval None
  */
void init_gpio_lightsensor(){
	/* init gpio input pin as analog*/
	__HAL_RCC_GPIOA_CLK_ENABLE();								//enable clock on the bus
	GPIO_InitStruct_Light.Pin 		= (uint16_t)0x0080U; 		// select 8th pin  (PA7)
	GPIO_InitStruct_Light.Mode 		= GPIO_MODE_ANALOG; 		// configure pins as analog input
	GPIO_InitStruct_Light.Pull 		= GPIO_NOPULL;				// no pull up or down resistors
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct_Light);				// setting GPIO registers
}

/**
  * @brief  initialization of gpio for adc
  * @note   None
  * @retval None
  */
void init_timer_lightsensor(){
	/* enable TIM3 clock */
	__HAL_RCC_TIM3_CLK_ENABLE();

	/* for cc configuration */
	TIM_MasterConfigTypeDef sMasterConfig;

	/* initialize TIM3 peripheral as follows:
		+ Period = 20 ms
		+ Prescaler = 30999
		+ ClockDivision = 0
		+ Counter direction = Up
	 */
	TIM3_Handle.Instance 				= TIM3;
	TIM3_Handle.Init.Prescaler         	= 30999;			// divide tim clock with 30999 to get 1 kHz (1 ms) --- (32 Mhz / 1 kHz) - 1 = 30999
	TIM3_Handle.Init.Period            	= 20;				// count up to 20 to get 50 Hz
	TIM3_Handle.Init.ClockDivision     	= 0;
	TIM3_Handle.Init.CounterMode      	= TIM_COUNTERMODE_UP;
	if (HAL_TIM_Base_Init(&TIM3_Handle) != HAL_OK){
		/* stay here if an error happened */
	}

	/* clear interrupt pending bits */
	__HAL_TIM_CLEAR_IT(&TIM3_Handle, TIM_IT_UPDATE);

	/* set counter register to 0 */
	(&TIM3_Handle)->Instance->CNT = 0;

	sMasterConfig.MasterOutputTrigger 	= TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode 		= TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&TIM3_Handle, &sMasterConfig) != HAL_OK){
		//error
	}
}

/**
  * @brief  initialization of dma between adc and memory
  * @note   None
  * @retval None
  */
void init_dma_lightsensor(){
	/* activate bus on which dma1 is connected */
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA1 Channel2 configuration ----------------------------------------------*/
	DMA_Handle.Instance 					= DMA1_Channel1;
	DMA_Handle.Init.Direction 				= DMA_PERIPH_TO_MEMORY;
	DMA_Handle.Init.PeriphInc 				= DMA_PINC_DISABLE;
	DMA_Handle.Init.MemInc 					= DMA_MINC_ENABLE;
	DMA_Handle.Init.Mode 					= DMA_CIRCULAR;
	DMA_Handle.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_HALFWORD;
	DMA_Handle.Init.MemDataAlignment 		= DMA_MDATAALIGN_HALFWORD;
	DMA_Handle.Init.Priority 				= DMA_PRIORITY_HIGH;
	HAL_DMA_DeInit(&DMA_Handle);
	if(HAL_DMA_Init(&DMA_Handle) != HAL_OK){
		while(1){
			//error
		}
	}

    __HAL_LINKDMA(&ADC_Handle,DMA_Handle,DMA_Handle);
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

/**
  * @brief  initialization of gpio for adc
  * @note   None
  * @retval None
  */
void init_adc_lightsensor(){
	/* Enable and set up clock of ADCx peripheral */
	__HAL_RCC_ADC1_CLK_ENABLE();

	/* configure adc prescaler to meet 12 mhz for the adc clock (14 mhz is the max) */
	PeriphClkInit.PeriphClockSelection 			= RCC_PERIPHCLK_ADC;
	PeriphClkInit.AdcClockSelection 			= RCC_CFGR_ADCPRE_DIV8;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

	/* configure adc */
	ADC_Handle.Instance 						= ADC1;
	ADC_Handle.Init.ContinuousConvMode 			= DISABLE;					// continuous mode enabled = permanent sampling and converting
	ADC_Handle.Init.DiscontinuousConvMode 		= DISABLE;
	ADC_Handle.Init.DataAlign 					= ADC_DATAALIGN_RIGHT;
	ADC_Handle.Init.ScanConvMode 				= ADC_SCAN_DISABLE;			// Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1)
	ADC_Handle.Init.ExternalTrigConv 			= ADC1_2_EXTERNALTRIG_T3_TRGO;
	HAL_ADC_Init(&ADC_Handle);

	/* Channel configuration */
	ADCChConfig.Channel 						= ADC_CHANNEL_7;
	ADCChConfig.Rank 							= ADC_REGULAR_RANK_1;
	ADCChConfig.SamplingTime 					= ADC_SAMPLETIME_28CYCLES_5;
	HAL_ADC_ConfigChannel(&ADC_Handle, &ADCChConfig);

	/* calibration */
	while(HAL_ADCEx_Calibration_Start(&ADC_Handle) != HAL_OK);

	/* NVIC configuration for ADC interrupt */
	/* Priority: high-priority */
	HAL_NVIC_SetPriority(ADC1_2_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
}

/**
  * @brief  start analog to digital conversion
  * @note   None
  * @retval None
  */
void start_lightsensor_adc_conversion(){
	HAL_ADC_Start(&ADC_Handle);
	HAL_Delay(1);
}

/**
  * @brief  stop analog to digital conversion
  * @note   None
  * @retval None
  */
void stop_lightsensor_adc_conversion(){
	HAL_ADC_Stop(&ADC_Handle);
}

/**
  * @brief  get recent adc conversion value
  * @note   None
  * @retval None
  */
void get_lightsensor_adc_conversion(uint32_t *adc_conversion){
	*adc_conversion = HAL_ADC_GetValue(&ADC_Handle);
}

/* Callback function for the adc interrupt */
/**
 * @brief adc conversion complete callback
 * @param ADC_HandleTypeDef* hadc
 * @retval None
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	uint32_t array_cumulus = 0;
	uint16_t array_averaged = 0;
	//static const uint16_t schmitt_base_th = 2;
	static uint16_t schmitt_ex_th = 2;

	/* init the filter with zeros */
	for(uint16_t i = BUFFERSIZE-1; i>0; i--){
		adc_buffer[i] = adc_buffer[i-1];
		array_cumulus += adc_buffer[i];
	}
	adc_buffer[0] = adc_raw;
	array_cumulus += adc_buffer[0];

	/* divide by 64 */
	array_averaged = array_cumulus >> 6;

	/* notice! a pointer from the alarmclock typpdef points to the variable "ambientlight_factor" */
	if(array_averaged <= schmitt_ex_th){
		ambientlight_factor = 1;
		schmitt_ex_th = 2;//schmitt_base_th + 2;
	}else{
		ambientlight_factor = 24;
		schmitt_ex_th = 0;//schmitt_base_th - 2;
	}
}

/**
  * @brief  initialization of the filter for adc values
  * @note   None
  * @retval None
  */
void init_filter(){
	/* init the filter with zeros */
	for(uint16_t i = 0; i<BUFFERSIZE; i++){
		adc_buffer[i] = 0;
	}
}
