/*
 * Autor: Nico Korn
 * Date: 26.10.2017
 * Firmware for the STM32F103 Microcontroller to work with WS2812b leds.
 *  *
 * Copyright (c) 2017 Nico Korn
 *
 * clock_ws2812.c this module contents the time function for the wordclock
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
#include "ws2812.h"
#include "buzzer.h"
#include "stm32f1xx.h"

/* variables */
static RTC_TimeTypeDef 		timestructureset;
static RTC_DateTypeDef 		datestructureset;
static RTC_TimeTypeDef 		timestructureget;
static RTC_DateTypeDef 		datestructureget;
static RTC_AlarmTypeDef 	alarmstructure;
static Alarm_Mode			alarm_mode;
static uint32_t				alarm_id;
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
static Number 				double_point;
static uint8_t	 			h0, h1, m0, m1;
static uint16_t 			x_offset;
static uint8_t 				y_offset;

/* global variables */
RTC_HandleTypeDef 	RTC_Handle;

/**
  * @brief  initialization of words & rtc
  * @note   None
  * @retval None
  */
void init_clock(){
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
	nine.number_construction[0][0] = 0;
	nine.number_construction[1][0] = 0;
	nine.number_construction[2][0] = 0;
	nine.number_construction[0][1] = 0;
	nine.number_construction[1][1] = 0;
	nine.number_construction[2][1] = 0;
	nine.number_construction[0][2] = 0;
	nine.number_construction[1][2] = 1;
	nine.number_construction[2][2] = 0;
	nine.number_construction[0][3] = 0;
	nine.number_construction[1][3] = 0;
	nine.number_construction[2][3] = 0;
	nine.number_construction[0][4] = 0;
	nine.number_construction[1][4] = 1;
	nine.number_construction[2][4] = 0;
	nine.number_construction[0][5] = 0;
	nine.number_construction[1][5] = 0;
	nine.number_construction[2][5] = 0;
	nine.number_construction[0][6] = 0;
	nine.number_construction[1][6] = 0;
	nine.number_construction[2][6] = 0;

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
	    /* Configure RTC Calendar */
	    RTC_CalendarConfig();
	}else{
		/* Clear source Reset Flag */
	    __HAL_RCC_CLEAR_RESET_FLAGS();
	}
}

/**
  * @brief  draws a a number into the IO buffer
  * @note   None
  * @retval None
  */
void draw_number(Number number, uint16_t x_offset, uint8_t y_offset, uint8_t *red, uint8_t *green, uint8_t *blue){
	for(uint16_t x = 0; x < 3; x++){
		for(uint8_t y = 0; y < 7; y++){
			if(number.number_construction[x][y] != 0){
				WS2812_framedata_setPixel((uint8_t)y_offset + (uint8_t)y, (uint16_t)x_offset + (uint16_t)x, (uint8_t)*red, (uint8_t)*green, (uint8_t)*blue);
			}
		}
	}
}

/**
  * @brief  draws hour or minutes according last read time from rtc/alarm with incremented/decremented hours or minutes from the setup mode
  * @note   None
  * @retval None
  */
