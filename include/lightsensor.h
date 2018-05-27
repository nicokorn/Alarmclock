/* Includes */
#include "stm32f1xx.h"

/* Exported types */
/*
* @brief  enumeration for touchbuttons
  */

/* Exported constants */

/* Exported macro */

/* Exported functions */
void init_lightsensor(void);
void init_gpio_microphone(void);
void init_adc_microphone(void);
void start_microphone_adc_conversion(void);
void stop_microphone_adc_conversion(void);
void get_microphone_adc_conversion(uint32_t *adc_conversion);
