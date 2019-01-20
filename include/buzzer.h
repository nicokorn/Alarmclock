/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BUZZER_H
#define __BUZZER_H

/* Includes */
#include "stm32f1xx.h"
#include "ws2812.h"
#include "clock.h"
#include "event.h"


/* Exported defines */
#define BUZZER_PIN 0x0800

/* Exported constants */

/* Exported macro */

/* Exported functions */
void init_buzzer(Alarmclock *alarmclock_param);
void BUZZER_TIM4_callback(Alarmclock *alarmclock_param);
void SNOOZE_TIM3_callback(Alarmclock *alarmclock_param);
void buzzer_stop(Alarmclock *alarmclock_param);
void buzzer_start(Alarmclock *alarmclock_param);
void snooze(Alarmclock *alarmclock_param);
void snooze_reset(Alarmclock *alarmclock_param);
void set_snooze(Alarmclock *alarmclock_param);

/* Exported types */
/*
* @brief  enumeration buzzer states
  */
typedef enum
{
  BUZZER_RESET = 0U,
  BUZZER_SET
}BUZZER_State;

/*
* @brief  enumeration snooze states
  */
typedef enum
{
  SNOOZE_RESET = 0U,
  SNOOZE_SET
}SNOOZE_State;

#endif
