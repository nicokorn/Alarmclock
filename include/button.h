/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BUTTON_H
#define __BUTTON_H

/* Includes */
#include "stm32f1xx.h"
#include "event.h"
#include "ws2812.h"

/* Exported defines */
#define BUTTON_MODE				GPIO_PIN_0
#define BUTTON_PLUS				GPIO_PIN_1
#define BUTTON_MINUS			GPIO_PIN_2
#define BUTTON_SNOOZE			GPIO_PIN_12
#define BUTTON_SNOOZE_DOUBLE	0xEFFF	//~GPIO_PIN_12
#define SWITCH_ALARM			GPIO_PIN_10

/* Exported constants */

/* Exported macro */

/* Exported functions */
void init_buttons(void);
void init_button_timer(void);
void set_button_irq(FunctionalState button_irq);
void BUTTON_TIM1_Callback(void);
void button_TIM1_start(void);
void button_TIM1_stop(void);

#endif
