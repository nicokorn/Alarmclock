/*
 * Autor: Nico Korn
 * Date: 15.05.2018
 * Firmware for a alarmlcock with custom made STM32F103 microcontroller board.
 *  *
 * Copyright (c) 2018 Nico Korn
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

/* Includes */
#include "buzzer.h"

/* defines */
#define BUZZER_BUZZ_PERIOD			2000 	// in ms
#define BUZZER_SNZ_TIME				60000 	// in 0.5ms
#define BUZZER_SNZ_ROUNDS			5 		// in rounds

/* global variables */
TIM_HandleTypeDef	TIM4_Handle;
TIM_HandleTypeDef	TIM3_Handle;

/* private variables */
static uint8_t 				snooze_counter;
static uint16_t 			beep_rounds;

/* private functions */
static void init_buzzer_timer(uint16_t beep_period);
static void init_snooze_timer(void);

/**
  * @brief  initialization of gpio output for the buzzer
  * @note   None
  * @retval None
  */
void init_buzzer(Alarmclock *alarmclock_param){
	/* init button gpio */
	__HAL_RCC_GPIOB_CLK_ENABLE();									//enable clock on the bus
	GPIO_InitTypeDef GPIO_InitStruct_Buzzer;
	GPIO_InitStruct_Buzzer.Pin 			= BUZZER_PIN; 				// select 12th pin (PB11)
	GPIO_InitStruct_Buzzer.Mode 		= GPIO_MODE_OUTPUT_PP; 		// configure as push pull outputt
	GPIO_InitStruct_Buzzer.Speed 		= GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct_Buzzer);					// setting GPIO registers

	/* get adress of the buzzer_state member */
	alarmclock_param->buzzer_state = BUZZER_RESET;

	/* init buzzer timer */
	init_buzzer_timer(BUZZER_BUZZ_PERIOD);

	/* init snooze timer */
	init_snooze_timer();

	/* set snooze counter to 0 */
	snooze_counter = 0;

	/* set beep rounds to 0 */
	beep_rounds = 0;
}

/**
  * @brief  initialisation of peripherals for the timer including enabling
  * 		timer interrupts.
  * @param  beep_period - buzzer beep period in ms
  * @retval None
  */
