/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BUTTON_H
#define __BUTTON_H

/* Includes */
#include "stm32f1xx.h"

/* Exported defines */
#define END_OF_QEUE		((uint16_t)0x0000)
#define BUTTON_MODE		GPIO_PIN_0
#define BUTTON_PLUS		GPIO_PIN_1
#define BUTTON_MINUS	GPIO_PIN_2
#define BUTTON_SNOOZE	GPIO_PIN_12
#define SWITCH_ALARM	GPIO_PIN_10

/* Exported constants */

/* Exported macro */

/* Exported functions */
void init_buttons(void);
void enable_buttons(void);
void disable_buttons(void);

#endif
