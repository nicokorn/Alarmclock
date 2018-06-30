/* Includes */
#include "ws2812.h"
#include "buzzer.h"
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
	MODE_TIME_SET_ALARM_min
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

/* Exported constants */

/* Exported macro */

/* Exported structs */
typedef struct {
	uint8_t 	number_construction[3][7];	// a number has a resolution of 7*3 Pixels
}Number;

typedef struct {
	Wordclock_Mode 		mode;
	RTC_TimeTypeDef 	timestructure;
	RTC_DateTypeDef 	datestructure;
	RTC_AlarmTypeDef 	alarmstructure;
	uint8_t				red;
	uint8_t				green;
	uint8_t				blue;
	uint8_t				color_index;
	uint16_t			event;
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
void draw_number(Number number, uint16_t x_offset, uint8_t y_offset, uint8_t *red, uint8_t *green, uint8_t *blue);
void RTC_CalendarConfig(Alarmclock *alarmclock_param);
void RTC_CalendarShow(uint8_t *showtime, uint8_t *showdate);
void draw_hh_mm(Time_Setup time, Alarmclock *alarmclock_param);
void alarm_IT(FunctionalState flag);
void RTC_AlarmEventCallback();
void increment_mode(Alarmclock *alarmclock_param);
void increment_clock_color(Alarmclock *alarmclock_param);
void decrement_clock_color(Alarmclock *alarmclock_param);
void init_clock(Alarmclock *alarmclock_param);
void setup_clock_blinking(Alarmclock *alarmclock_param);
void get_clock_preferences(Alarmclock *alarmclock_param);
void set_clock_preferences(Alarmclock *alarmclock_param);
void refresh_clock_display(Alarmclock *alarmclock_param);