static void init_buzzer_timer(uint16_t beep_period){
	/* enable TIM4 clock */
	__HAL_RCC_TIM4_CLK_ENABLE();

	/* initialize TIM4 peripheral as follows:
		+ Period = beep_period / 2 (e.g. to have an standard output frequency equal to 2 HZ)
		+ Prescaler = 0
		+ ClockDivision = 0
		+ Counter direction = Up
	 */
	TIM4_Handle.Instance 				= TIM4;
	TIM4_Handle.Init.Prescaler			= 35999;				// divide tim clock with 35999 to get 1 kHz (1 ms) --- (36 Mhz / 1 kHz) - 1 = 35999
	TIM4_Handle.Init.Period				= (uint32_t)(beep_period / 2);		// divided by 2 because every timer update event (e.g. 2 Hz = 500 ms) the led shall toggle
	TIM4_Handle.Init.ClockDivision		= 0;
	TIM4_Handle.Init.CounterMode		= TIM_COUNTERMODE_UP;
	if (HAL_TIM_Base_Init(&TIM4_Handle) != HAL_OK){
		/* stay here if an error happened */
	}

	/* clear interrupt pending bits */
	__HAL_TIM_CLEAR_IT(&TIM4_Handle, TIM_IT_UPDATE);

	/* set counter register to 0 */
	(&TIM4_Handle)->Instance->CNT = 0;

	/* Enable and set TIM4 Interrupt */
	HAL_NVIC_SetPriority(TIM4_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(TIM4_IRQn);
}

/**
  * @brief  initialisation of peripherals for the timer including enabling
  * 		timer interrupts.
  * @param  None
  * @retval None
  */
static void init_snooze_timer(void){
	/* enable TIM3 clock */
	__HAL_RCC_TIM3_CLK_ENABLE();

	/* initialize TIM3 peripheral */
	TIM3_Handle.Instance 				= TIM3;
	TIM3_Handle.Init.Prescaler			= 35999;				// divide tim clock with 35999 to get 1 kHz (1 ms) --- (36 Mhz / 1 kHz) - 1 = 35999
	TIM3_Handle.Init.Period				= (uint32_t)BUZZER_SNZ_TIME;
	TIM3_Handle.Init.ClockDivision		= 0;
	TIM3_Handle.Init.CounterMode		= TIM_COUNTERMODE_UP;
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
  * @brief  callback function for the TIM4 interrupt
  * @param  TIM_HandleTypeDef
  * @retval None
  */
void BUZZER_TIM4_callback(Alarmclock *alarmclock_param){
	/* toggle buzzer pin */
	if(beep_rounds < 50){
		GPIOB->ODR ^= BUZZER_PIN;
		beep_rounds++;
		/* queue  buzzer event */
		queue_event(BUZZER_PIN);
	}else{
		/* stop buzzer time */
		HAL_TIM_Base_Stop_IT(&TIM4_Handle);
		/* switch off buzzer */
		GPIOB->BRR ^= BUZZER_PIN;
		/* reset beep rounds */
		beep_rounds = 0;
		/* reset buzzer state */
		alarmclock_param->buzzer_state = BUZZER_RESET;
	}
}

/**
  * @brief  buzzer start function
  * @param  TIM_HandleTypeDef
  * @retval None
  */
void buzzer_start(Alarmclock *alarmclock_param){
	/* set counter register to 0 */
	(&TIM4_Handle)->Instance->CNT = 0;
	/* start buzzer time */
	HAL_TIM_Base_Start_IT(&TIM4_Handle);
	/* switch on buzzer */
	GPIOB->BSRR ^= BUZZER_PIN;
	/* reset beep counter */
	beep_rounds = 0;
	/* set buzzer state */
	alarmclock_param->buzzer_state = BUZZER_SET;
	/* queue  buzzer event */
	queue_event(BUZZER_PIN);
}

/**
  * @brief  buzzer stop function
  * @param  TIM_HandleTypeDef
  * @retval None
  */
void buzzer_stop(Alarmclock *alarmclock_param){
	/* stop buzzer time */
	HAL_TIM_Base_Stop_IT(&TIM4_Handle);
	/* switch off buzzer */
	GPIOB->BRR ^= BUZZER_PIN;
	/* set buzzer state */
	alarmclock_param->buzzer_state = BUZZER_RESET;
}

/**
  * @brief  buzzer snooze function
  * @param  Alarmclock
  * @retval None
  */
void snooze(Alarmclock *alarmclock_param){
	/* if tim3 is running stop it */
	HAL_TIM_Base_Stop_IT(&TIM3_Handle);
	/* set buzzer state to snooze */
	alarmclock_param->snooze_state = SNOOZE_SET;
	/* set counter register to 0 */
	(&TIM3_Handle)->Instance->CNT = 0;
	/* start snooze timer */
	HAL_TIM_Base_Start_IT(&TIM3_Handle);
}

/**
  * @brief  buzzer reset function
  * @param  Alarmclock
  * @retval None
  */
void snooze_reset(Alarmclock *alarmclock_param){
	/* reset alarm */
	snooze_counter = 0;
	/* stop snooze timer */
	HAL_TIM_Base_Stop_IT(&TIM3_Handle);
	/* set counter to zero */
	(&TIM3_Handle)->Instance->CNT = 0;
	/* set buzzer state to reset */
	alarmclock_param->snooze_state = SNOOZE_RESET;
}

/**
  * @brief  snooze timer callback function for the TIM3 interrupt
  * @param  Alarmclock
  * @retval None
  */
void SNOOZE_TIM3_callback(Alarmclock *alarmclock_param){
	if(snooze_counter < alarmclock_param->snooze_duration*BUZZER_SNZ_ROUNDS*2){
		/* increment snooze counter */
		snooze_counter++;
		/* start buzzer */
		if(snooze_counter%(2*alarmclock_param->snooze_duration)==0){
			buzzer_start(alarmclock_param);
		}
	}else{
		snooze_reset(alarmclock_param);
	}
}
