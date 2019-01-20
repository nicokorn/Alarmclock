/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CLOCK_H
#define __CLOCK_H

/* Includes */
#include <stdio.h>
#include "ws2812.h"
#include "button.h"
//#include "lightsensor.h"
#include "stm32f1xx.h"

/* Exported types */
/*
* @brief  enumeration for clock modes
  */
typedef enum
{
	MODE_TIME_CLOCK,
	MODE_TIME_SET_CLOCK_h,
	MODE_TIME_SET_CLOCK_min,
	MODE_TIME_SET_ALARM_h,
	MODE_TIME_SET_ALARM_min,
	MODE_TIME_SET_SNOOZE,
	MODE_TIME_SET_ALARM_STYLE,
	MODE_TIME_LUX
}Wordclock_Mode;

/*
* @brief  enumeration for blinking
  */
typedef enum
{
	MINUTES,
	HOURS,
	CLOCK,
	ALARM,
	DISABLED
}Time_Setup;

/*
* @brief  enumeration for alarm modes
  */
typedef enum
{
	LOCKED,
	UNLOCKED
}Alarm_Mode;

/*
* @brief  enumeration for alarm styles
  */
typedef enum
{
	FLASHING,
	COLORFALL
}Alarm_Style;

/* Exported constants */

/* Exported macro */

typedef struct {
	Wordclock_Mode 		mode;
	RTC_TimeTypeDef 	timestructure;
	RTC_DateTypeDef 	datestructure;
	RTC_AlarmTypeDef 	alarmstructure;
	uint8_t				red;
	uint8_t				green;
	uint8_t				blue;
	uint8_t				color_index;
	uint8_t				buzzer_state;
	uint8_t				snooze_state;
	uint8_t				snooze_duration;
	Alarm_Style			alarm_style;
	uint16_t			event;
	uint16_t*			ambient_light_factor;
}Alarmclock;

/* Exported functions */
void init_RTC( Alarmclock *alarmclock_param);
void draw_time(Alarmclock *alarmclock_param);
void led_clock_hour_plus(Alarmclock *alarmclock_param);
void led_clock_hour_minus(Alarmclock *alarmclock_param);
void led_clock_minute_plus(Alarmclock *alarmclock_param);
void led_clock_minute_minus(Alarmclock *alarmclock_param);
void led_alarm_hour_plus(Alarmclock *alarmclock_param);
void led_alarm_hour_minus(Alarmclock *alarmclock_param);
void led_alarm_minute_plus(Alarmclock *alarmclock_param);
void led_alarm_minute_minus(Alarmclock *alarmclock_param);
void RTC_CalendarConfig(Alarmclock *alarmclock_param);
void RTC_CalendarShow(uint8_t *showtime, uint8_t *showdate);
void draw_hh_mm(Time_Setup time, Alarmclock *alarmclock_param);
void alarm_IT(FunctionalState flag);
void RTC_AlarmEventCallback(Alarmclock *alarmclock_param);
void increment_mode(Alarmclock *alarmclock_param);
void increment_clock_color(Alarmclock *alarmclock_param);
void decrement_clock_color(Alarmclock *alarmclock_param);
void init_clock(Alarmclock *alarmclock_param);
void setup_clock_blinking(Alarmclock *alarmclock_param);
void get_clock_preferences(Alarmclock *alarmclock_param);
void set_clock_preferences(Alarmclock *alarmclock_param);
void refresh_clock_display(Alarmclock *alarmclock_param);
void draw_mode(Alarmclock *alarmclock_param);
uint8_t read_alarm_switch(void);
void draw_lux(Alarmclock *alarmclock_param);
void set_alarm_irq(FunctionalState alarm_irq, Alarmclock *alarmclock_param);
void clock_intro();
void alarm_style_plus(Alarmclock *alarmclock_param);
void alarm_style_minus(Alarmclock *alarmclock_param);
void show_alarm_style(Alarmclock *alarmclock_param);
void snooze_plus(Alarmclock *alarmclock_param);
void snooze_minus(Alarmclock *alarmclock_param);
void draw_snooze(Alarmclock *alarmclock_param);

#endif
