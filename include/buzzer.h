/* Includes */
#include "stm32f1xx.h"

/* Exported defines */
#define BUZZER_PIN 0x0800

/* Exported constants */

/* Exported macro */

/* Exported functions */
void init_buzzer(uint16_t beep_period);
void init_buzzer_timer(uint16_t beep_period);
void BUZZER_TIM3_callback(void);
void buzzer_stop(void);
void buzzer_start(void);
