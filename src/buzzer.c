/*
 * Autor: Nico Korn
 * Date: 29.01.2018
 * Firmware for the STM32F103 Microcontroller to work with WS2812b leds.
 *  *
 * Copyright (c) 2017 Nico Korn
 *
 * buzzer.c this module contents buzzer init and functions.
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

#include "buzzer.h"
#include "ws2812.h"
#include "stm32f1xx.h"

/* global variables */
/* Configuration struct for the timer. This variable needs to be declared global
 * because it's also used in the interrupt handler module */
TIM_HandleTypeDef    		TIM3_Handle;

/**
  * @brief  initialization of gpio output for the buzzer
  * @note   None
  * @retval None
  */
void init_buzzer(uint16_t beep_period){
	/* init button gpio */
	__HAL_RCC_GPIOB_CLK_ENABLE();									//enable clock on the bus
	GPIO_InitTypeDef GPIO_InitStruct_Buzzer;
	GPIO_InitStruct_Buzzer.Pin = 		BUZZER_PIN; 				// select 12th pin (PB11)
	GPIO_InitStruct_Buzzer.Mode = 		GPIO_MODE_OUTPUT_PP; 		// configure as push pull outputt
	GPIO_InitStruct_Buzzer.Speed = 		GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct_Buzzer);					// setting GPIO registers

	/* init buzzer timer */
	init_buzzer_timer(beep_period);
}

/**
  * @brief  initialisation of peripherals for the timer including enabling
  * 		timer interrupts.
  * @param  beep_period - buzzer beep period in ms
  * @retval None
  */
void init_buzzer_timer(uint16_t beep_period){
	/* enable TIM3 clock */
	__HAL_RCC_TIM3_CLK_ENABLE();

	/* initialize TIM3 peripheral as follows:
		+ Period = blink_period / 2 (e.g. to have an standard output frequency equal to 2 HZ)
		+ Prescaler = 0
		+ ClockDivision = 0
		+ Counter direction = Up
	 */
	TIM3_Handle.Instance = TIM3;
	TIM3_Handle.Init.Prescaler         = 30999;				// divide tim clock with 30999 to get 1 kHz (1 ms) --- (32 Mhz / 1 kHz) - 1 = 30999
	TIM3_Handle.Init.Period            = beep_period / 2;	// divided by 2 because every timer update event (e.g. 2 Hz = 500 ms) the led shall toggle
	TIM3_Handle.Init.ClockDivision     = 0;
	TIM3_Handle.Init.CounterMode       = TIM_COUNTERMODE_UP;
	if (HAL_TIM_Base_Init(&TIM3_Handle) != HAL_OK){
		/* stay here if an error happened */
	}

	/* clear interrupt pending bits */
	__HAL_TIM_CLEAR_IT(&TIM3_Handle, TIM_IT_UPDATE);

	/* set counter register to 0 */
	(&TIM3_Handle)->Instance->CNT = 0;

	/* Enable and set TIM3 Interrupt */
	HAL_NVIC_SetPriority(TIM3_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

/**
  * @brief  callback function for the TIM3 interrupt
  * @param  TIM_HandleTypeDef
  * @retval None
  */
void BUZZER_TIM3_callback(){
	/* toggle buzzer pin */
	GPIOB->ODR ^= BUZZER_PIN;
}

/**
  * @brief  buzzer start function
  * @param  TIM_HandleTypeDef
  * @retval None
  */
void buzzer_start(){
	/* set counter register to 0 */
	(&TIM3_Handle)->Instance->CNT = 0;
	/* start buzzer time */
	HAL_TIM_Base_Start_IT(&TIM3_Handle);
	/* switch on buzzer */
	GPIOB->BSRR ^= BUZZER_PIN;
}

/**
  * @brief  buzzer stop function
  * @param  TIM_HandleTypeDef
  * @retval None
  */
void buzzer_stop(){
	/* stop buzzer time */
	HAL_TIM_Base_Stop_IT(&TIM3_Handle);
	/* switch off buzzer */
	GPIOB->BRR ^= BUZZER_PIN;
}
