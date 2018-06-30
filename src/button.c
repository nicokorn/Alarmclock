/*
 * Autor: Nico Korn
 * Date: 29.01.2018
 * Firmware for the STM32F103 Microcontroller to work with WS2812b leds.
 *  *
 * Copyright (c) 2017 Nico Korn
 *
 * buttons.c this module contents button init and functions.
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

#include "button.h"
#include "stm32f1xx.h"

/**
  * @brief  initialization of touch buttons
  * @note   None
  * @retval None
  */
void init_buttons(){
	/* init button gpio */
	__HAL_RCC_GPIOB_CLK_ENABLE();																	//enable clock on the bus
	GPIO_InitTypeDef GPIO_InitStruct_Button;
	GPIO_InitStruct_Button.Pin = 	BUTTON_MODE | BUTTON_PLUS | BUTTON_MINUS | BUTTON_SNOOZE; 	// select pin 1,2,3,13
	GPIO_InitStruct_Button.Mode = 	GPIO_MODE_IT_FALLING; 											// configure pins for pp o,utput
	GPIO_InitStruct_Button.Pull = 	GPIO_NOPULL;													// state clear because line either set up to 3.3V or down to GND
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct_Button);													// setting GPIO registers

	/* Enable and set EXTI lines Interrupt */
	HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
	HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI1_IRQn);
	HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}
