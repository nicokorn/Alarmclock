/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LIGHTSENSOR_H
#define __LIGHTSENSOR_H

/* Includes */
#include "stm32f1xx.h"
#include "ws2812.h"
#include "clock.h"

/* Exported types */
/*
* @brief  enumeration for touchbuttons
  */

/* Exported constants */

/* Exported macro */

/* Exported functions */
void init_lightsensor(Alarmclock *alarmclock_param);
void init_gpio_lightsensor(void);
void init_adc_lightsensor(void);
void start_lightsensor_adc_conversion(void);
void stop_lightsensor_adc_conversion(void);
void get_lightsensor_adc_conversion(uint32_t *adc_conversion);
void init_dma_lightsensor();
void init_timer_lightsensor(void);
void init_filter();

#endif
