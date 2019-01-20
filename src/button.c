/*
 * Autor: Nico Korn
 * Date: 15.05.2018
 * Firmware for a alarmlcock with custom made STM32F103 microcontroller board.
 *  *
 * Copyright (c) 2018 Nico Korn
 *
 * buttons.c this module contents init and functions for push and switch buttons.
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

/* global variables */
TIM_HandleTypeDef	TIM1_Handle;

/* private variables */
static uint16_t 				push_event_count;
static uint16_t 				button_pin, button_pin_temp;
static uint16_t 				wait_for_double_click = 0;
static uint32_t 				low_or_high_active_button;

/**
  * @brief  initialization of touch and switch buttons
  * @note   None
  * @retval None
  */
void init_buttons(){
	/* init push button gpio */
	__HAL_RCC_GPIOB_CLK_ENABLE();																		//enable clock on the bus
	GPIO_InitTypeDef GPIO_InitStruct_Touch_BTN;
	GPIO_InitStruct_Touch_BTN.Pin 		= BUTTON_MODE | BUTTON_PLUS | BUTTON_MINUS | BUTTON_SNOOZE; 	// select pin 1,2,3,13
	GPIO_InitStruct_Touch_BTN.Mode 		= GPIO_MODE_IT_FALLING; 									// configure pins for pp o,utput
	GPIO_InitStruct_Touch_BTN.Pull 		= GPIO_NOPULL;													// state clear because line either set up to 3.3V or down to GND
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct_Touch_BTN);													// setting GPIO registers

	/* init switch button gpio */
	GPIO_InitTypeDef GPIO_InitStruct_Switch_BTN;
	GPIO_InitStruct_Switch_BTN.Pin 		= SWITCH_ALARM; 												// select pin 11
	GPIO_InitStruct_Switch_BTN.Mode 	= GPIO_MODE_IT_RISING_FALLING; 									// configure pins for IT input
	GPIO_InitStruct_Switch_BTN.Pull 	= GPIO_NOPULL;													// state clear because line either set up to 3.3V or down to GND
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct_Switch_BTN);													// setting GPIO registers

	/* Enable and set EXTI lines Interrupt */
	HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
	HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI1_IRQn);
	HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

	init_button_timer();
}

/**
  * @brief  Initialization of timer for buttons. If a button is pushed for a longer time,
  * button events shall be fired based on a timer period, until the button is being released.
  * The longer the button is pushed, the shorter the timer period which is firing button events
  * shall be.
  * @param  void
  * @retval None
  */
void init_button_timer(void){
	/* enable TIM1 clock */
	__HAL_RCC_TIM1_CLK_ENABLE();

	/* initialize TIM1 peripheral */
	TIM1_Handle.Instance 				= TIM1;
	TIM1_Handle.Init.Prescaler			= 35999;	// divide tim clock with 31999 to get 1 kHz (1 ms) --- (72 Mhz / 2 kHz) - 1 = 35999: 0.5ms per tick
	TIM1_Handle.Init.Period				= 1000;		// at the beginning 500 ms period
	TIM1_Handle.Init.ClockDivision		= 0;
	TIM1_Handle.Init.CounterMode		= TIM_COUNTERMODE_UP;
	if (HAL_TIM_Base_Init(&TIM1_Handle) != HAL_OK){
		/* stay here if an error happened */
	}

	/* clear interrupt pending bits */
	__HAL_TIM_CLEAR_IT(&TIM1_Handle, TIM_IT_UPDATE);

	/* set counter register to 0 */
	(&TIM1_Handle)->Instance->CNT = 0;

	/* Enable and set TIM1 Interrupt */
	HAL_NVIC_SetPriority(TIM1_UP_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);
}

/**
  * @brief  enables/disables all button irq's
  * @param  SET/RESET
  * @retval None
  */
void set_button_irq(FunctionalState button_irq){
	if(button_irq != DISABLE){
		/* clear interrupt pending bits */
		__HAL_GPIO_EXTI_CLEAR_IT(BUTTON_MODE | BUTTON_PLUS | BUTTON_MINUS | BUTTON_SNOOZE | SWITCH_ALARM);
		/* Enable and set EXTI lines Interrupt */
		HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(EXTI0_IRQn);
		HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(EXTI1_IRQn);
		HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(EXTI2_IRQn);
		HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	}else{
		HAL_NVIC_DisableIRQ(EXTI0_IRQn);
		HAL_NVIC_DisableIRQ(EXTI1_IRQn);
		HAL_NVIC_DisableIRQ(EXTI2_IRQn);
		HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
	}
}

