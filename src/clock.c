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

/* variables */
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
static Letter 				e;
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
static uint8_t	 			h0, h1, m0, m1;
static uint8_t	 			h0_old, h1_old, m0_old, m1_old, init;
static uint8_t	 			h0_time_change_flag, h1_time_change_flag, m0_time_change_flag, m1_time_change_flag;
static int16_t 				h0_x_offset, h1_x_offset, m0_x_offset, m1_x_offset, x_offset;
static int8_t 				h0_y_offset, h1_y_offset, m0_y_offset, m1_y_offset, y_offset;
static uint8_t 				color_pattern[4][3] = {
									{0x05, 0x05, 0x05},	//white
									{0x05, 0x00, 0x00},	//red
									{0x00, 0x05, 0x00},	//green
									{0x00, 0x00, 0x05}	//blue
									};
static uint32_t				hal_tick_temp;
extern uint8_t 				WS2812_TC;					// led transmission flag
extern uint16_t				adc_raw;

/* global variables */
RTC_HandleTypeDef 			RTC_Handle;

/**
  * @brief  initialization of words & rtc
  * @note   None
  * @retval None
  */
void init_RTC( Alarmclock *alarmclock_param){
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
	a.letter_construction[1][1] = 0;
	a.letter_construction[2][1] = 1;
	a.letter_construction[3][1] = 0;
	a.letter_construction[4][1] = 0;

	a.letter_construction[0][2] = 0;
	a.letter_construction[1][2] = 1;
	a.letter_construction[2][2] = 0;
	a.letter_construction[3][2] = 1;
	a.letter_construction[4][2] = 0;

	a.letter_construction[0][3] = 0;
	a.letter_construction[1][3] = 1;
	a.letter_construction[2][3] = 1;
	a.letter_construction[3][3] = 1;
	a.letter_construction[4][3] = 0;

	a.letter_construction[0][4] = 0;
	a.letter_construction[1][4] = 1;
	a.letter_construction[2][4] = 0;
	a.letter_construction[3][4] = 1;
	a.letter_construction[4][4] = 0;

	a.letter_construction[0][5] = 0;
	a.letter_construction[1][5] = 1;
	a.letter_construction[2][5] = 0;
	a.letter_construction[3][5] = 1;
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

	e.letter_construction[0][1] = 0;
	e.letter_construction[1][1] = 1;
	e.letter_construction[2][1] = 1;
	e.letter_construction[3][1] = 1;
	e.letter_construction[4][1] = 0;

	e.letter_construction[0][2] = 0;
	e.letter_construction[1][2] = 1;
	e.letter_construction[2][2] = 0;
	e.letter_construction[3][2] = 0;
	e.letter_construction[4][2] = 0;

	e.letter_construction[0][3] = 0;
	e.letter_construction[1][3] = 1;
	e.letter_construction[2][3] = 1;
	e.letter_construction[3][3] = 0;
	e.letter_construction[4][3] = 0;

	e.letter_construction[0][4] = 0;
	e.letter_construction[1][4] = 1;
	e.letter_construction[2][4] = 0;
	e.letter_construction[3][4] = 0;
	e.letter_construction[4][4] = 0;

	e.letter_construction[0][5] = 0;
	e.letter_construction[1][5] = 1;
	e.letter_construction[2][5] = 1;
	e.letter_construction[3][5] = 1;
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

	i.letter_construction[0][1] = 0;
	i.letter_construction[1][1] = 1;
	i.letter_construction[2][1] = 1;
	i.letter_construction[3][1] = 1;
	i.letter_construction[4][1] = 0;

	i.letter_construction[0][2] = 0;
	i.letter_construction[1][2] = 0;
	i.letter_construction[2][2] = 1;
	i.letter_construction[3][2] = 0;
	i.letter_construction[4][2] = 0;

	i.letter_construction[0][3] = 0;
	i.letter_construction[1][3] = 0;
	i.letter_construction[2][3] = 1;
	i.letter_construction[3][3] = 0;
	i.letter_construction[4][3] = 0;

	i.letter_construction[0][4] = 0;
	i.letter_construction[1][4] = 0;
	i.letter_construction[2][4] = 1;
	i.letter_construction[3][4] = 0;
	i.letter_construction[4][4] = 0;

	i.letter_construction[0][5] = 0;
	i.letter_construction[1][5] = 1;
	i.letter_construction[2][5] = 1;
	i.letter_construction[3][5] = 1;
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

	l.letter_construction[0][1] = 0;
	l.letter_construction[1][1] = 1;
	l.letter_construction[2][1] = 0;
	l.letter_construction[3][1] = 0;
	l.letter_construction[4][1] = 0;

	l.letter_construction[0][2] = 0;
	l.letter_construction[1][2] = 1;
	l.letter_construction[2][2] = 0;
	l.letter_construction[3][2] = 0;
	l.letter_construction[4][2] = 0;

	l.letter_construction[0][3] = 0;
	l.letter_construction[1][3] = 1;
	l.letter_construction[2][3] = 0;
	l.letter_construction[3][3] = 0;
	l.letter_construction[4][3] = 0;

	l.letter_construction[0][4] = 0;
	l.letter_construction[1][4] = 1;
	l.letter_construction[2][4] = 0;
	l.letter_construction[3][4] = 0;
	l.letter_construction[4][4] = 0;

	l.letter_construction[0][5] = 0;
	l.letter_construction[1][5] = 1;
	l.letter_construction[2][5] = 1;
	l.letter_construction[3][5] = 1;
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

	r.letter_construction[0][1] = 0;
	r.letter_construction[1][1] = 1;
	r.letter_construction[2][1] = 1;
	r.letter_construction[3][1] = 0;
	r.letter_construction[4][1] = 0;

	r.letter_construction[0][2] = 0;
	r.letter_construction[1][2] = 1;
	r.letter_construction[2][2] = 0;
	r.letter_construction[3][2] = 1;
	r.letter_construction[4][2] = 0;

	r.letter_construction[0][3] = 0;
	r.letter_construction[1][3] = 1;
	r.letter_construction[2][3] = 1;
	r.letter_construction[3][3] = 0;
	r.letter_construction[4][3] = 0;

	r.letter_construction[0][4] = 0;
	r.letter_construction[1][4] = 1;
	r.letter_construction[2][4] = 0;
	r.letter_construction[3][4] = 1;
	r.letter_construction[4][4] = 0;

	r.letter_construction[0][5] = 0;
	r.letter_construction[1][5] = 1;
	r.letter_construction[2][5] = 0;
	r.letter_construction[3][5] = 1;
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
	s.letter_construction[1][1] = 0;
	s.letter_construction[2][1] = 1;
	s.letter_construction[3][1] = 1;
	s.letter_construction[4][1] = 0;

	s.letter_construction[0][2] = 0;
	s.letter_construction[1][2] = 1;
	s.letter_construction[2][2] = 0;
	s.letter_construction[3][2] = 0;
	s.letter_construction[4][2] = 0;

	s.letter_construction[0][3] = 0;
	s.letter_construction[1][3] = 0;
	s.letter_construction[2][3] = 1;
	s.letter_construction[3][3] = 0;
	s.letter_construction[4][3] = 0;

	s.letter_construction[0][4] = 0;
	s.letter_construction[1][4] = 0;
	s.letter_construction[2][4] = 0;
	s.letter_construction[3][4] = 1;
	s.letter_construction[4][4] = 0;

	s.letter_construction[0][5] = 0;
	s.letter_construction[1][5] = 1;
	s.letter_construction[2][5] = 1;
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

	t.letter_construction[0][1] = 0;
	t.letter_construction[1][1] = 1;
	t.letter_construction[2][1] = 1;
	t.letter_construction[3][1] = 1;
	t.letter_construction[4][1] = 0;

	t.letter_construction[0][2] = 0;
	t.letter_construction[1][2] = 0;
	t.letter_construction[2][2] = 1;
	t.letter_construction[3][2] = 0;
	t.letter_construction[4][2] = 0;

	t.letter_construction[0][3] = 0;
	t.letter_construction[1][3] = 0;
	t.letter_construction[2][3] = 1;
	t.letter_construction[3][3] = 0;
	t.letter_construction[4][3] = 0;

	t.letter_construction[0][4] = 0;
	t.letter_construction[1][4] = 0;
	t.letter_construction[2][4] = 1;
	t.letter_construction[3][4] = 0;
	t.letter_construction[4][4] = 0;

	t.letter_construction[0][5] = 0;
	t.letter_construction[1][5] = 0;
	t.letter_construction[2][5] = 1;
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

	u.letter_construction[0][1] = 0;
	u.letter_construction[1][1] = 1;
	u.letter_construction[2][1] = 0;
	u.letter_construction[3][1] = 1;
	u.letter_construction[4][1] = 0;

	u.letter_construction[0][2] = 0;
	u.letter_construction[1][2] = 1;
	u.letter_construction[2][2] = 0;
	u.letter_construction[3][2] = 1;
	u.letter_construction[4][2] = 0;

	u.letter_construction[0][3] = 0;
	u.letter_construction[1][3] = 1;
	u.letter_construction[2][3] = 0;
	u.letter_construction[3][3] = 1;
	u.letter_construction[4][3] = 0;

	u.letter_construction[0][4] = 0;
	u.letter_construction[1][4] = 1;
	u.letter_construction[2][4] = 0;
	u.letter_construction[3][4] = 1;
	u.letter_construction[4][4] = 0;

	u.letter_construction[0][5] = 0;
	u.letter_construction[1][5] = 0;
	u.letter_construction[2][5] = 1;
	u.letter_construction[3][5] = 1;
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

	p.letter_construction[0][1] = 0;
	p.letter_construction[1][1] = 1;
	p.letter_construction[2][1] = 1;
	p.letter_construction[3][1] = 0;
	p.letter_construction[4][1] = 0;

	p.letter_construction[0][2] = 0;
	p.letter_construction[1][2] = 1;
	p.letter_construction[2][2] = 0;
	p.letter_construction[3][2] = 1;
	p.letter_construction[4][2] = 0;

	p.letter_construction[0][3] = 0;
	p.letter_construction[1][3] = 1;
	p.letter_construction[2][3] = 1;
	p.letter_construction[3][3] = 0;
	p.letter_construction[4][3] = 0;

	p.letter_construction[0][4] = 0;
	p.letter_construction[1][4] = 1;
	p.letter_construction[2][4] = 0;
	p.letter_construction[3][4] = 0;
	p.letter_construction[4][4] = 0;

	p.letter_construction[0][5] = 0;
	p.letter_construction[1][5] = 1;
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
	c.letter_construction[1][1] = 0;
	c.letter_construction[2][1] = 1;
	c.letter_construction[3][1] = 1;
	c.letter_construction[4][1] = 0;

	c.letter_construction[0][2] = 0;
	c.letter_construction[1][2] = 1;
	c.letter_construction[2][2] = 0;
	c.letter_construction[3][2] = 0;
	c.letter_construction[4][2] = 0;

	c.letter_construction[0][3] = 0;
	c.letter_construction[1][3] = 1;
	c.letter_construction[2][3] = 0;
	c.letter_construction[3][3] = 0;
	c.letter_construction[4][3] = 0;

	c.letter_construction[0][4] = 0;
	c.letter_construction[1][4] = 1;
	c.letter_construction[2][4] = 0;
	c.letter_construction[3][4] = 0;
	c.letter_construction[4][4] = 0;

	c.letter_construction[0][5] = 0;
	c.letter_construction[1][5] = 0;
	c.letter_construction[2][5] = 1;
	c.letter_construction[3][5] = 1;
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
	o.letter_construction[1][1] = 0;
	o.letter_construction[2][1] = 1;
	o.letter_construction[3][1] = 0;
	o.letter_construction[4][1] = 0;

	o.letter_construction[0][2] = 0;
	o.letter_construction[1][2] = 1;
	o.letter_construction[2][2] = 0;
	o.letter_construction[3][2] = 1;
	o.letter_construction[4][2] = 0;

	o.letter_construction[0][3] = 0;
	o.letter_construction[1][3] = 1;
	o.letter_construction[2][3] = 0;
	o.letter_construction[3][3] = 1;
	o.letter_construction[4][3] = 0;

	o.letter_construction[0][4] = 0;
	o.letter_construction[1][4] = 1;
	o.letter_construction[2][4] = 0;
	o.letter_construction[3][4] = 1;
	o.letter_construction[4][4] = 0;

	o.letter_construction[0][5] = 0;
	o.letter_construction[1][5] = 0;
	o.letter_construction[2][5] = 1;
	o.letter_construction[3][5] = 1;
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

	k.letter_construction[0][1] = 0;
	k.letter_construction[1][1] = 1;
	k.letter_construction[2][1] = 0;
	k.letter_construction[3][1] = 1;
	k.letter_construction[4][1] = 0;

	k.letter_construction[0][2] = 0;
	k.letter_construction[1][2] = 1;
	k.letter_construction[2][2] = 0;
	k.letter_construction[3][2] = 1;
	k.letter_construction[4][2] = 0;

	k.letter_construction[0][3] = 0;
	k.letter_construction[1][3] = 1;
	k.letter_construction[2][3] = 1;
	k.letter_construction[3][3] = 0;
	k.letter_construction[4][3] = 0;

	k.letter_construction[0][4] = 0;
	k.letter_construction[1][4] = 1;
	k.letter_construction[2][4] = 0;
	k.letter_construction[3][4] = 1;
	k.letter_construction[4][4] = 0;

	k.letter_construction[0][5] = 0;
	k.letter_construction[1][5] = 1;
	k.letter_construction[2][5] = 0;
	k.letter_construction[3][5] = 1;
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
	m.letter_construction[0][0] = 0;
	m.letter_construction[1][0] = 0;
	m.letter_construction[2][0] = 0;
	m.letter_construction[3][0] = 0;
	m.letter_construction[4][0] = 0;

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
}

