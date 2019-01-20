/*
 * Autor: Nico Korn
 * Date: 15.05.2018
 * Firmware for a alarmlcock with custom made STM32F103 microcontroller board.
 *  *
 * Copyright (c) 2018 Nico Korn
 *
 * clock.c this module contents all functions regarding the alarmclock
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

#include "clock.h"

/* defines */
#define SETUP_CLOCK_BLINKING_PERIOD	1000 // in ms
/* uncomment this to use the development mode to have modes like displayed ambient light measurement */
//#define DEV_MODE


/* private variables */
static uint8_t				mode_count;
static uint8_t	 			h0, h1, m0, m1;
static uint8_t	 			h0_old, h1_old, m0_old, m1_old, init;
static uint8_t	 			h0_time_change_flag, h1_time_change_flag, m0_time_change_flag, m1_time_change_flag;
static int16_t 				h0_x_offset, h1_x_offset, m0_x_offset, m1_x_offset, x_offset;
static int8_t 				h0_y_offset, h1_y_offset, m0_y_offset, m1_y_offset, y_offset;
static uint8_t 				color_pattern[4][3] = {
									{0x09, 0x09, 0x09},	//white
									{0x09, 0x00, 0x00},	//red
									{0x00, 0x09, 0x00},	//green
									{0x00, 0x00, 0x09}	//blue
									};
static uint32_t				hal_tick_temp;



/* global variables */
extern uint8_t 				WS2812_TC;					// led transmission flag
RTC_HandleTypeDef 			RTC_Handle;

/**
  * @brief  initialization of words & rtc
  * @note   None
  * @retval None
  */
void init_RTC( Alarmclock *alarmclock_param){
	/* Configure RTC prescaler and RTC data registers */
	/* RTC configured as follow:
	- Asynch Prediv  = Calculated automatically by HAL */
	RTC_Handle.Instance = 			RTC;
	RTC_Handle.Init.AsynchPrediv = 	RTC_AUTO_1_SECOND;

	if (HAL_RTC_Init(&RTC_Handle) != HAL_OK){
	    /* Initialization Error */
	    //Error_Handler();
	}

	/* Read the Back Up Register 1 Data */
	if (HAL_RTCEx_BKUPRead(&RTC_Handle, RTC_BKP_DR1) != 0x32F2){
	    /* configure RTC calendar */
	    RTC_CalendarConfig(alarmclock_param);
	}else{
		/* Clear source Reset Flag */
	    __HAL_RCC_CLEAR_RESET_FLAGS();
	}

	/* set time changing effect flags */
	h0_time_change_flag = 0;
	h1_time_change_flag = 0;
	m0_time_change_flag = 0;
	m1_time_change_flag = 0;

	/* initiation flag for clock number startup effect */
	init = 1;

	/* set mode count for mode increment function */
	mode_count = 6;
	#ifdef DEV_MODE
	mode_count = 7;
	#endif

}

/**
  * @brief  shows alarmclock intro during startup
  * @note   None
  * @retval None
  */
void clock_intro(){
	uint8_t redtest = 0xff;
	uint8_t greentest = 0x00;
	uint8_t bluetest = 0x00;
	int8_t running_text_offset = -30;
	uint16_t ambient = 10;
	uint8_t	clock_background_framebuffer[ROW*COL*3];	//7 rows * 17 cols * 3 (RGB) = 363 --- separate frame buffer for background fx --- 1 array entry contents a color component information in 8 bit. 3 entries together = 1 RGB Information
	/* disable button irq */
	set_button_irq(DISABLE);
	for(uint16_t i = 0; i<440; i++){
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

		/* write text */
		draw_letter('p', -1-running_text_offset, 0, 0x00, 0x00, 0x00, &ambient);
		draw_letter('i', 3-running_text_offset, 0, 0x00, 0x00, 0x00, &ambient);
		draw_letter('x', 7-running_text_offset, 0, 0x00, 0x00, 0x00, &ambient);
		draw_letter('e', 11-running_text_offset, 0, 0x00, 0x00, 0x00, &ambient);
		draw_letter('l', 15-running_text_offset, 0, 0x00, 0x00, 0x00, &ambient);

		draw_letter('c', 22-running_text_offset, 0, 0x00, 0x00, 0x00, &ambient);
		draw_letter('l', 26-running_text_offset, 0, 0x00, 0x00, 0x00, &ambient);
		draw_letter('o', 30-running_text_offset, 0, 0x00, 0x00, 0x00, &ambient);
		draw_letter('c', 34-running_text_offset, 0, 0x00, 0x00, 0x00, &ambient);
		draw_letter('k', 38-running_text_offset, 0, 0x00, 0x00, 0x00, &ambient);

		/* refresh letter positions to create a running text */
		if(i%5 == 0){
			running_text_offset++;
		}

		sendbuf_WS2812();
		HAL_Delay(5);
	}
	/* enable button irq */
	set_button_irq(ENABLE);
}

/**
  * @brief  shows the mode on the display
  * @note   None
  * @retval None
  */