/**
 * @brief EXTI line detection callbacks. Here used to safe pin events into a queue.
 * @param GPIO_Pin: Specifies the pins connected EXTI line
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	button_pin_temp = GPIO_Pin;

	/* stop any display animations for quick reaction */
	WS2812_stop_animation();

	/* check if tim1 is still running, to identify a double click
	 * if not, then start a new pushed button process */
	if(!wait_for_double_click){
		/* set push event count to 0, as the timer will be started through a button it event */
		push_event_count = 0;
		/* put GPIO_Pin into a variable with module wide scope */
		button_pin = GPIO_Pin;
		/* queue event */
		queue_event(button_pin);
		/*fgwefwwf*/
		wait_for_double_click = 1;
		/* start timer for button keep pushed event, except it is a switch */
		if(button_pin != SWITCH_ALARM){
			button_TIM1_start();
		}
	}else{
		/* If the snooze button has been pushed a second time during the timer window
		 * load it as negation of BUTTON_SNOOZE into the queue. The negation classifies
		 * a double click action */
		if(button_pin == BUTTON_SNOOZE){	//snooze button has double click action
			button_pin = ~button_pin;			//negation to identify the button event as double click
			/* queue event */
			queue_event(button_pin);
			/* stop the timer */
			button_TIM1_stop();
			/* reset double click */
			wait_for_double_click = 0;
		}else{
			/* set push event count to 0, as the timer will be started through a button it event */
			push_event_count = 0;
			/* put GPIO_Pin into a variable with module wide scope */
			button_pin = GPIO_Pin;
			/* queue event */
			queue_event(button_pin);
			/* stop the timer */
			button_TIM1_stop();
			/* start timer for button keep pushed event */
			if(button_pin != SWITCH_ALARM){
				button_TIM1_start();
			}
		}

	}
}

/**
  * @brief  callback function for the TIM1 interrupt
  * @param  TIM_HandleTypeDef
  * @retval None
  */
void BUTTON_TIM1_Callback(void){
	if(button_pin == BUTTON_SNOOZE){
		low_or_high_active_button = GPIO_PIN_SET;
	}else{
		low_or_high_active_button = GPIO_PIN_RESET;
	}

	/* if the button still is pushed on this timer 1 callback, then deploy an button pushed event
	 * into the queue. If the button is released, wait in a window for a following push to which is then
	 * identified as a double click. If not a second button push occurs during this window, the
	 * timer will be stopped.
	 */
	if((GPIOB->IDR & button_pin) != (uint32_t)low_or_high_active_button){	// is the button still pushed?
		/* Set the auto-reload value */
		///(&TIM1_Handle)->Instance->ARR = 500; //250ms
		if(wait_for_double_click == 1){
			button_TIM1_stop();
			clear_event_queue();
			wait_for_double_click = 0;
		}
	}else{
		/* put button event into queue */
		queue_event(button_pin);
		/* in function to the push event count change the period */
		if(push_event_count >= 0 && push_event_count < 6){	//start with 500 ms periods for 6 push_event_counts
			/* Set the auto-reload value */
			(&TIM1_Handle)->Instance->ARR = 500;
			push_event_count++;
		}else if(push_event_count >= 6 && push_event_count < 21){
			/* Set the auto-reload value */
			(&TIM1_Handle)->Instance->ARR = 250;
			push_event_count++;
		}else if(push_event_count >= 21){
			/* Set the auto-reload value */
			(&TIM1_Handle)->Instance->ARR = 125;
			push_event_count++;
		}else{
			while(1){
				//error
			}
		}
	}
}

/**
  * @brief  timer start function
  * @param  TIM_HandleTypeDef
  * @retval None
  */
void button_TIM1_start(void){
	/* set counter register to 0 */
	(&TIM1_Handle)->Instance->CNT = 0;
	/* Set the auto-reload value */
	(&TIM1_Handle)->Instance->ARR = 1000;
	/* start push event count timer */
	HAL_TIM_Base_Start_IT(&TIM1_Handle);
}

/**
  * @brief  timer stop function
  * @param  TIM_HandleTypeDef
  * @retval None
  */
void button_TIM1_stop(void){
	/* stop buzzer time */
	HAL_TIM_Base_Stop_IT(&TIM1_Handle);
	/* set counter register to 0 */
	(&TIM1_Handle)->Instance->CNT = 0;
}
