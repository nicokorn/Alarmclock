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
#define SETUP_BLINKING_PERIOD	1000 // in ms
#define BUZZER_BUZZ_PERIOD		1000 // in ms

/* variables */
extern uint8_t 			WS2812_TC;							// led transmission flag
static uint8_t 			red,green,blue;						// led color
static Wordclock_Mode 	mode, read_mode, old_read_mode;		// mode states for the fsm

/* function prototypes */
void SystemClock_Config(void);
void refresh_clock_display(void);
void alarm_lock(FunctionalState state);
void setup_blinking(Wordclock_Mode mode);
void get_preferences(uint8_t *red, uint8_t *green, uint8_t *blue);
void set_preferences(uint8_t *red, uint8_t *green, uint8_t *blue);

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
	init_lightsensor();

	/* load color preferences from the BKP register */
	get_preferences(&red, &green, &blue);

	/* initialize clock (also load time information from BKP register) */
	init_clock();

	/* init system mode */
	mode = MODE_TIME_CLOCK;

	/* test leds */
	WS2812_led_test();

	while (1){
		/* read last valid mode - needs to be read here to save synchronized behavior */
		read_mode = mode;
		/* finite state machine */
		switch(read_mode){
			case MODE_TIME_CLOCK:			/* unlock alarm */
											alarm_lock(UNLOCKED);
											/* refresh clock */
											refresh_clock_display();
											read_mode = MODE_TIME_CLOCK;
			break;
			case MODE_TIME_SET_CLOCK_h:		/* lock alarm */
											alarm_lock(LOCKED);
											/* let the hours blink */
											setup_blinking(read_mode);
											read_mode = MODE_TIME_SET_CLOCK_h;
			break;
			case MODE_TIME_SET_CLOCK_min:	/* lock alarm */
											alarm_lock(LOCKED);
											/* let the minutes blink */
											setup_blinking(read_mode);
											read_mode = MODE_TIME_SET_CLOCK_min;
			break;
			case MODE_TIME_SET_ALARM_h:		/* lock alarm */
											alarm_lock(LOCKED);
											/*let the hours blink*/
											setup_blinking(read_mode);
											read_mode = MODE_TIME_SET_ALARM_h;
			break;
			case MODE_TIME_SET_ALARM_min:	/* lock alarm */
											alarm_lock(LOCKED);
											/* let the minutes blink */
											setup_blinking(read_mode);
											read_mode = MODE_TIME_SET_ALARM_min;
			break;
		}
	}
	/* get ambient light to control led light strength */
	// to do
	/* save recent mode into old mode, to detect further mode changes */
	old_read_mode = read_mode;
	/* wait 1 ms */
	HAL_Delay(1);
}

/* Callback functions for the interrupts -------------------------------------*/
/**
 * @brief EXTI line detection callbacks.
 * @param GPIO_Pin: Specifies the pins connected EXTI line
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	/* functional part of the finite state machine, graphical part is implemented in the main routine */
	switch(mode){
		/* normal clock mode with displaying time, ability to change color, snooze alarm and switch on/off alarm */
		case MODE_TIME_CLOCK:			switch(GPIO_Pin){
											case BUTTON_MODE: 	/* set next mode */
																mode++;
											break;
											case BUTTON_PLUS:	/* color up */
																WS2812_color_wheel_plus(&red, &green, &blue);
											break;
											case BUTTON_MINUS:	/* color down */
																WS2812_color_wheel_minus(&red, &green, &blue);
											break;
											case BUTTON_SNOOZE:	/* stop alarm */
																buzzer_stop();
											break;
											case SWITCH_ALARM:	/* switch on/off alarm */
																if(((GPIOB->IDR & (uint32_t)SWITCH_ALARM) >> 9) == (uint32_t)RESET){
																	/* disable alarm */
																	alarm_IT(DISABLE);
																}else if(((GPIOB->IDR & (uint32_t)SWITCH_ALARM) >> 9) == (uint32_t)SET){
																	/* enable alarm */
																	alarm_IT(ENABLE);
																};
											break;
										}
		break;
		/* in this mode, the hours of the clock can be set, also alarm can be switched and snoozed */
		case MODE_TIME_SET_CLOCK_h:		switch(GPIO_Pin){
											case BUTTON_MODE: 	/* set next mode */
																mode++;
											break;
											case BUTTON_PLUS:	/* clock hour up */
																led_clock_hour_plus();
											break;
											case BUTTON_MINUS:	/* clock hour down */
																led_clock_hour_minus();
											break;
											case BUTTON_SNOOZE:;
											break;
											case SWITCH_ALARM:;
											break;
										}
		break;
		/* in this mode, the minutes of the clock can be set, also alarm can be switched and snoozed */
		case MODE_TIME_SET_CLOCK_min:	switch(GPIO_Pin){
											case BUTTON_MODE: 	/* set next mode */
																mode++;
											break;
											case BUTTON_PLUS:	/* clock minute up */
																led_clock_minute_plus();
											break;
											case BUTTON_MINUS:	/* clock minute down */
																led_clock_minute_minus();
											break;
											case BUTTON_SNOOZE:;
											break;
											case SWITCH_ALARM:;
											break;
										}
		break;
		/* in this mode, the hours of the alarm can be set, also alarm can be switched and snoozed */
		case MODE_TIME_SET_ALARM_h:		switch(GPIO_Pin){
											case BUTTON_MODE: 	/* set next mode */
																mode++;
											break;
											case BUTTON_PLUS:	/* alarm hour up */
																led_alarm_hour_plus();
											break;
											case BUTTON_MINUS:	/* alarm hour down */
																led_alarm_hour_minus();
											break;
											case BUTTON_SNOOZE:;
											break;
											case SWITCH_ALARM:;
											break;
										}
		break;
		/* in this mode, the minutes of the alarm can be set, also alarm can be switched and snoozed */
		case MODE_TIME_SET_ALARM_min:	switch(GPIO_Pin){
											case BUTTON_MODE: 	/* set next mode */
																mode++;
											break;
											case BUTTON_PLUS:	/* alarm minute up */
																led_alarm_minute_plus();
											break;
											case BUTTON_MINUS:	/* alarm minute down */
																led_alarm_minute_minus();
											break;
											case BUTTON_SNOOZE:;
											break;
											case SWITCH_ALARM:;
											break;
										}
		break;
	}
}