void draw_mode(Alarmclock *alarmclock_param){
	switch(alarmclock_param->mode){
		case MODE_TIME_SET_CLOCK_h:
									/* disable button irq */
									set_button_irq(DISABLE);
									/* write time setup on the display */
									draw_string("time setup", 0, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
									/* enable button irq */
									set_button_irq(ENABLE);
		break;
		case MODE_TIME_SET_ALARM_h:
									/* disable button irq */
									set_button_irq(DISABLE);
									/* write alarm setup on the display */
									draw_string("alarm setup", 0, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
									/* enable button irq */
									set_button_irq(ENABLE);
		break;
		case MODE_TIME_SET_ALARM_STYLE:
									/* disable button irq */
									set_button_irq(DISABLE);
									/* write alarm setup on the display */
									draw_string("choose alarm fx", 0, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
									/* enable button irq */
									set_button_irq(ENABLE);
		break;
		case MODE_TIME_SET_SNOOZE:
									/* disable button irq */
									set_button_irq(DISABLE);
									/* write alarm setup on the display */
									draw_string("snooze setup", 0, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
									/* enable button irq */
									set_button_irq(ENABLE);
		break;
		case MODE_TIME_LUX:
									/* disable button irq */
									set_button_irq(DISABLE);
									/* write alarm setup on the display */
									draw_string("lux mode", 0, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
									/* enable button irq */
									set_button_irq(ENABLE);
		break;
	}
}

/**
  * @brief  draws hour or minutes according last read time from rtc/alarm with incremented/decremented hours or minutes from the setup mode
  * @note   None
  * @retval None
  */
void draw_hh_mm(Time_Setup time, Alarmclock *alarmclock_param){
	/* depending on mode, last refreshed time data is load into h0h1:m0m1 or the alarm data into h0h1:m0m1 */
	if(alarmclock_param->mode == MODE_TIME_SET_CLOCK_h || alarmclock_param->mode == MODE_TIME_SET_CLOCK_min){
		/* process hours */
		h0 = alarmclock_param->timestructure.Hours / 10;
		h1 = alarmclock_param->timestructure.Hours % 10;
		/* process minutes */
		m0 = alarmclock_param->timestructure.Minutes / 10;
		m1 = alarmclock_param->timestructure.Minutes % 10;
	}else if(alarmclock_param->mode == MODE_TIME_SET_ALARM_h || alarmclock_param->mode == MODE_TIME_SET_ALARM_min){
		/* process hours */
		h0 = alarmclock_param->alarmstructure.AlarmTime.Hours / 10;
		h1 = alarmclock_param->alarmstructure.AlarmTime.Hours % 10;
		/* process minutes */
		m0 = alarmclock_param->alarmstructure.AlarmTime.Minutes / 10;
		m1 = alarmclock_param->alarmstructure.AlarmTime.Minutes % 10;
	}

	/* draw double point */
	draw_number(':', (uint16_t)7, (uint8_t)0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	if(time == HOURS){
		/* draw numbers into display buffer */
		/* number position h0 */
		h0_x_offset = 0;
		h0_y_offset = 0;
		switch(h0){
			case 0:	draw_number('0', h0_x_offset, h0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 1:	draw_number('1', h0_x_offset, h0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 2:	draw_number('2', h0_x_offset, h0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
		}
		/* number position h1 */
		h1_x_offset = 4;
		h1_y_offset = 0;
		switch(h1){
			case 0:	draw_number('0', h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 1:	draw_number('1', h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 2:	draw_number('2', h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 3:	draw_number('3', h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 4:	draw_number('4', h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 5:	draw_number('5', h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 6:	draw_number('6', h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 7:	draw_number('7', h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 8:	draw_number('8', h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 9:	draw_number('9', h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
		}
	}else if(time == MINUTES){
		/* number position m0 */
		m0_x_offset = 10;
		m0_y_offset = 0;
		switch(m0){
			case 0:	draw_number('0', m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 1:	draw_number('1', m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 2:	draw_number('2', m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 3:	draw_number('3', m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 4:	draw_number('4', m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 5:	draw_number('5', m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
		}
		/* number position m1 */
		m1_x_offset = 14;
		m1_y_offset = 0;
		switch(m1){
			case 0:	draw_number('0', m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 1:	draw_number('1', m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 2:	draw_number('2', m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 3:	draw_number('3', m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 4:	draw_number('4', m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 5:	draw_number('5', m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 6:	draw_number('6', m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 7:	draw_number('7', m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 8:	draw_number('8', m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 9:	draw_number('9', m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
		}
	}else{
		while(1){
			//error
		}
	}
}

/**
  * @brief  reads time from rtc and draws it into the frame buffer
  * @note   None
  * @retval None
  */
void draw_time(Alarmclock *alarmclock_param){
	HAL_RTC_GetTime(&RTC_Handle, &alarmclock_param->timestructure, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&RTC_Handle, &alarmclock_param->datestructure, RTC_FORMAT_BIN);

	/* hours and minutes local places: hh:mm = h0h1:m0m2 */
	/* process hours */
	h0 = alarmclock_param->timestructure.Hours / 10;
	h1 = alarmclock_param->timestructure.Hours % 10;

	/* process minutes */
	m0 = alarmclock_param->timestructure.Minutes / 10;
	m1 = alarmclock_param->timestructure.Minutes % 10;

	/* draw numbers into display buffer */
	/* number position h0 */
	if(h0 != h0_old || init){
		h0_time_change_flag = 1;
		h0_x_offset = 0;
		h0_y_offset = -8;
	}else if(h0 == h0_old && h0_time_change_flag == 1){
		if(h0_y_offset != 0){
			h0_y_offset++;
		}else if(h0_y_offset == 0){
			h0_time_change_flag = 0;
		}
	}else if(h0 == h0_old && h0_time_change_flag == 0){
		h0_x_offset = 0;
		h0_y_offset = 0;
	}
	switch(h0){
		case 0:	if(h0_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number('2', h0_x_offset, h0_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number('0', h0_x_offset, h0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 1:	if(h0_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number('0', h0_x_offset, h0_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number('1', h0_x_offset, h0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 2:	if(h0_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number('1', h0_x_offset, h0_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number('2', h0_x_offset, h0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
	}
	/* number position h1 */
	/* if a new time starts, start time change effect */
	if(h1 != h1_old || init){
		h1_time_change_flag = 1;
		h1_x_offset = 4;
		h1_y_offset = -8;
	}else if(h1 == h1_old && h1_time_change_flag == 1){
		if(h1_y_offset != 0){
			h1_y_offset++;
		}else if(h1_y_offset == 0){
			h1_time_change_flag = 0;
		}
	}else if(h1 == h1_old && h1_time_change_flag == 0){
		h1_x_offset = 4;
		h1_y_offset = 0;
	}
	switch(h1){
		case 0:	if(h1_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number('9', h1_x_offset, h1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number('0', h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 1:	if(h1_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number('0', h1_x_offset, h1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number('1', h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 2:	if(h1_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number('1', h1_x_offset, h1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number('2', h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 3:	if(h1_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number('2', h1_x_offset, h1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number('3', h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 4:	if(h1_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number('3', h1_x_offset, h1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number('4', h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 5:	if(h1_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number('4', h1_x_offset, h1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number('5', h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 6:	if(h1_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number('5', h1_x_offset, h1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number('6', h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 7:	if(h1_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number('6', h1_x_offset, h1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number('7', h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 8:	if(h1_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number('7', h1_x_offset, h1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number('8', h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 9:	if(h1_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number('8', h1_x_offset, h1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number('9', h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
	}
	/* second double point */
	x_offset = 7;
	y_offset = 0;
	if(alarmclock_param->timestructure.Seconds%2){
		draw_number(':', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	}
	/* number position m0 */
	if(m0 != m0_old || init){
		m0_time_change_flag = 1;
		m0_x_offset = 10;
		m0_y_offset = -8;
	}else if(m0 == m0_old && m0_time_change_flag == 1){
		if(m0_y_offset != 0){
			m0_y_offset++;
		}else if(m0_y_offset == 0){
			m0_time_change_flag = 0;
		}
	}else if(m0 == m0_old && m0_time_change_flag == 0){
		m0_x_offset = 10;
		m0_y_offset = 0;
	}
	switch(m0){
	case 0:	if(m0_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number('5', m0_x_offset, m0_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number('0', m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 1:	if(m0_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number('0', m0_x_offset, m0_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number('1', m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 2:	if(m0_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number('1', m0_x_offset, m0_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number('2', m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 3:	if(m0_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number('2', m0_x_offset, m0_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number('3', m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 4:	if(m0_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number('3', m0_x_offset, m0_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number('4', m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 5:	if(m0_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number('4', m0_x_offset, m0_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number('5', m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	}
	/* number position m1 */
	if(m1 != m1_old || init){
		m1_time_change_flag = 1;
		m1_x_offset = 14;
		m1_y_offset = -8;
	}else if(m1 == m1_old && m1_time_change_flag == 1){
		if(m1_y_offset != 0){
			m1_y_offset++;
		}else if(m1_y_offset == 0){
			m1_time_change_flag = 0;
		}
	}else if(m1 == m1_old && m1_time_change_flag == 0){
		m1_x_offset = 14;
		m1_y_offset = 0;
	}
	switch(m1){
	case 0:	if(m1_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number('9', m1_x_offset, m1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number('0', m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 1:	if(m1_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number('0', m1_x_offset, m1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number('1', m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 2:	if(m1_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number('1', m1_x_offset, m1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number('2', m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 3:	if(m1_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number('2', m1_x_offset, m1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number('3', m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 4:	if(m1_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number('3', m1_x_offset, m1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number('4', m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 5:	if(m1_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number('4', m1_x_offset, m1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number('5', m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 6:	if(m1_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number('5', m1_x_offset, m1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number('6', m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 7:	if(m1_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number('6', m1_x_offset, m1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number('7', m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 8:	if(m1_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number('7', m1_x_offset, m1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number('8', m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 9:	if(m1_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number('8', m1_x_offset, m1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number('9', m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	}
	/* save values for time changing effects */
	h0_old = h0;
	h1_old = h1;
	m0_old = m0;
	m1_old = m1;
	init = 0;
}

/**
  * @brief  set hour +1 on the RTC registers
  * @note   None
  * @retval None
  */
void led_clock_hour_plus(Alarmclock *alarmclock_param){
	/* reset tick counter, for clean setup blinking of the numbers */
	HAL_SetTick(501);
	/* read time and date from rtc registers and save it into time and datestructureget */
	if(alarmclock_param->timestructure.Hours - 0x01 > 0x00){
		alarmclock_param->timestructure.Hours -= 0x01;
	}else{
		alarmclock_param->timestructure.Hours = 0x17;
	}
	HAL_RTC_SetTime(&RTC_Handle, &alarmclock_param->timestructure, RTC_FORMAT_BIN);
}

/**
  * @brief  set hour -1 on the RTC registers
  * @note   None
  * @retval None
  */
void led_clock_hour_minus(Alarmclock *alarmclock_param){
	/* reset tick counter, for clean setup blinking of the numbers */
	HAL_SetTick(501);
	/* increment 1 hour and save it on the struct */
	if(alarmclock_param->timestructure.Hours + 0x01 < 0x18){
		alarmclock_param->timestructure.Hours += 0x01;
	}else{
		alarmclock_param->timestructure.Hours = 0x00;
	}
	/* set new time in the rtc registers with the structs */
	HAL_RTC_SetTime(&RTC_Handle, &alarmclock_param->timestructure, RTC_FORMAT_BIN);
}

/**
  * @brief  set minute +1 on the RTC registers
  * @note   None
  * @retval None
  */
void led_clock_minute_plus(Alarmclock *alarmclock_param){
	/* reset tick counter, for clean setup blinking of the numbers */
	HAL_SetTick(501);
	if(alarmclock_param->timestructure.Minutes - 0x01 > 0x00){
		alarmclock_param->timestructure.Minutes -= 0x01;
	}else{
		alarmclock_param->timestructure.Minutes = 0x3b;
	}
	HAL_RTC_SetTime(&RTC_Handle, &alarmclock_param->timestructure, RTC_FORMAT_BIN);
}

/**
  * @brief  set minute -1 on the RTC registers
  * @note   None
  * @retval None
  */
void led_clock_minute_minus(Alarmclock *alarmclock_param){
	/* reset tick counter, for clean setup blinking of the numbers */
	HAL_SetTick(501);
	if(alarmclock_param->timestructure.Minutes + 0x01 < 0x3c){
		alarmclock_param->timestructure.Minutes += 0x01;
	}else{
		alarmclock_param->timestructure.Minutes = 0x00;
	}
	HAL_RTC_SetTime(&RTC_Handle, &alarmclock_param->timestructure, RTC_FORMAT_BIN);
}

/**
  * @brief  set hour +1 on the RTC alarm registers
  * @note   None
  * @retval None
  */
void led_alarm_hour_plus(Alarmclock *alarmclock_param){
	/* reset tick counter, for clean setup blinking of the numbers */
	HAL_SetTick(501);
	if(alarmclock_param->alarmstructure.AlarmTime.Hours - 0x01 > 0x00){
		alarmclock_param->alarmstructure.AlarmTime.Hours -= 0x01;
	}else{
		alarmclock_param->alarmstructure.AlarmTime.Hours = 0x17;
	}
	alarmclock_param->alarmstructure.AlarmTime.Seconds = 0x00;
}

/**
  * @brief  set hour -1 on the RTC alarm registers
  * @note   None
  * @retval None
  */
void led_alarm_hour_minus(Alarmclock *alarmclock_param){
	/* reset tick counter, for clean setup blinking of the numbers */
	HAL_SetTick(501);
	if(alarmclock_param->alarmstructure.AlarmTime.Hours + 0x01 < 0x18){
		alarmclock_param->alarmstructure.AlarmTime.Hours += 0x01;
	}else{
		alarmclock_param->alarmstructure.AlarmTime.Hours = 0x00;
	}
	alarmclock_param->alarmstructure.AlarmTime.Seconds = 0x00;
}

/**
  * @brief  set minute +1 on the RTC alarm registers
  * @note   None
  * @retval None
  */
void led_alarm_minute_plus(Alarmclock *alarmclock_param){
	/* reset tick counter, for clean setup blinking of the numbers */
	HAL_SetTick(501);
	if(alarmclock_param->alarmstructure.AlarmTime.Minutes - 0x01 > 0x00){
		alarmclock_param->alarmstructure.AlarmTime.Minutes -= 0x01;
	}else{
		alarmclock_param->alarmstructure.AlarmTime.Minutes = 0x3b;
	}
	alarmclock_param->alarmstructure.AlarmTime.Seconds = 0x00;
}

/**
  * @brief  set minute -1 on the RTC alarm registers
  * @note   None
  * @retval None
  */
void led_alarm_minute_minus(Alarmclock *alarmclock_param){
	/* reset tick counter, for clean setup blinking of the numbers */
	HAL_SetTick(501);
	if(alarmclock_param->alarmstructure.AlarmTime.Minutes + 0x01 < 0x3c){
		alarmclock_param->alarmstructure.AlarmTime.Minutes += 0x01;
	}else{
		alarmclock_param->alarmstructure.AlarmTime.Minutes = 0x00;
	}
	alarmclock_param->alarmstructure.AlarmTime.Seconds = 0x00;
}

/**
  * @brief  sets next alarm style
  * @note   None
  * @retval None
  */
void alarm_style_plus(Alarmclock *alarmclock_param){
	if(alarmclock_param->alarm_style > 0 ){
		alarmclock_param->alarm_style--;
	}else{
		alarmclock_param->alarm_style = 1;
	}
}

/**
  * @brief  sets alarm style before
  * @note   None
  * @retval None
  */
void alarm_style_minus(Alarmclock *alarmclock_param){
	if(alarmclock_param->alarm_style < 1 ){
		alarmclock_param->alarm_style++;
	}else{
		alarmclock_param->alarm_style = 0;
	}
}

/**
  * @brief  sets alarm style before
  * @note   None
  * @retval None
  */
void show_alarm_style(Alarmclock *alarmclock_param){
	switch(alarmclock_param->alarm_style){
		case FLASHING:	WS2812_display_flash(50, 3);
		break;
		case COLORFALL:	WS2812_display_colorfall();
		break;
	}
}

/**
  * @brief  increase snooze
  * @note   None
  * @retval None
  */
void snooze_plus(Alarmclock *alarmclock_param){
	if(alarmclock_param->snooze_duration > 5 ){
		alarmclock_param->snooze_duration -= 5;
	}else{
		alarmclock_param->snooze_duration = 30;
	}
}

/**
  * @brief  decrease snooze
  * @note   None
  * @retval None
  */
void snooze_minus(Alarmclock *alarmclock_param){
	if(alarmclock_param->snooze_duration < 30 ){
		alarmclock_param->snooze_duration += 5;
	}else{
		alarmclock_param->snooze_duration = 5;
	}
}

/**
  * @brief  show recent snooze
  * @note   None
  * @retval None
  */
void draw_snooze(Alarmclock *alarmclock_param){
	switch(alarmclock_param->snooze_duration){
		case 0:		draw_string("no snooze", 0, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 5:		draw_string(" 5 m", 0, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 10:	draw_string("10 m", 0, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 15:	draw_string("15 m", 0, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 20:	draw_string("20 m", 0, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 25:	draw_string("25 m", 0, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 30:	draw_string("30 m", 0, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
	}
}

/**
  * @brief  Configure the current time, alarm time and date.
  * @param  None
  * @retval None
  */
void RTC_CalendarConfig(Alarmclock *alarmclock_param){
	/* Configure the Date */
	/* Set Date: Tuesday February 18th 2014 */
	alarmclock_param->datestructure.Year 		= 0x14;
	alarmclock_param->datestructure.Month 		= RTC_MONTH_FEBRUARY;
	alarmclock_param->datestructure.Date 		= 0x18;
	alarmclock_param->datestructure.WeekDay 	= RTC_WEEKDAY_TUESDAY;

	if(HAL_RTC_SetDate(&RTC_Handle,&alarmclock_param->datestructure,RTC_FORMAT_BCD) != HAL_OK){
		/* Initialization Error */
		//Error_Handler();
	}

	/* Configure the Time */
	/* Set Time: 02:00:00 */
	alarmclock_param->timestructure.Hours 		= 0x02;
	alarmclock_param->timestructure.Minutes 	= 0x00;
	alarmclock_param->timestructure.Seconds 	= 0x00;

	if (HAL_RTC_SetTime(&RTC_Handle,&alarmclock_param->timestructure,RTC_FORMAT_BCD) != HAL_OK){
		/* Initialization Error */
		//Error_Handler();
	}

	/* Writes a data in a RTC Backup data Register1 */
	HAL_RTCEx_BKUPWrite(&RTC_Handle, RTC_BKP_DR1, 0x32F2);
}

/**
  * @brief  RTC alarm callback
  * @param  None
  * @retval None
  */
void RTC_AlarmEventCallback(Alarmclock *alarmclock_param){
		buzzer_start(alarmclock_param);
}

/**
  * @brief  this function increments the recent mode
  * @param  None
  * @retval None
  */
void increment_mode(Alarmclock *alarmclock_param){
	/* reset tick counter, for clean setup blinking of the numbers */
	HAL_SetTick(1);
	if(alarmclock_param->mode < mode_count){
		alarmclock_param->mode++;
	}else{
		alarmclock_param->mode = 0;
	}
	/* only in the clock mode, the alarm shall work */
	if(alarmclock_param->mode != MODE_TIME_CLOCK){
		/* disable the alarm interrupt */
		set_alarm_irq(DISABLE, alarmclock_param);
	}else{
		set_alarm_irq(ENABLE, alarmclock_param);
	}
	/* draw mode change on the clock display */
	set_button_irq(DISABLE);
	draw_mode(alarmclock_param);
	set_button_irq(ENABLE);
}

/**
  * @brief  this function is used to refresh time and background of the wordclock
  * @param  None
  * @retval None
  */
void refresh_clock_display(Alarmclock *alarmclock_param){
	/* erase frame buffer */
	WS2812_clear_buffer();
	/* write time into frame buffer */
	draw_time(alarmclock_param);
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
void setup_clock_blinking(Alarmclock *alarmclock_param){
	/* erase frame buffer */
	WS2812_clear_buffer();
	/* get recent hal tick value for blinking the numbers */
	hal_tick_temp = HAL_GetTick();
	/* write setup time into frame buffer */
	if(alarmclock_param->mode == MODE_TIME_SET_CLOCK_h || alarmclock_param->mode == MODE_TIME_SET_ALARM_h){
		draw_hh_mm(MINUTES, alarmclock_param);
		if(hal_tick_temp % SETUP_CLOCK_BLINKING_PERIOD > 500 && hal_tick_temp % SETUP_CLOCK_BLINKING_PERIOD < 1000){
			draw_hh_mm(HOURS, alarmclock_param);
		}
	}else if(alarmclock_param->mode == MODE_TIME_SET_CLOCK_min || alarmclock_param->mode == MODE_TIME_SET_ALARM_min){
		draw_hh_mm(HOURS, alarmclock_param);
		if(hal_tick_temp % SETUP_CLOCK_BLINKING_PERIOD > 500 && hal_tick_temp % SETUP_CLOCK_BLINKING_PERIOD < 1000){
			draw_hh_mm(MINUTES, alarmclock_param);
		}
	}
	/* wait for the data transmission to the led's to be ready */
	while(!WS2812_TC);
	/* send frame buffer to the leds */
	sendbuf_WS2812();
}

/**
  * @brief  this function checks alarm switch position
  * @param  None
  * @retval None
  */
uint8_t read_alarm_switch(void){
	if(GPIOB->IDR &= (uint32_t)SWITCH_ALARM){
		return 0;
	}else{
		return 1;
	}
}

/**
  * @brief  this function is used to get color from the backup register BKP_DR
  * @param  None
  * @retval None
  */
void get_clock_preferences(Alarmclock *alarmclock_param){
	  uint32_t backupregister = 0U;
	  uint32_t backupregister_value = 0U;
	  uint32_t backup_register_mask = 0x000000FF;

	  /* get reset variable */
	  backupregister = (uint32_t)BKP_BASE;
	  backupregister += (1 * 4U);
	  backupregister_value = (*(__IO uint32_t *)(backupregister)) & BKP_DR1_D;

	  /* if variable = 0x32F2, BKP registers have saved preferences, otherwise use default values */
	  if(backupregister_value != 0x32F2){
			/* init default color */
			alarmclock_param->color_index = 0;
		  	alarmclock_param->red =  color_pattern[alarmclock_param->color_index][0];
		  	alarmclock_param->green =  color_pattern[alarmclock_param->color_index][1];
		  	alarmclock_param->blue =  color_pattern[alarmclock_param->color_index][2];
		  	/* init default alarm time */
		  	alarmclock_param->alarmstructure.AlarmTime.Hours 	= 0x06;
		  	alarmclock_param->alarmstructure.AlarmTime.Minutes 	= 0x00;
		  	alarmclock_param->alarmstructure.AlarmTime.Seconds 	= 0x00;
		  	/* init default snooze duration */
		  	alarmclock_param->snooze_duration = 5;
		  	/* init default alarm style */
		  	alarmclock_param->alarm_style = FLASHING;
		    /* safe initial preferences */
		    set_clock_preferences(alarmclock_param);
	  }else{
		  /* get red color */
		  backupregister = (uint32_t)BKP_BASE;
		  backupregister += (2 * 4U);

		  alarmclock_param->red = (*(__IO uint32_t *)(backupregister)) & (uint32_t)backup_register_mask;
		  backupregister = 0U;

		  /* get green color */
		  backupregister = (uint32_t)BKP_BASE;
		  backupregister += (3 * 4U);

		  alarmclock_param->green = (*(__IO uint32_t *)(backupregister)) & (uint32_t)backup_register_mask;
		  backupregister = 0U;

		  /* get blue color */
		  backupregister = (uint32_t)BKP_BASE;
		  backupregister += (4 * 4U);

		  alarmclock_param->blue = (*(__IO uint32_t *)(backupregister)) & (uint32_t)backup_register_mask;
		  backupregister = 0U;
		  
		  /* get color index */
		  backupregister = (uint32_t)BKP_BASE;
		  backupregister += (5 * 4U);

		  alarmclock_param->color_index = (*(__IO uint32_t *)(backupregister)) & (uint32_t)backup_register_mask;
		  backupregister = 0U;

		  /* get alarm time: hours */
		  backupregister = (uint32_t)BKP_BASE;
		  backupregister += (6 * 4U);

		  alarmclock_param->alarmstructure.AlarmTime.Hours = (*(__IO uint32_t *)(backupregister)) & (uint32_t)backup_register_mask;
		  backupregister = 0U;

		  /* get alarm time: minutes */
		  backupregister = (uint32_t)BKP_BASE;
		  backupregister += (7 * 4U);

		  alarmclock_param->alarmstructure.AlarmTime.Minutes = (*(__IO uint32_t *)(backupregister)) & (uint32_t)backup_register_mask;
		  backupregister = 0U;

		  /* get alarm time: seconds */
		  backupregister = (uint32_t)BKP_BASE;
		  backupregister += (8 * 4U);

		  alarmclock_param->alarmstructure.AlarmTime.Seconds = (*(__IO uint32_t *)(backupregister)) & (uint32_t)backup_register_mask;
		  backupregister = 0U;

		  /* Enable and set RTC interrupt for alarm function */
		  alarmclock_param->alarmstructure.AlarmTime.Seconds = 0x00;

		  /* get alarm style */
		  backupregister = (uint32_t)BKP_BASE;
		  backupregister += (9 * 4U);

		  alarmclock_param->alarm_style = (*(__IO uint32_t *)(backupregister)) & (uint32_t)backup_register_mask;
		  backupregister = 0U;

		  /* get snooze duration */
		  backupregister = (uint32_t)BKP_BASE;
		  backupregister += (10 * 4U);

		  alarmclock_param->snooze_duration = (*(__IO uint32_t *)(backupregister)) & (uint32_t)backup_register_mask;
		  backupregister = 0U;
	  }
}

/**
  * @brief  this function is used to set color in the backup register BKP_DR
  * @param  None
  * @retval None
  */
void set_clock_preferences(Alarmclock *alarmclock_param){
	  uint32_t tmp = 0U;
	  uint32_t backup_register_mask = 0x000000FF;

	  /* set red color */
	  tmp = (uint32_t)BKP_BASE;
	  tmp += (2 * 4U);

	  *(__IO uint32_t *) tmp = alarmclock_param->red & backup_register_mask;
	  tmp = 0U;

	  /* set green color */
	  tmp = (uint32_t)BKP_BASE;
	  tmp += (3 * 4U);

	  *(__IO uint32_t *) tmp =  alarmclock_param->green & backup_register_mask;
	  tmp = 0U;

	  /* set blue color */
	  tmp = (uint32_t)BKP_BASE;
	  tmp += (4 * 4U);

	  *(__IO uint32_t *) tmp = alarmclock_param->blue & backup_register_mask;
	  tmp = 0U;
	  
	  /* set color index */
	  tmp = (uint32_t)BKP_BASE;
	  tmp += (5 * 4U);

	  *(__IO uint32_t *) tmp = alarmclock_param->color_index & backup_register_mask;
	  tmp = 0U;

	  /* set alarm: hours */
	  tmp = (uint32_t)BKP_BASE;
	  tmp += (6 * 4U);

	  *(__IO uint32_t *) tmp = alarmclock_param->alarmstructure.AlarmTime.Hours & backup_register_mask;
	  tmp = 0U;

	  /* set alarm: minutes */
	  tmp = (uint32_t)BKP_BASE;
	  tmp += (7 * 4U);

	  *(__IO uint32_t *) tmp = alarmclock_param->alarmstructure.AlarmTime.Minutes & backup_register_mask;
	  tmp = 0U;

	  /* set alarm: seconds */
	  tmp = (uint32_t)BKP_BASE;
	  tmp += (8 * 4U);

	  *(__IO uint32_t *) tmp = alarmclock_param->alarmstructure.AlarmTime.Seconds & backup_register_mask;
	  tmp = 0U;

	  /* set alarm style */
	  tmp = (uint32_t)BKP_BASE;
	  tmp += (9 * 4U);

	  *(__IO uint32_t *) tmp = alarmclock_param->alarm_style & backup_register_mask;
	  tmp = 0U;

	  /* set snooze duration */
	  tmp = (uint32_t)BKP_BASE;
	  tmp += (10 * 4U);

	  *(__IO uint32_t *) tmp = alarmclock_param->snooze_duration & backup_register_mask;
	  tmp = 0U;
}

/**
  * @brief  this function increments the color
  * @param  None
  * @retval None
  */
void increment_clock_color(Alarmclock *alarmclock_param){
	/* variables for the next color */
	uint8_t new_red, new_green, new_blue;

	/* get the new color with the incremented color index */
	if(alarmclock_param->color_index < 3){
		alarmclock_param->color_index++;
	}else{
		alarmclock_param->color_index = 0;
	}

	/* the new colors */
	new_red = color_pattern[alarmclock_param->color_index][0];
	new_green = color_pattern[alarmclock_param->color_index][1];
	new_blue = color_pattern[alarmclock_param->color_index][2];

	/* color loop effect */
	while(alarmclock_param->red != new_red || alarmclock_param->green != new_green || alarmclock_param->blue != new_blue){
		/* red */
		if(alarmclock_param->red < new_red){
			alarmclock_param->red++;
		}else if(alarmclock_param->red > new_red){
			alarmclock_param->red--;
		}
		/* green */
		if(alarmclock_param->green < new_green){
			alarmclock_param->green++;
		}else if(alarmclock_param->green > new_green){
			alarmclock_param->green--;
		}
		/* blue */
		if(alarmclock_param->blue < new_blue){
			alarmclock_param->blue++;
		}else if(alarmclock_param->blue > new_blue){
			alarmclock_param->blue--;
		}
		/* refresh clock display */
		refresh_clock_display(alarmclock_param);
		HAL_Delay(50);
	}
}

/**
  * @brief  this function increments the color
  * @param  None
  * @retval None
  */
void decrement_clock_color(Alarmclock *alarmclock_param){
	/* variables for the next color */
	uint8_t new_red, new_green, new_blue;

	if(alarmclock_param->color_index > 0){
		alarmclock_param->color_index--;
	}else{
		alarmclock_param->color_index = 3;
	}

	/* the new colors */
	new_red = color_pattern[alarmclock_param->color_index][0];
	new_green = color_pattern[alarmclock_param->color_index][1];
	new_blue = color_pattern[alarmclock_param->color_index][2];

	/* color loop effect */
	while(alarmclock_param->red != new_red || alarmclock_param->green != new_green || alarmclock_param->blue != new_blue){
		/* red */
		if(alarmclock_param->red < new_red){
			alarmclock_param->red++;
		}else if(alarmclock_param->red > new_red){
			alarmclock_param->red--;
		}
		/* green */
		if(alarmclock_param->green < new_green){
			alarmclock_param->green++;
		}else if(alarmclock_param->green > new_green){
			alarmclock_param->green--;
		}
		/* blue */
		if(alarmclock_param->blue < new_blue){
			alarmclock_param->blue++;
		}else if(alarmclock_param->blue > new_blue){
			alarmclock_param->blue--;
		}
		/* refresh clock display */
		refresh_clock_display(alarmclock_param);
		HAL_Delay(50);
	}
}

/**
  * @brief  this function inits the alarmclock
  * @param  None
  * @retval None
  */
void init_clock(Alarmclock *alarmclock_param){
	/* load preferences from the BKP register, time is loadet with init_RTc() on the bottom of this function */
	get_clock_preferences(alarmclock_param);
	/* set system mode */
	alarmclock_param->mode = MODE_TIME_CLOCK;
	/* initialize RTC (also load time information from BKP register) */
	init_RTC(alarmclock_param);
	/* set alarm interrupt */
	HAL_RTC_SetAlarm_IT(&RTC_Handle, &alarmclock_param->alarmstructure, RTC_FORMAT_BIN);
	HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
}

/**
  * @brief  enables/disables all alarm it
  * @param  SET/RESET
  * @retval None
  */
void set_alarm_irq(FunctionalState alarm_irq, Alarmclock *alarmclock_param){
	if(alarm_irq != DISABLE){
		/* enable the alarm if it is still disabled */
		if(!__HAL_RTC_ALARM_GET_IT_SOURCE(&RTC_Handle, RTC_IT_ALRA)){
			/* rtc interrupt */
			HAL_RTC_SetAlarm_IT(&RTC_Handle, &alarmclock_param->alarmstructure, RTC_FORMAT_BIN);
			HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 1, 0);
			HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
		}
	}else{
		/* disable the alarm interrupt if alarm is still enabled*/
		if(__HAL_RTC_ALARM_GET_IT_SOURCE(&RTC_Handle, RTC_IT_ALRA)){
			/* rtc interrupt */
			HAL_RTC_DeactivateAlarm(&RTC_Handle, alarmclock_param->alarmstructure.Alarm);
		}
	}
}

/**
  * @brief RTC MSP Initialization
  *        This function configures the hardware resources used in this example
  * @param hrtc: RTC handle pointer
  *
  * @note  Care must be taken when HAL_RCCEx_PeriphCLKConfig() is used to select
  *        the RTC clock source; in this case the Backup domain will be reset in
  *        order to modify the RTC Clock source, as consequence RTC registers (including
  *        the backup registers) and RCC_BDCR register are set to their reset values.
  *
  * @retval None
  */
void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc){
	RCC_OscInitTypeDef        RCC_OscInitStruct;
	RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

	/*##-1- Enables the PWR Clock and Enables access to the backup domain ###################################*/
	/* To change the source clock of the RTC feature (LSE, LSI), You have to:
     	 - Enable the power clock using __HAL_RCC_PWR_CLK_ENABLE()
     	 - Enable write access using HAL_PWR_EnableBkUpAccess() function before to
       	   configure the RTC clock source (to be done once after reset).
     	 - Reset the Back up Domain using __HAL_RCC_BACKUPRESET_FORCE() and
       	   __HAL_RCC_BACKUPRESET_RELEASE().
     	 - Configure the needed RTc clock source */
	__HAL_RCC_PWR_CLK_ENABLE();
	HAL_PWR_EnableBkUpAccess();

	/* Enable BKP CLK for backup registers */
	__HAL_RCC_BKP_CLK_ENABLE();

	/*##-2- Configue LSE as RTC clock soucre ###################################*/
	RCC_OscInitStruct.OscillatorType =	RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.PLL.PLLState = 	RCC_PLL_NONE;
	RCC_OscInitStruct.LSEState = 		RCC_LSE_ON;
	if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){
		//Error_Handler();
	}

	PeriphClkInitStruct.PeriphClockSelection = 	RCC_PERIPHCLK_RTC;
	PeriphClkInitStruct.RTCClockSelection = 	RCC_RTCCLKSOURCE_LSE;
	if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK){
		//Error_Handler();
	}

	/*##-3- Enable RTC peripheral Clocks #######################################*/
	/* Enable RTC Clock */
	__HAL_RCC_RTC_ENABLE();
}

/**
  * @brief  draws hour recent measured adc value from the ambient light sensor
  * @note   None
  * @retval None
  */
void draw_lux(Alarmclock *alarmclock_param){
		uint32_t adc0,adc1,adc2,adc3,x_offset,y_offset;
		uint16_t adc_avr;

		adc_avr = get_avr_lux();

		/* process hours */
		adc0 = adc_avr % 10;
		adc1 = adc_avr / 10;
		adc1 = adc1 % 10;
		adc2 = adc_avr /100;
		adc2 = adc2 % 10;
		adc3 = adc_avr /1000;
		adc3 = adc3 % 10;

		/* erase frame buffer */
		WS2812_clear_buffer();

		x_offset = 14;
		y_offset = 0;
		switch(adc0){
			case 0:	draw_number('0', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 1:	draw_number('1', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 2:	draw_number('2', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 3:	draw_number('3', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 4:	draw_number('4', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 5:	draw_number('5', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 6:	draw_number('6', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 7:	draw_number('7', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 8:	draw_number('8', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 9:	draw_number('9', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
		}
		x_offset = 10;
		y_offset = 0;
		switch(adc1){
			case 0:	draw_number('0', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 1:	draw_number('1', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 2:	draw_number('2', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 3:	draw_number('3', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 4:	draw_number('4', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 5:	draw_number('5', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 6:	draw_number('6', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 7:	draw_number('7', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 8:	draw_number('8', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 9:	draw_number('9', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
		}
		x_offset = 6;
		y_offset = 0;
		switch(adc2){
			case 0:	draw_number('0', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 1:	draw_number('1', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 2:	draw_number('2', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 3:	draw_number('3', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 4:	draw_number('4', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 5:	draw_number('5', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 6:	draw_number('6', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 7:	draw_number('7', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 8:	draw_number('8', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 9:	draw_number('9', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
		}
		x_offset = 2;
		y_offset = 0;
		switch(adc3){
			case 0:	draw_number('0', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 1:	draw_number('1', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 2:	draw_number('2', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 3:	draw_number('3', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 4:	draw_number('4', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 5:	draw_number('5', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 6:	draw_number('6', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 7:	draw_number('7', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 8:	draw_number('8', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 9:	draw_number('9', x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
		}
		/* wait for the data transmission to the led's to be ready */
		while(!WS2812_TC);
		/* send frame buffer to the leds */
		sendbuf_WS2812();
}
