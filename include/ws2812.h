/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WS2812_H
#define __WS2812_H

/* Includes */
#include "stm32f1xx.h"

/* Exported constants */
/* amount of rows */
#define ROW 7
/* amount of columns */
#define COL 17

/* Exported macro */

/* Exported structs */
typedef struct {
	uint8_t 	number_construction[3][7];	// a number has a resolution of 7*3 Pixels
}Number;

typedef struct {
	uint8_t 	letter_construction[5][7];	// a letter has a resolution of 7*3 Pixels
}Letter;

/* Exported functions */
void init_ws2812(void);
void sendbuf_WS2812();
void WS2812_framedata_setPixel(uint8_t row, uint16_t column, uint8_t red, uint8_t green, uint8_t blue);
void DMA_SetConfiguration(DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength);
//void TIM2_IRQHandlerCall(void);
//void DMA2_Stream7_IRQHandlerCall(void);
void TransferComplete(DMA_HandleTypeDef *DmaHandle);
void TransferError(DMA_HandleTypeDef *DmaHandle);
void sendbuf_WS2812(void);
void WS2812_configuration(uint8_t row, uint16_t column);
void init_gpio(void);
void init_timer(void);
void init_dma(void);
void init_font(void);
Letter char_to_letter(char charistic);
void WS2812_color_wheel_plus(uint8_t *red, uint8_t *green, uint8_t *blue);
void WS2812_color_wheel_minus(uint8_t *red, uint8_t *green, uint8_t *blue);
void WS2812_led_test(void);
void WS2812_set_line(int8_t row_start, int16_t column_start, int8_t row_end, int16_t column_end, uint8_t red, uint8_t green, uint8_t blue);
void WS2812_clear_buffer(void);
void WS2812_background_matrix(void);
void WS2812_TIM2_callback(void);
void WS2812_foreground_colour(uint8_t red, uint8_t green, uint8_t blue);
uint8_t WS2812_display_flash(uint32_t speed_ms, uint8_t flash_count);
void draw_letter(char character, int16_t x_offset, int8_t y_offset, uint8_t *red, uint8_t *green, uint8_t *blue, uint16_t *ambient_factor);
void draw_number(char character, int16_t x_offset, int8_t y_offset, uint8_t *red, uint8_t *green, uint8_t *blue, uint16_t *ambient_factor);
void draw_string(char *string, int16_t x_offset, int8_t y_offset, uint8_t *red, uint8_t *green, uint8_t *blue, uint16_t *ambient_factor);
Letter char_to_letter(char charistic);
Number char_to_number(char charistic);
uint8_t WS2812_display_colorfall(void);
void WS2812_stop_animation(void);

#endif
