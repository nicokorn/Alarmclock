/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __EVENT_H
#define __EVENT_H

/* Includes */
#include "stm32f1xx.h"

/* Exported defines */

/* Exported constants */

/* Exported macro */

/* Exported functions */
uint16_t unqueue_event(void);
void queue_event(uint16_t GPIO_Pin);
void clear_event_queue(void);
void init_event_engine(void);

#endif
