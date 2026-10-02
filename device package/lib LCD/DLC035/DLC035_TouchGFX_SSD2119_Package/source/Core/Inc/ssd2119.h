#ifndef __SSD2119_H__
#define __SSD2119_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "Adafruit_GFX.h"

#define SSD2119_WIDTH   320U
#define SSD2119_HEIGHT  240U

#define SSD2119_BLACK   0x0000U
#define SSD2119_WHITE   0xFFFFU
#define SSD2119_RED     0xF800U
#define SSD2119_GREEN   0x0f21U
#define SSD2119_BLUE    0x001FU
#define SSD2119_YELLOW  0xFFE0U
#define SSD2119_ORANGE  0xfba1U
#define SSD2119_CYAN    0x07FFU
#define SSD2119_MAGENTA 0xF81FU

HAL_StatusTypeDef SSD2119_Init(SPI_HandleTypeDef *hspi);
void SSD2119_Reset(void);
HAL_StatusTypeDef SSD2119_SetWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
HAL_StatusTypeDef SSD2119_WritePixels(const uint16_t *pixels, uint32_t count);
HAL_StatusTypeDef SSD2119_WriteBitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *pixels);
uint8_t SSD2119_IsTransferBusy(void);
void SSD2119_TxCpltCallback(void);
void SSD2119_FillScreen(uint16_t color);
void SSD2119_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void SSD2119_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void SSD2119_SetFont(const GFXfont *font);
const GFXfont *SSD2119_GetFont(void);
void SSD2119_DrawChar(uint16_t x, uint16_t y, char ch, uint16_t fg, uint16_t bg, uint8_t scale);
void SSD2119_Print(uint16_t x, uint16_t y, const char *text, uint16_t fg, uint16_t bg, uint8_t scale);

#ifdef __cplusplus
}
#endif

#endif /* __SSD2119_H__ */



