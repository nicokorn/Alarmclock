/* Includes */
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

/* Exported functions */
void init_clock(void);
void draw_time(uint8_t *red, uint8_t *green, uint8_t *blue);
void led_clock_hour_plus(void);
void led_clock_hour_minus(void);
void led_clock_minute_plus(void);
void led_clock_minute_minus(void);
void led_alarm_hour_plus(void);
void led_alarm_hour_minus(void);
void led_alarm_minute_plus(void);
void led_alarm_minute_minus(void);
void draw_number(Number number, uint16_t x_offset, uint8_t y_offset, uint8_t *red, uint8_t *green, uint8_t *blue);
void RTC_CalendarConfig(void);
void RTC_CalendarShow(uint8_t *showtime, uint8_t *showdate);
void draw_hh_mm(Time_Setup time, Wordclock_Mode mode, uint8_t *red, uint8_t *green, uint8_t *blue);
void alarm_IT(FunctionalState flag);
void RTC_AlarmEventCallback();