void draw_hh_mm(Time_Setup time, Wordclock_Mode mode, uint8_t *red, uint8_t *green, uint8_t *blue){
	/* depending on mode, last refreshed time data is load into h0h1:m0m1 or the alarm data into h0h1:m0m1 */
	if(mode == MODE_TIME_SET_CLOCK_h || mode == MODE_TIME_SET_CLOCK_min){
		/* process hours */
		h0 = timestructureget.Hours / 10;
		h1 = timestructureget.Hours % 10;
		/* process minutes */
		m0 = timestructureget.Minutes / 10;
		m1 = timestructureget.Minutes % 10;
	}else if(mode == MODE_TIME_SET_ALARM_h || mode == MODE_TIME_SET_ALARM_min){
		/* process hours */
		h0 = alarmstructure.AlarmTime.Hours / 10;
		h1 = alarmstructure.AlarmTime.Hours % 10;
		/* process minutes */
		m0 = alarmstructure.AlarmTime.Hours / 10;
		m1 = alarmstructure.AlarmTime.Hours % 10;
	}

	/* draw double point */
	draw_number(double_point, (uint16_t)7, (uint8_t)0, red, green, blue);
	if(time == HOURS){
		/* draw numbers into display buffer */
		/* number position h0 */
		x_offset = 0;
		y_offset = 0;
		switch(h0){
			case 0:	draw_number(zero, x_offset, y_offset, red, green, blue);
			break;
			case 1:	draw_number(one, x_offset, y_offset, red, green, blue);
			break;
			case 2:	draw_number(two, x_offset, y_offset, red, green, blue);
			break;
		}
		/* number position h1 */
		x_offset = 4;
		y_offset = 0;
		switch(h1){
			case 0:	draw_number(zero, x_offset, y_offset, red, green, blue);
			break;
			case 1:	draw_number(one, x_offset, y_offset, red, green, blue);
			break;
			case 2:	draw_number(two, x_offset, y_offset, red, green, blue);
			break;
			case 3:	draw_number(three, x_offset, y_offset, red, green, blue);
			break;
			case 4:	draw_number(four, x_offset, y_offset, red, green, blue);
			break;
			case 5:	draw_number(five, x_offset, y_offset, red, green, blue);
			break;
			case 6:	draw_number(six, x_offset, y_offset, red, green, blue);
			break;
			case 7:	draw_number(seven, x_offset, y_offset, red, green, blue);
			break;
			case 8:	draw_number(eight, x_offset, y_offset, red, green, blue);
			break;
			case 9:	draw_number(nine, x_offset, y_offset, red, green, blue);
			break;
		}
	}else if(time == MINUTES){
		/* number position m0 */
		x_offset = 10;
		y_offset = 0;
		switch(m0){
			case 0:	draw_number(zero, x_offset, y_offset, red, green, blue);
			break;
			case 1:	draw_number(one, x_offset, y_offset, red, green, blue);
			break;
			case 2:	draw_number(two, x_offset, y_offset, red, green, blue);
			break;
			case 3:	draw_number(three, x_offset, y_offset, red, green, blue);
			break;
			case 4:	draw_number(four, x_offset, y_offset, red, green, blue);
			break;
			case 5:	draw_number(five, x_offset, y_offset, red, green, blue);
			break;
		}
		/* number position m1 */
		x_offset = 14;
		y_offset = 0;
		switch(m1){
			case 0:	draw_number(zero, x_offset, y_offset, red, green, blue);
			break;
			case 1:	draw_number(one, x_offset, y_offset, red, green, blue);
			break;
			case 2:	draw_number(two, x_offset, y_offset, red, green, blue);
			break;
			case 3:	draw_number(three, x_offset, y_offset, red, green, blue);
			break;
			case 4:	draw_number(four, x_offset, y_offset, red, green, blue);
			break;
			case 5:	draw_number(five, x_offset, y_offset, red, green, blue);
			break;
			case 6:	draw_number(six, x_offset, y_offset, red, green, blue);
			break;
			case 7:	draw_number(seven, x_offset, y_offset, red, green, blue);
			break;
			case 8:	draw_number(eight, x_offset, y_offset, red, green, blue);
			break;
			case 9:	draw_number(nine, x_offset, y_offset, red, green, blue);
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
void draw_time(uint8_t *red, uint8_t *green, uint8_t *blue){
	HAL_RTC_GetTime(&RTC_Handle, &timestructureget, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&RTC_Handle, &datestructureget, RTC_FORMAT_BIN);

	/* hours and minutes local places: hh:mm = h0h1:m0m2 */
	/* process hours */
	h0 = timestructureget.Hours / 10;
	h1 = timestructureget.Hours % 10;

	/* process minutes */
	m0 = timestructureget.Minutes / 10;
	m1 = timestructureget.Minutes % 10;

	/* draw numbers into display buffer */
	/* number position h0 */
	x_offset = 0;
	y_offset = 0;
	switch(h0){
		case 0:	draw_number(zero, x_offset, y_offset, red, green, blue);
		break;
		case 1:	draw_number(one, x_offset, y_offset, red, green, blue);
		break;
		case 2:	draw_number(two, x_offset, y_offset, red, green, blue);
		break;
	}
	/* number position h1 */
	x_offset = 4;
	y_offset = 0;
	switch(h1){
		case 0:	draw_number(zero, x_offset, y_offset, red, green, blue);
		break;
		case 1:	draw_number(one, x_offset, y_offset, red, green, blue);
		break;
		case 2:	draw_number(two, x_offset, y_offset, red, green, blue);
		break;
		case 3:	draw_number(three, x_offset, y_offset, red, green, blue);
		break;
		case 4:	draw_number(four, x_offset, y_offset, red, green, blue);
		break;
		case 5:	draw_number(five, x_offset, y_offset, red, green, blue);
		break;
		case 6:	draw_number(six, x_offset, y_offset, red, green, blue);
		break;
		case 7:	draw_number(seven, x_offset, y_offset, red, green, blue);
		break;
		case 8:	draw_number(eight, x_offset, y_offset, red, green, blue);
		break;
		case 9:	draw_number(nine, x_offset, y_offset, red, green, blue);
		break;
	}
	/* second double point */
	x_offset = 7;
	y_offset = 0;
	if(timestructureget.Seconds%2){
		draw_number(double_point, x_offset, y_offset, red, green, blue);
	}
	/* number position m0 */
	x_offset = 10;
	y_offset = 0;
	switch(m0){
	case 0:	draw_number(zero, x_offset, y_offset, red, green, blue);
	break;
	case 1:	draw_number(one, x_offset, y_offset, red, green, blue);
	break;
	case 2:	draw_number(two, x_offset, y_offset, red, green, blue);
	break;
	case 3:	draw_number(three, x_offset, y_offset, red, green, blue);
	break;
	case 4:	draw_number(four, x_offset, y_offset, red, green, blue);
	break;
	case 5:	draw_number(five, x_offset, y_offset, red, green, blue);
	break;
	}
	/* number position m1 */
	x_offset = 14;
	y_offset = 0;
	switch(m1){
	case 0:	draw_number(zero, x_offset, y_offset, red, green, blue);
	break;
	case 1:	draw_number(one, x_offset, y_offset, red, green, blue);
	break;
	case 2:	draw_number(two, x_offset, y_offset, red, green, blue);
	break;
	case 3:	draw_number(three, x_offset, y_offset, red, green, blue);
	break;
	case 4:	draw_number(four, x_offset, y_offset, red, green, blue);
	break;
	case 5:	draw_number(five, x_offset, y_offset, red, green, blue);
	break;
	case 6:	draw_number(six, x_offset, y_offset, red, green, blue);
	break;
	case 7:	draw_number(seven, x_offset, y_offset, red, green, blue);
	break;
	case 8:	draw_number(eight, x_offset, y_offset, red, green, blue);
	break;
	case 9:	draw_number(nine, x_offset, y_offset, red, green, blue);
	break;
	}
}

/**
  * @brief  set hour +1 on the RTC registers
  * @note   None
  * @retval None
  */
void led_clock_hour_plus(){
	/* read time and date from rtc registers and save it into time and datestructureget*/
	//HAL_RTC_GetTime(&RTC_Handle, &timestructureget, RTC_FORMAT_BIN);
	//HAL_RTC_GetDate(&RTC_Handle, &datestructureget, RTC_FORMAT_BIN);

	/* increment 1 hour and save it on the struct */
	if(timestructureget.Hours + 0x01 < 0x18){
		timestructureset.Hours += 0x01;
	}else{
		timestructureset.Hours = 0x00;
	}

	/* don't change minutes, seconds and date */
	/*
	timestructureset.Minutes = 	timestructureget.Minutes;
	timestructureset.Seconds = 	timestructureget.Seconds;

	datestructureset.Year = 	datestructureget.Year;
	datestructureset.Month = 	datestructureget.Month;
	datestructureset.Date = 	datestructureget.Date;
	datestructureset.WeekDay = 	datestructureget.WeekDay;
	*/

	/* set new time in the rtc registers with the structs */
	HAL_RTC_SetTime(&RTC_Handle, &timestructureset, RTC_FORMAT_BIN);
	//HAL_RTC_SetDate(&RTC_Handle, &datestructureset, RTC_FORMAT_BIN);
}

/**
  * @brief  set hour -1 on the RTC registers
  * @note   None
  * @retval None
  */
void led_clock_hour_minus(){
	/* read time and date from rtc registers and save it into time and datestructureget*/
	//HAL_RTC_GetTime(&RTC_Handle, &timestructureget, RTC_FORMAT_BIN);
	//HAL_RTC_GetDate(&RTC_Handle, &datestructureget, RTC_FORMAT_BIN);

	if(timestructureget.Hours - 0x01 > 0x00){
		timestructureset.Hours -= 0x01;
	}else{
		timestructureset.Hours = 0x17;
	}

	/*
	timestructureset.Minutes = 	timestructureget.Minutes;
	timestructureset.Seconds = 	timestructureget.Seconds;

	datestructureset.Year = 	datestructureget.Year;
	datestructureset.Month = 	datestructureget.Month;
	datestructureset.Date = 	datestructureget.Date;
	datestructureset.WeekDay = 	datestructureget.WeekDay;
	*/

	HAL_RTC_SetTime(&RTC_Handle, &timestructureset, RTC_FORMAT_BIN);
	//HAL_RTC_SetDate(&RTC_Handle, &datestructureset, RTC_FORMAT_BIN);
}

/**
  * @brief  set minute +1 on the RTC registers
  * @note   None
  * @retval None
  */
void led_clock_minute_plus(){
	/* read time and date from rtc registers and save it into time and datestructureget*/
	//HAL_RTC_GetTime(&RTC_Handle, &timestructureget, RTC_FORMAT_BIN);
	//HAL_RTC_GetDate(&RTC_Handle, &datestructureget, RTC_FORMAT_BIN);

	//timestructureset.Hours = timestructureget.Hours;
	if(timestructureget.Minutes + 0x01 < 0x3c){
		timestructureset.Minutes += 0x01;
	}else{
		timestructureset.Minutes = 0x00;
	}

	/*
	timestructureset.Seconds = timestructureget.Seconds;

	datestructureset.Year = 	datestructureget.Year;
	datestructureset.Month = 	datestructureget.Month;
	datestructureset.Date = 	datestructureget.Date;
	datestructureset.WeekDay = 	datestructureget.WeekDay;
	*/

	HAL_RTC_SetTime(&RTC_Handle, &timestructureset, RTC_FORMAT_BIN);
	//HAL_RTC_SetDate(&RTC_Handle, &datestructureset, RTC_FORMAT_BIN);
}

/**
  * @brief  set minute -1 on the RTC registers
  * @note   None
  * @retval None
  */
void led_clock_minute_minus(){
	/* read time and date from rtc registers and save it into time and datestructureget*/
	//HAL_RTC_GetTime(&RTC_Handle, &timestructureget, RTC_FORMAT_BIN);
	//HAL_RTC_GetDate(&RTC_Handle, &datestructureget, RTC_FORMAT_BIN);

	//timestructureset.Hours = timestructureget.Hours;
	if(timestructureget.Minutes - 0x01 > 0x00){
		timestructureset.Minutes -= 0x01;
	}else{
		timestructureset.Minutes = 0x3b;
	}
	/*
	timestructureset.Seconds = timestructureget.Seconds;

	datestructureset.Year = 	datestructureget.Year;
	datestructureset.Month = 	datestructureget.Month;
	datestructureset.Date = 	datestructureget.Date;
	datestructureset.WeekDay = 	datestructureget.WeekDay;
	*/

	HAL_RTC_SetTime(&RTC_Handle, &timestructureset, RTC_FORMAT_BIN);
	//HAL_RTC_SetDate(&RTC_Handle, &datestructureset, RTC_FORMAT_BIN);
}

/**
  * @brief  set hour +1 on the RTC alarm registers
  * @note   None
  * @retval None
  */
void led_alarm_hour_plus(){
	HAL_RTC_GetAlarm(&RTC_Handle, &alarmstructure, alarm_id, RTC_FORMAT_BCD);
	if(alarmstructure.AlarmTime.Hours + 0x01 < 0x18){
		alarmstructure.AlarmTime.Hours += 0x01;
	}else{
		alarmstructure.AlarmTime.Hours = 0x00;
	}
	HAL_RTC_SetAlarm(&RTC_Handle, &alarmstructure, RTC_FORMAT_BCD);
}

/**
  * @brief  set hour -1 on the RTC alarm registers
  * @note   None
  * @retval None
  */
void led_alarm_hour_minus(){
	HAL_RTC_GetAlarm(&RTC_Handle, &alarmstructure, alarm_id, RTC_FORMAT_BCD);
	if(alarmstructure.AlarmTime.Hours - 0x01 > 0x00){
		alarmstructure.AlarmTime.Hours -= 0x01;
	}else{
		alarmstructure.AlarmTime.Hours = 0x17;
	}
	HAL_RTC_SetAlarm(&RTC_Handle, &alarmstructure, RTC_FORMAT_BCD);
}

/**
  * @brief  set minute +1 on the RTC alarm registers
  * @note   None
  * @retval None
  */
void led_alarm_minute_plus(){
	HAL_RTC_GetAlarm(&RTC_Handle, &alarmstructure, alarm_id, RTC_FORMAT_BCD);
	if(alarmstructure.AlarmTime.Minutes + 0x01 < 0x3c){
		alarmstructure.AlarmTime.Minutes += 0x01;
	}else{
		alarmstructure.AlarmTime.Minutes = 0x00;
	}
	HAL_RTC_SetAlarm(&RTC_Handle, &alarmstructure, RTC_FORMAT_BCD);
}

/**
  * @brief  set minute -1 on the RTC alarm registers
  * @note   None
  * @retval None
  */
void led_alarm_minute_minus(){
	HAL_RTC_GetAlarm(&RTC_Handle, &alarmstructure, alarm_id, RTC_FORMAT_BCD);
	if(alarmstructure.AlarmTime.Minutes - 0x01 > 0x00){
		alarmstructure.AlarmTime.Minutes -= 0x01;
	}else{
		alarmstructure.AlarmTime.Minutes = 0x3b;
	}
	HAL_RTC_SetAlarm(&RTC_Handle, &alarmstructure, RTC_FORMAT_BCD);
}

/**
  * @brief  Configure the current time, alarm time and date.
  * @param  None
  * @retval None
  */
void RTC_CalendarConfig(void){
	/* Configure the Date */
	/* Set Date: Tuesday February 18th 2014 */
	datestructureset.Year = 	0x14;
	datestructureset.Month = 	RTC_MONTH_FEBRUARY;
	datestructureset.Date = 	0x18;
	datestructureset.WeekDay = 	RTC_WEEKDAY_TUESDAY;

	if(HAL_RTC_SetDate(&RTC_Handle,&datestructureset,RTC_FORMAT_BCD) != HAL_OK){
		/* Initialization Error */
		//Error_Handler();
	}

	/* Configure the Time */
	/* Set Time: 02:00:00 */
	timestructureset.Hours = 	0x02;
	timestructureset.Minutes = 	0x00;
	timestructureset.Seconds = 	0x00;

	if (HAL_RTC_SetTime(&RTC_Handle, &timestructureset, RTC_FORMAT_BCD) != HAL_OK){
		/* Initialization Error */
		//Error_Handler();
	}

	/* Configure the alarm */
	/* Set alarm: 00:00:00 */
	alarmstructure.AlarmTime.Hours = 	0x00;
	alarmstructure.AlarmTime.Minutes = 0x00;
	alarmstructure.AlarmTime.Seconds = 0x00;
	alarm_id = (uint32_t)1;
	alarmstructure.Alarm = alarm_id;

	if (HAL_RTC_SetAlarm_IT(&RTC_Handle, &alarmstructure, RTC_FORMAT_BCD) != HAL_OK){
		/* Initialization Error */
		//Error_Handler();
	}
	/* Enable and set RTC Interrupt */
	HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);

	/* Writes a data in a RTC Backup data Register1 */
	HAL_RTCEx_BKUPWrite(&RTC_Handle, RTC_BKP_DR1, 0x32F2);
}

/**
  * @brief  deactivates/activate the alarm interrupt ... enabling/disabling rtc alarm interrupt happens only
  * 		if FlagStatus flag has been changed.
  * @param  None
  * @retval None
  */
void alarm_IT(FunctionalState flag){
	if(flag == ENABLE){
		__HAL_RTC_ALARM_DISABLE_IT(&RTC_Handle, RTC_IT_ALRA);
	}else if(flag == DISABLE){
		__HAL_RTC_ALARM_ENABLE_IT(&RTC_Handle, RTC_IT_ALRA);
	}
}

/**
  * @brief  Either locks or unlocks the alarm.
  * @param  None
  * @retval None
  */
void alarm_lock(Alarm_Mode alarm_lock){
	alarm_mode = alarm_lock;
}

/**
  * @brief  RTC alarm callback
  * @param  None
  * @retval None
  */
void RTC_AlarmEventCallback(){
	if(alarm_mode == UNLOCKED){
		buzzer_start();
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