/**
  * @brief  draws a a number into the IO buffer
  * @note   None
  * @retval None
  */
void draw_number(Number number, int16_t x_offset, int8_t y_offset, uint8_t *red, uint8_t *green, uint8_t *blue, uint16_t *ambient_factor){
	for(int16_t x = 0; x < 3; x++){
		for(int8_t y = 0; y < 7; y++){
			if(number.number_construction[x][y] != 0 && (y_offset+y >= 0) && (y_offset+y < ROW) && (x_offset+x >= 0) && (x_offset+x < COL)){
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
void draw_letter(Letter letter, int16_t x_offset, int8_t y_offset, uint8_t *red, uint8_t *green, uint8_t *blue, uint16_t *ambient_factor){
	for(int16_t x = 0; x < 5; x++){
		for(int8_t y = 0; y < 7; y++){
			if(letter.letter_construction[x][y] != 0 && (y_offset+y >= 0) && (y_offset+y < ROW) && (x_offset+x >= 0) && (x_offset+x < COL)){
				WS2812_framedata_setPixel((uint8_t)y_offset + (uint8_t)y, (uint16_t)x_offset + (uint16_t)x, (uint8_t)*red*(uint8_t)*ambient_factor, (uint8_t)*green*(uint8_t)*ambient_factor, (uint8_t)*blue*(uint8_t)*ambient_factor);
			}
		}
	}
}

/**
  * @brief  draws mode as letters
  * @note   None
  * @retval None
  */
void draw_mode(Alarmclock *alarmclock_param){
	switch(alarmclock_param->mode){
		case MODE_TIME_SET_CLOCK_h:
									/* ignore isr */
									isr_disable();
									for(int16_t index = 0; index>-24; index--){
										/* erase frame buffer */
										WS2812_clear_buffer();
										/* write letters into buffer */
										draw_letter(t, index+0, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
										draw_letter(i, index+4, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
										draw_letter(m, index+9, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
										draw_letter(e, index+14, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
										draw_letter(s, index+20, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
										draw_letter(e, index+24, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
										draw_letter(t, index+28, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
										draw_letter(u, index+32, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
										draw_letter(p, index+36, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
										/* wait for the data transmission to the led's to be ready */
										while(!WS2812_TC);
										/* send frame buffer to the leds */
										sendbuf_WS2812();
										/* roll through the text with XX ms period */
										if(index == 0){
											HAL_Delay(500);
										}else{
											HAL_Delay(100);
										}
									}
									/* wait a bit to show the letters */

									HAL_Delay(500);
									isr_enable();
		break;
		case MODE_TIME_SET_ALARM_h:
									/* ignore isr */
									isr_disable();
									for(int16_t index = 0; index>-28; index--){
										/* erase frame buffer */
										WS2812_clear_buffer();
										/* write letters into buffer */
										draw_letter(a, index+0, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
										draw_letter(l, index+4, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
										draw_letter(a, index+8, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
										draw_letter(r, index+12, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
										draw_letter(m, index+17, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
										draw_letter(s, index+24, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
										draw_letter(e, index+28, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
										draw_letter(t, index+32, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
										draw_letter(u, index+36, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
										draw_letter(p, index+40, 0, &alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
										/* wait for the data transmission to the led's to be ready */
										while(!WS2812_TC);
										/* send frame buffer to the leds */
										sendbuf_WS2812();
										/* roll through the text with XX ms period */
										if(index == 0){
											HAL_Delay(500);
										}else{
											HAL_Delay(100);
										}
									}
									/* wait a bit to show the letters */
									HAL_Delay(500);
									isr_enable();
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
	draw_number(doublepoint, (uint16_t)7, (uint8_t)0,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	if(time == HOURS){
		/* draw numbers into display buffer */
		/* number position h0 */
		h0_x_offset = 0;
		h0_y_offset = 0;
		switch(h0){
			case 0:	draw_number(zero, h0_x_offset, h0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 1:	draw_number(one, h0_x_offset, h0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 2:	draw_number(two, h0_x_offset, h0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
		}
		/* number position h1 */
		h1_x_offset = 4;
		h1_y_offset = 0;
		switch(h1){
			case 0:	draw_number(zero, h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 1:	draw_number(one, h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 2:	draw_number(two, h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 3:	draw_number(three, h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 4:	draw_number(four, h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 5:	draw_number(five, h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 6:	draw_number(six, h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 7:	draw_number(seven, h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 8:	draw_number(eight, h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 9:	draw_number(nine, h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
		}
	}else if(time == MINUTES){
		/* number position m0 */
		m0_x_offset = 10;
		m0_y_offset = 0;
		switch(m0){
			case 0:	draw_number(zero, m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 1:	draw_number(one, m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 2:	draw_number(two, m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 3:	draw_number(three, m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 4:	draw_number(four, m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 5:	draw_number(five, m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
		}
		/* number position m1 */
		m1_x_offset = 14;
		m1_y_offset = 0;
		switch(m1){
			case 0:	draw_number(zero, m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 1:	draw_number(one, m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 2:	draw_number(two, m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 3:	draw_number(three, m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 4:	draw_number(four, m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 5:	draw_number(five, m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 6:	draw_number(six, m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 7:	draw_number(seven, m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 8:	draw_number(eight, m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 9:	draw_number(nine, m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
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
					draw_number(two, h0_x_offset, h0_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number(zero, h0_x_offset, h0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 1:	if(h0_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number(zero, h0_x_offset, h0_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number(one, h0_x_offset, h0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 2:	if(h0_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number(one, h0_x_offset, h0_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number(two, h0_x_offset, h0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
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
					draw_number(nine, h1_x_offset, h1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number(zero, h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 1:	if(h1_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number(zero, h1_x_offset, h1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number(one, h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 2:	if(h1_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number(one, h1_x_offset, h1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number(two, h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 3:	if(h1_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number(two, h1_x_offset, h1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number(three, h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 4:	if(h1_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number(three, h1_x_offset, h1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number(four, h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 5:	if(h1_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number(four, h1_x_offset, h1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number(five, h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 6:	if(h1_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number(five, h1_x_offset, h1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number(six, h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 7:	if(h1_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number(six, h1_x_offset, h1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number(seven, h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 8:	if(h1_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number(seven, h1_x_offset, h1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number(eight, h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
		case 9:	if(h1_y_offset < 0){	//old number during time change effect shall running out of the screen
					draw_number(eight, h1_x_offset, h1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
				}
				draw_number(nine, h1_x_offset, h1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
		break;
	}
	/* second double point */
	x_offset = 7;
	y_offset = 0;
	if(alarmclock_param->timestructure.Seconds%2){
		draw_number(doublepoint, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
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
				draw_number(five, m0_x_offset, m0_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number(zero, m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 1:	if(m0_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number(zero, m0_x_offset, m0_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number(one, m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 2:	if(m0_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number(one, m0_x_offset, m0_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number(two, m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 3:	if(m0_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number(two, m0_x_offset, m0_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number(three, m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 4:	if(m0_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number(three, m0_x_offset, m0_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number(four, m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 5:	if(m0_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number(four, m0_x_offset, m0_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number(five, m0_x_offset, m0_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
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
				draw_number(nine, m1_x_offset, m1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number(zero, m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 1:	if(m1_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number(zero, m1_x_offset, m1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number(one, m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 2:	if(m1_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number(one, m1_x_offset, m1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number(two, m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 3:	if(m1_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number(two, m1_x_offset, m1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number(three, m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 4:	if(m1_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number(three, m1_x_offset, m1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number(four, m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 5:	if(m1_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number(four, m1_x_offset, m1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number(five, m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 6:	if(m1_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number(five, m1_x_offset, m1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number(six, m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 7:	if(m1_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number(six, m1_x_offset, m1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number(seven, m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 8:	if(m1_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number(seven, m1_x_offset, m1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number(eight, m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
	break;
	case 9:	if(m1_y_offset < 0){	//old number during time change effect shall running out of the screen
				draw_number(eight, m1_x_offset, m1_y_offset+8,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			}
			draw_number(nine, m1_x_offset, m1_y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
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
void RTC_AlarmEventCallback(){
		buzzer_start();
}

/**
  * @brief  this function increments the recent mode
  * @param  None
  * @retval None
  */
void increment_mode(Alarmclock *alarmclock_param){
	/* reset tick counter, for clean setup blinking of the numbers */
	HAL_SetTick(1);
	if(alarmclock_param->mode < 4){
		alarmclock_param->mode++;
	}else{
		alarmclock_param->mode = 0;
	}
	/* only in the clock mode, the alarm shall work */
	if(alarmclock_param->mode != MODE_TIME_CLOCK){
		/* disable the alarm interrupt */
		HAL_RTC_DeactivateAlarm(&RTC_Handle, alarmclock_param->alarmstructure.Alarm);
	}
	/* draw mode change on the clock display */
	draw_mode(alarmclock_param);
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
	/* get recent hal tick value */
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
  * @brief  this function checks alarm switch position and activates or deactivates it
  * @param  None
  * @retval None
  */
void read_alarm_switch(Alarmclock *alarmclock_param){
	if(GPIOB->IDR &= (uint32_t)SWITCH_ALARM){
		/* disable the alarm interrupt */
		HAL_RTC_DeactivateAlarm(&RTC_Handle, alarmclock_param->alarmstructure.Alarm);
	}else{
		/* Enable and set RTC interrupt for alarm function */
		HAL_RTC_SetAlarm_IT(&RTC_Handle, &alarmclock_param->alarmstructure, RTC_FORMAT_BIN);
		HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 1, 0);
		HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
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
}

/**
  * @brief  this function increments the color
  * @param  None
  * @retval None
  */
void increment_clock_color(Alarmclock *alarmclock_param){
	if(alarmclock_param->color_index < 3){
		alarmclock_param->color_index++;
	}else{
		alarmclock_param->color_index = 0;
	}
	alarmclock_param->red = color_pattern[alarmclock_param->color_index][0];
	alarmclock_param->green = color_pattern[alarmclock_param->color_index][1];
	alarmclock_param->blue = color_pattern[alarmclock_param->color_index][2];
}

/**
  * @brief  this function increments the color
  * @param  None
  * @retval None
  */
void decrement_clock_color(Alarmclock *alarmclock_param){
	if(alarmclock_param->color_index > 0){
		alarmclock_param->color_index--;
	}else{
		alarmclock_param->color_index = 3;
	}
	alarmclock_param->red = color_pattern[alarmclock_param->color_index][0];
	alarmclock_param->green = color_pattern[alarmclock_param->color_index][1];
	alarmclock_param->blue = color_pattern[alarmclock_param->color_index][2];
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
		uint16_t ambientlight_factor;

		/* process hours */
		adc0 = adc_raw % 10;
		adc1 = adc_raw / 10;
		adc1 = adc1 % 10;
		adc2 = adc_raw /100;
		adc2 = adc2 % 10;
		adc3 = adc_raw /1000;
		adc3 = adc3 % 10;

		/* divide by 4 */
		ambientlight_factor = adc_raw >> 2;
		//ambientlight_factor = ambientlight_factor * 0x11;

		if(adc_raw < 5){
			ambientlight_factor = 1;
		}else{
			ambientlight_factor = 8;
		}
		alarmclock_param->red = ambientlight_factor * 0x11;
		alarmclock_param->green = ambientlight_factor * 0x11;
		alarmclock_param->blue = ambientlight_factor * 0x11;

		/* erase frame buffer */
		WS2812_clear_buffer();

		x_offset = 14;
		y_offset = 0;
		switch(adc0){
			case 0:	draw_number(zero, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 1:	draw_number(one, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 2:	draw_number(two, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 3:	draw_number(three, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 4:	draw_number(four, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 5:	draw_number(five, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 6:	draw_number(six, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 7:	draw_number(seven, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 8:	draw_number(eight, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 9:	draw_number(nine, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
		}
		x_offset = 10;
		y_offset = 0;
		switch(adc1){
			case 0:	draw_number(zero, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 1:	draw_number(one, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 2:	draw_number(two, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 3:	draw_number(three, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 4:	draw_number(four, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 5:	draw_number(five, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 6:	draw_number(six, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 7:	draw_number(seven, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 8:	draw_number(eight, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 9:	draw_number(nine, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
		}
		x_offset = 6;
		y_offset = 0;
		switch(adc2){
			case 0:	draw_number(zero, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 1:	draw_number(one, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 2:	draw_number(two, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 3:	draw_number(three, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 4:	draw_number(four, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 5:	draw_number(five, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 6:	draw_number(six, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 7:	draw_number(seven, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 8:	draw_number(eight, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 9:	draw_number(nine, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
		}
		x_offset = 2;
		y_offset = 0;
		switch(adc3){
			case 0:	draw_number(zero, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 1:	draw_number(one, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 2:	draw_number(two, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 3:	draw_number(three, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 4:	draw_number(four, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 5:	draw_number(five, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 6:	draw_number(six, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 7:	draw_number(seven, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 8:	draw_number(eight, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
			case 9:	draw_number(nine, x_offset, y_offset,&alarmclock_param->red, &alarmclock_param->green, &alarmclock_param->blue, alarmclock_param->ambient_light_factor);
			break;
		}
		/* wait for the data transmission to the led's to be ready */
		while(!WS2812_TC);
		/* send frame buffer to the leds */
		sendbuf_WS2812();
}