/**
  * @brief  this function is used to refresh time and background of the wordclock
  * @param  None
  * @retval None
  */
void refresh_clock_display(){
	/* erase frame buffer */
	WS2812_clear_buffer();
	/* write time into frame buffer */
	draw_time(&red, &green, &blue);
	/* wait for the data transmission to the led's to be ready */
	while(!WS2812_TC);
	/* send frame buffer to the leds */
	sendbuf_WS2812();
}

/**
  * @brief  this function lets hours or minutes blinking on the display
  * @param  None
  * @retval None
  */
void setup_blinking(Wordclock_Mode mode){
	/* erase frame buffer */
	WS2812_clear_buffer();
	/* write setup time into frame buffer */
	if(mode == MODE_TIME_SET_CLOCK_h || mode == MODE_TIME_SET_ALARM_h){
		draw_hh_mm(MINUTES, mode, &red, &green, &blue);
		if(HAL_GetTick() % SETUP_BLINKING_PERIOD > 0 && HAL_GetTick() % SETUP_BLINKING_PERIOD < 500){
			draw_hh_mm(HOURS, mode, &red, &green, &blue);
		}
	}else if(mode == MODE_TIME_SET_CLOCK_min || mode == MODE_TIME_SET_ALARM_min){
		draw_hh_mm(HOURS, mode, &red, &green, &blue);
		if(HAL_GetTick() % SETUP_BLINKING_PERIOD > 0 && HAL_GetTick() % SETUP_BLINKING_PERIOD < 500){
			draw_hh_mm(MINUTES, mode, &red, &green, &blue);
		}
	}

	/* wait for the data transmission to the led's to be ready */
	while(!WS2812_TC);
	/* send frame buffer to the leds */
	sendbuf_WS2812();
}

/**
  * @brief  this function is used to get color from the backup register BKP_DR
  * @param  None
  * @retval None
  */
void get_preferences(uint8_t *red, uint8_t *green, uint8_t *blue){
	  uint32_t backupregister = 0U;
	  uint32_t backupregister_value = 0U;
	  uint32_t backup_register_mask = 0x000000FF;

	  /* get reset variable */
	  backupregister = (uint32_t)BKP_BASE;
	  backupregister += (1 * 4U);
	  backupregister_value = (*(__IO uint32_t *)(backupregister)) & BKP_DR1_D;

	  /* if variable = 0x32F2, BKP registers have saved preferences */
	  if(backupregister_value != 0x32F2){
			/* init default color */
			*red = 0x00;
			*green = 0x00;
			*blue = 0xff;
	  }else{
		  /* get red color */
		  backupregister = (uint32_t)BKP_BASE;
		  backupregister += (2 * 4U);

		  *red = (*(__IO uint32_t *)(backupregister)) & (uint32_t)backup_register_mask;
		  backupregister = 0U;

		  /* get green color */
		  backupregister = (uint32_t)BKP_BASE;
		  backupregister += (3 * 4U);

		  *green = (*(__IO uint32_t *)(backupregister)) & (uint32_t)backup_register_mask;
		  backupregister = 0U;


		  /* get blue color */
		  backupregister = (uint32_t)BKP_BASE;
		  backupregister += (4 * 4U);

		  *blue = (*(__IO uint32_t *)(backupregister)) & (uint32_t)backup_register_mask;
		  backupregister = 0U;
	  }
}

/**
  * @brief  this function is used to set color in the backup register BKP_DR
  * @param  None
  * @retval None
  */
void set_preferences(uint8_t *red, uint8_t *green, uint8_t *blue){
	  uint32_t tmp = 0U;
	  uint32_t backup_register_mask = 0x000000FF;

	  /* set red color */
	  tmp = (uint32_t)BKP_BASE;
	  tmp += (2 * 4U);

	  *(__IO uint32_t *) tmp = ( *(__IO uint32_t *) red & backup_register_mask);
	  tmp = 0U;

	  /* set green color */
	  tmp = (uint32_t)BKP_BASE;
	  tmp += (3 * 4U);

	  *(__IO uint32_t *) tmp =  ( *(__IO uint32_t *) green & backup_register_mask);
	  tmp = 0U;

	  /* set blue color */
	  tmp = (uint32_t)BKP_BASE;
	  tmp += (4 * 4U);

	  *(__IO uint32_t *) tmp = ( *(__IO uint32_t *) blue & backup_register_mask);
	  tmp = 0U;
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
