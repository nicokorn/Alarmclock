/* Includes */
#include "stm32f1xx.h"

/* Exported types */

/* Exported constants */

/* Exported macro */

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
void WS2812_color_wheel_plus(uint8_t *red, uint8_t *green, uint8_t *blue);
void WS2812_color_wheel_minus(uint8_t *red, uint8_t *green, uint8_t *blue);
void WS2812_led_test(void);
void WS2812_set_line(int8_t row_start, int16_t column_start, int8_t row_end, int16_t column_end, uint8_t red, uint8_t green, uint8_t blue);
void WS2812_clear_buffer(void);
void WS2812_background_matrix(void);
void WS2812_background_equalizer(void);
void WS2812_set_clock_fx(uint8_t *fx_mode, uint32_t *counter, uint8_t *red, uint8_t *green, uint8_t *blue);
void WS2812_TIM2_callback(void);
