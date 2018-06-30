/*
 * Autor: Nico Korn
 * Date: 04.02.2018
 * Firmware for the STM32F103 Microcontroller to work with WS2812b leds.
 *  *
 * Copyright (c) 2018 Nico Korn
 *
 * main.c this module contents the main routine with fsm
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* defines */
#define BUZZER_BUZZ_PERIOD			1000 // in ms
#define EVENT_QUEUE_LENGTH			11 // length of event buffer

/* variables */
static uint16_t			event_queue[EVENT_QUEUE_LENGTH];
static uint16_t			queue_index;
static Queue_Lock		queue_lock;
static Alarmclock		alarmclock;

/* function prototypes */
void SystemClock_Config(void);
uint16_t unqeue_next_event();
void init_event_queue();


/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void){

	/* STM32F103xG HAL library initialization:
       - Configure the Flash prefetch
       - Systick timer is configured by default as source of time base, but user 
         can eventually implement his proper time base source (a general purpose 
         timer for example or other time source), keeping in mind that Time base 
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
    */
	HAL_Init();

	/* Configure the system clock to 72 MHz */
	SystemClock_Config();

	/* initialize ws2812 peripherals */
	init_ws2812();

	/* initialize buttons */
	init_buttons();

	/* initialize buzzer */
	init_buzzer(BUZZER_BUZZ_PERIOD);

	/* initialize lightsensor */
	//init_lightsensor();

	/* initialize RTC and init mode */
	init_clock(&alarmclock);

	/* init event queue with end flag */
	init_event_queue();

	/* test leds */
	WS2812_led_test();

	while (1){
		/* check for new events */
		alarmclock.event = unqeue_next_event();
		/* finite state machine */
		switch(alarmclock.mode){
			case MODE_TIME_CLOCK:			switch(alarmclock.event){
												case BUTTON_MODE:	increment_mode(&alarmclock);
												break;
												case BUTTON_PLUS:	increment_clock_color(&alarmclock);
																	set_clock_preferences(&alarmclock);	// set preferences
												break;
												case BUTTON_MINUS:	decrement_clock_color(&alarmclock);
																	set_clock_preferences(&alarmclock);	// set preferences
												break;
												case BUTTON_SNOOZE: buzzer_stop();
												break;
												case SWITCH_ALARM:;
												break;
											}
											refresh_clock_display(&alarmclock);		// refresh clock
			break;
			case MODE_TIME_SET_CLOCK_h:		switch(alarmclock.event){
												case BUTTON_MODE:	increment_mode(&alarmclock);
												break;
												case BUTTON_PLUS:	led_clock_hour_plus(&alarmclock);
												break;
												case BUTTON_MINUS:	led_clock_hour_minus(&alarmclock);
												break;
												case BUTTON_SNOOZE:	buzzer_stop();
												break;
												case SWITCH_ALARM:;
												break;
											}
											setup_clock_blinking(&alarmclock);			// let the hours blink
			break;
			case MODE_TIME_SET_CLOCK_min:	switch(alarmclock.event){
												case BUTTON_MODE:	increment_mode(&alarmclock);
												break;
												case BUTTON_PLUS:	led_clock_minute_plus(&alarmclock);
												break;
												case BUTTON_MINUS:	led_clock_minute_minus(&alarmclock);
												break;
												case BUTTON_SNOOZE:	buzzer_stop();
												break;
												case SWITCH_ALARM:;
												break;
											}
											setup_clock_blinking(&alarmclock);		// let the minutes blink
			break;
			case MODE_TIME_SET_ALARM_h:		switch(alarmclock.event){
												case BUTTON_MODE:	increment_mode(&alarmclock);
												break;
												case BUTTON_PLUS:	led_alarm_hour_plus(&alarmclock);
																	set_clock_preferences(&alarmclock);	// set preferences
												break;
												case BUTTON_MINUS:	led_alarm_hour_minus(&alarmclock);
																	set_clock_preferences(&alarmclock);	// set preferences
												break;
												case BUTTON_SNOOZE:	buzzer_stop();
												break;
												case SWITCH_ALARM:;
												break;
											}
											setup_clock_blinking(&alarmclock);		// let the hours blink
			break;
			case MODE_TIME_SET_ALARM_min:	switch(alarmclock.event){
												case BUTTON_MODE:	increment_mode(&alarmclock);
												break;
												case BUTTON_PLUS:	led_alarm_minute_plus(&alarmclock);
																	set_clock_preferences(&alarmclock);	// set preferences
												break;
												case BUTTON_MINUS:	led_alarm_minute_minus(&alarmclock);
																	set_clock_preferences(&alarmclock);	// set preferences
												break;
												case BUTTON_SNOOZE:	buzzer_stop();
												break;
												case SWITCH_ALARM:;
												break;
											}
											setup_clock_blinking(&alarmclock);		// let the minutes blink
			break;
		}
		HAL_Delay(100);
	}
}

/* Callback function for the interrupts */
/**
 * @brief EXTI line detection callbacks.
 * @param GPIO_Pin: Specifies the pins connected EXTI line
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	uint16_t index = 0;
	uint16_t run_flag = 1;
	/* lock the queue */
	queue_lock = QLOCKED;
	/* save button event into event qeue */
	while(index < EVENT_QUEUE_LENGTH && run_flag == 1){
		if(event_queue[index] == END_OF_QEUE){
			event_queue[index] = GPIO_Pin;
			event_queue[index+1] = END_OF_QEUE;
			run_flag = 0;
		}else if(event_queue[EVENT_QUEUE_LENGTH-1] == END_OF_QEUE){
			//do nothing
			run_flag = 0;
		}
		index++;
	}
	queue_lock = QUNLOCKED;
}

/**
  * @brief  this function takes out recent events from the qeue
  * @param  None
  * @retval event
  */
uint16_t unqeue_next_event(){
	uint16_t latest_event;
	/* wait for unlocked queue */
	while(queue_lock);
	/* check for new events */
	latest_event = event_queue[0];
	/* shift all entities, because an event has been taken out of the qeue, except it's the END_OF_QEUE at index 0 */
	if(event_queue[0] != END_OF_QEUE){
		queue_index = 0;
		while(event_queue[queue_index+1] != END_OF_QEUE){
			event_queue[queue_index] = event_queue[queue_index+1];
			queue_index++;
		}
		event_queue[queue_index] = END_OF_QEUE;
	}
	return latest_event;
}

/**
  * @brief  this function initiates the queue
  * @param  None
  * @retval None
  */
void init_event_queue(){
	event_queue[0] = END_OF_QEUE;
	queue_lock = QUNLOCKED;
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 72000000
  *            HCLK(Hz)                       = 72000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 8000000
  *            HSE PREDIV1                    = 1
  *            PLLMUL                         = 9
  *            Flash Latency(WS)              = 2
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef clkinitstruct = {0};
  RCC_OscInitTypeDef oscinitstruct = {0};
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  oscinitstruct.OscillatorType  = RCC_OSCILLATORTYPE_HSE;
  oscinitstruct.HSEState        = RCC_HSE_ON;
  oscinitstruct.HSEPredivValue  = RCC_HSE_PREDIV_DIV1;
  oscinitstruct.PLL.PLLState    = RCC_PLL_ON;
  oscinitstruct.PLL.PLLSource   = RCC_PLLSOURCE_HSE;
  oscinitstruct.PLL.PLLMUL      = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&oscinitstruct)!= HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  clkinitstruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  clkinitstruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clkinitstruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;
  clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;  
  if (HAL_RCC_ClockConfig(&clkinitstruct, FLASH_LATENCY_2)!= HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
