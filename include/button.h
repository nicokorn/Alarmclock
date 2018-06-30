/* Includes */
#include "stm32f1xx.h"

/* Exported defines */
#define END_OF_QEUE		((uint16_t)0x0000)
#define BUTTON_MODE		GPIO_PIN_0
#define BUTTON_PLUS		GPIO_PIN_1
#define BUTTON_MINUS	GPIO_PIN_2
#define BUTTON_SNOOZE	GPIO_PIN_12

/* Exported constants */

/* Exported macro */

/* Exported functions */
void init_buttons(void);
