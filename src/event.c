/*
 * Autor: Nico Korn
 * Date: 09.12.2018
 * Firmware for a alarmlcock with custom made STM32F103 microcontroller board.
 *  *
 * Copyright (c) 2018 Nico Korn
 *
 * event.c this module contents init and functions for the event management.
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

#include "event.h"
#include "buzzer.h"
#include "button.h"
#include "ws2812.h"
#include "stm32f1xx.h"

/* defines */
#define EVENT_QUEUE_LENGTH		11 		// length of event buffer
#define END_OF_QUEUE			((uint16_t)0x1010)

/* variables */
static uint16_t					event_queue[EVENT_QUEUE_LENGTH];

/**
  * @brief  this function initiates the queue
  * @param  None
  * @retval None
  */
void init_event_engine(){
	event_queue[0] = END_OF_QUEUE;
}

/**
  * @brief  this function queues a new event
  * @param  GPIO_Pin
  * @retval event
  */
void queue_event(uint16_t GPIO_Pin){
	uint16_t index = 0;
	uint16_t run_flag = 1;

	/* save button event into event queue */
	while(index < EVENT_QUEUE_LENGTH && run_flag == 1){
		if(event_queue[EVENT_QUEUE_LENGTH-1] == END_OF_QUEUE){	//check if queue is full
			/* stop here because the queue is already full */
			run_flag = 0;
		}else if(event_queue[index] == END_OF_QUEUE){	//safe new event into queue
			event_queue[index] = GPIO_Pin;
			event_queue[index+1] = END_OF_QUEUE;
			run_flag = 0;
		}
		index++;
	}
}

/**
  * @brief  this function takes out recent events from the queue
  * @param  None
  * @retval event
  */
uint16_t unqueue_event(void){
	uint16_t latest_event;
	uint16_t index = 0;

	/* save first entity of the event queue into local variable */
	latest_event = event_queue[0];

	/* check for new event */
	if(latest_event != END_OF_QUEUE){

		/* shift event queue, because the newest event has been taken out */
		do{
			event_queue[index] = event_queue[index+1];
			index++;
		}while(event_queue[index-1] != END_OF_QUEUE);
		/* only 1 END_OF_QUEUE shall be as the last valid queue entity -> empty entities shall have zeros e.g. [0] = BUTTON_1, [1] = END_OF_QUEUE, [2...10] = 0 */
		event_queue[index] = 0;
	}else{
		/* do nothing, because no new event has been saved into the event queue: first entity -> END_OF_QUEUE */
	}
	return latest_event;
}

/**
  * @brief  this function clears the event queue
  * @param  None
  * @retval None
  */
void clear_event_queue(void){
	uint16_t index = 0;

	/* fill the event queue with zeros and put END_OF_QUEUE at the begining of the queue */
	for(index = 0; index < EVENT_QUEUE_LENGTH; index++){
		event_queue[index] = 0;
	}
	event_queue[0] = END_OF_QUEUE;
}
