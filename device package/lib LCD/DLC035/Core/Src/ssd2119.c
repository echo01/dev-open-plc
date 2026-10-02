#include "ssd2119.h"

#define SSD2119_REG_OSC_START          0x00U
#define SSD2119_REG_OUTPUT_CTRL        0x01U
#define SSD2119_REG_LCD_DRIVE_AC_CTRL  0x02U
#define SSD2119_REG_PWR_CTRL_1         0x03U
#define SSD2119_REG_DISPLAY_CTRL       0x07U
#define SSD2119_REG_FRAME_CYCLE_CTRL   0x0BU
#define SSD2119_REG_PWR_CTRL_2         0x0CU
#define SSD2119_REG_PWR_CTRL_3         0x0DU
#define SSD2119_REG_PWR_CTRL_4         0x0EU
#define SSD2119_REG_GATE_SCAN_START    0x0FU
#define SSD2119_REG_SLEEP_MODE_1       0x10U
#define SSD2119_REG_ENTRY_MODE         0x11U
#define SSD2119_REG_SLEEP_MODE_2       0x12U
#define SSD2119_REG_GEN_IF_CTRL        0x15U
#define SSD2119_REG_PWR_CTRL_5         0x1EU
#define SSD2119_REG_RAM_DATA           0x22U
#define SSD2119_REG_RAM_WRITE_MASK_1   0x23U
#define SSD2119_REG_RAM_WRITE_MASK_2   0x24U
#define SSD2119_REG_FRAME_FREQ_CTRL    0x25U
#define SSD2119_REG_ANALOG_SET         0x26U
#define SSD2119_REG_VCOM_OTP_1         0x28U
#define SSD2119_REG_GAMMA_CTRL_1       0x30U
#define SSD2119_REG_GAMMA_CTRL_2       0x31U
#define SSD2119_REG_GAMMA_CTRL_3       0x32U
#define SSD2119_REG_GAMMA_CTRL_4       0x33U
#define SSD2119_REG_GAMMA_CTRL_5       0x34U
#define SSD2119_REG_GAMMA_CTRL_6       0x35U
#define SSD2119_REG_GAMMA_CTRL_7       0x36U
#define SSD2119_REG_GAMMA_CTRL_8       0x37U
#define SSD2119_REG_GAMMA_CTRL_9       0x3AU
#define SSD2119_REG_GAMMA_CTRL_10      0x3BU
#define SSD2119_REG_V_RAM_POS          0x44U
#define SSD2119_REG_H_RAM_START        0x45U
#define SSD2119_REG_H_RAM_END          0x46U
#define SSD2119_REG_X_RAM_ADDR         0x4EU
#define SSD2119_REG_Y_RAM_ADDR         0x4FU

#define SSD2119_SPI_TIMEOUT_MS         100U

/* The startup solid bars showed red/blue swapped with BGR=1 (0x7AEF).
   Restore BGR=0; this does not alter SPI byte order or scan direction. */
#define SSD2119_OUTPUT_CTRL_DLC035      0x72EFU
static SPI_HandleTypeDef *ssd2119_hspi;
static const GFXfont *ssd2119_font;
static volatile uint8_t ssd2119_transfer_busy;

static inline void SSD2119_CS_Low(void)
{
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
}

static inline void SSD2119_CS_High(void)
{
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

static inline void SSD2119_DC_Command(void)
{
  HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);
}

static inline void SSD2119_DC_Data(void)
{
  HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
}

static HAL_StatusTypeDef SSD2119_WriteBytes(const uint8_t *data, uint16_t size)
{
  return HAL_SPI_Transmit(ssd2119_hspi, (uint8_t *)data, size, SSD2119_SPI_TIMEOUT_MS);
}

static HAL_StatusTypeDef SSD2119_WriteCommand(uint8_t command)
{
  HAL_StatusTypeDef status;

  SSD2119_CS_Low();
  SSD2119_DC_Command();
  status = SSD2119_WriteBytes(&command, 1U);
  SSD2119_CS_High();

  return status;
}

static HAL_StatusTypeDef SSD2119_WriteData16(uint16_t data)
{
  uint8_t bytes[2];
  HAL_StatusTypeDef status;

  bytes[0] = (uint8_t)(data >> 8);
  bytes[1] = (uint8_t)(data & 0xFFU);

  SSD2119_CS_Low();
  SSD2119_DC_Data();
  status = SSD2119_WriteBytes(bytes, 2U);
  SSD2119_CS_High();

  return status;
}

static HAL_StatusTypeDef SSD2119_WriteRegister(uint8_t reg, uint16_t data)
{
  HAL_StatusTypeDef status = SSD2119_WriteCommand(reg);

  if (status == HAL_OK)
  {
    status = SSD2119_WriteData16(data);
  }

  return status;
}

HAL_StatusTypeDef SSD2119_SetWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  uint16_t x_end = (uint16_t)(x + w - 1U);
  uint16_t y_end = (uint16_t)(y + h - 1U);
  uint16_t phys_x;
  uint16_t phys_y;
  uint16_t phys_x_end;
  uint16_t phys_y_end;
  HAL_StatusTypeDef status = HAL_OK;

  if ((ssd2119_hspi == NULL) || (x >= SSD2119_WIDTH) || (y >= SSD2119_HEIGHT) || (w == 0U) || (h == 0U))
  {
    return HAL_ERROR;
  }

  if (x_end >= SSD2119_WIDTH)
  {
    x_end = SSD2119_WIDTH - 1U;
  }
  if (y_end >= SSD2119_HEIGHT)
  {
    y_end = SSD2119_HEIGHT - 1U;
  }

  /* The mounted DLC0350QEM06DT-1 glass is 180 degrees from the logical UI. */
  phys_x = (uint16_t)((SSD2119_WIDTH - 1U) - x_end);
  phys_y = (uint16_t)((SSD2119_HEIGHT - 1U) - y_end);
  phys_x_end = (uint16_t)((SSD2119_WIDTH - 1U) - x);
  phys_y_end = (uint16_t)((SSD2119_HEIGHT - 1U) - y);

  status |= SSD2119_WriteRegister(SSD2119_REG_V_RAM_POS, (uint16_t)((phys_y_end << 8) | phys_y));
  status |= SSD2119_WriteRegister(SSD2119_REG_H_RAM_START, phys_x);
  status |= SSD2119_WriteRegister(SSD2119_REG_H_RAM_END, phys_x_end);
  status |= SSD2119_WriteRegister(SSD2119_REG_X_RAM_ADDR, phys_x);
  status |= SSD2119_WriteRegister(SSD2119_REG_Y_RAM_ADDR, phys_y);
  status |= SSD2119_WriteCommand(SSD2119_REG_RAM_DATA);

  return (status == HAL_OK) ? HAL_OK : HAL_ERROR;
}
static void SSD2119_WriteColorStream(uint16_t color, uint32_t count)
{
  uint8_t bytes[64];
  const uint32_t pixels_per_chunk = sizeof(bytes) / 2U;

  for (uint32_t i = 0U; i < pixels_per_chunk; i++)
  {
    bytes[(i * 2U)] = (uint8_t)(color >> 8);
    bytes[(i * 2U) + 1U] = (uint8_t)(color & 0xFFU);
  }

  SSD2119_CS_Low();
  SSD2119_DC_Data();
  while (count > 0U)
  {
    uint16_t chunk_pixels = (count > pixels_per_chunk) ? (uint16_t)pixels_per_chunk : (uint16_t)count;
    (void)SSD2119_WriteBytes(bytes, (uint16_t)(chunk_pixels * 2U));
    count -= chunk_pixels;
  }
  SSD2119_CS_High();
}

static void SSD2119_GetGlyph(char ch, uint8_t glyph[5])
{
  static const uint8_t unknown[5] = {0x02U, 0x01U, 0x51U, 0x09U, 0x06U};
  static const uint8_t space[5] = {0x00U, 0x00U, 0x00U, 0x00U, 0x00U};
  static const uint8_t h_upper[5] = {0x7FU, 0x08U, 0x08U, 0x08U, 0x7FU};
  static const uint8_t e_lower[5] = {0x38U, 0x54U, 0x54U, 0x54U, 0x18U};
  static const uint8_t l_lower[5] = {0x00U, 0x41U, 0x7FU, 0x40U, 0x00U};
  static const uint8_t o_lower[5] = {0x38U, 0x44U, 0x44U, 0x44U, 0x38U};
  const uint8_t *src = unknown;

  switch (ch)
  {
    case ' ': src = space; break;
    case 'H': src = h_upper; break;
    case 'e': src = e_lower; break;
    case 'l': src = l_lower; break;
    case 'o': src = o_lower; break;
    default: break;
  }

  for (uint8_t i = 0U; i < 5U; i++)
  {
    glyph[i] = src[i];
  }
}
static void SSD2119_FillRectClipped(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color)
{
  if ((w == 0U) || (h == 0U) || (x >= (int16_t)SSD2119_WIDTH) || (y >= (int16_t)SSD2119_HEIGHT))
  {
    return;
  }

  if (x < 0)
  {
    uint16_t trim = (uint16_t)(-x);
    if (trim >= w)
    {
      return;
    }
    w = (uint16_t)(w - trim);
    x = 0;
  }

  if (y < 0)
  {
    uint16_t trim = (uint16_t)(-y);
    if (trim >= h)
    {
      return;
    }
    h = (uint16_t)(h - trim);
    y = 0;
  }

  if (((uint16_t)x + w) > SSD2119_WIDTH)
  {
    w = (uint16_t)(SSD2119_WIDTH - (uint16_t)x);
  }
  if (((uint16_t)y + h) > SSD2119_HEIGHT)
  {
    h = (uint16_t)(SSD2119_HEIGHT - (uint16_t)y);
  }

  SSD2119_FillRect((uint16_t)x, (uint16_t)y, w, h, color);
}

static uint16_t SSD2119_GFXCharAdvance(char ch, uint8_t scale)
{
  const GFXfont *font = ssd2119_font;
  const GFXglyph *glyph;

  if ((font == NULL) || ((uint8_t)ch < font->first) || ((uint8_t)ch > font->last))
  {
    return (uint16_t)(6U * scale);
  }

  glyph = &font->glyph[(uint8_t)ch - font->first];
  return (uint16_t)glyph->xAdvance * scale;
}

static void SSD2119_DrawGFXChar(int16_t x, int16_t baseline, char ch, uint16_t color, uint8_t scale)
{
  const GFXfont *font = ssd2119_font;
  const GFXglyph *glyph;
  const uint8_t *bitmap;
  uint16_t bitmap_offset;
  uint8_t bits = 0U;
  uint8_t bit = 0U;

  if ((font == NULL) || ((uint8_t)ch < font->first) || ((uint8_t)ch > font->last))
  {
    return;
  }

  glyph = &font->glyph[(uint8_t)ch - font->first];
  bitmap = font->bitmap;
  bitmap_offset = glyph->bitmapOffset;

  for (uint8_t yy = 0U; yy < glyph->height; yy++)
  {
    for (uint8_t xx = 0U; xx < glyph->width; xx++)
    {
      if ((bit & 0x07U) == 0U)
      {
        bits = pgm_read_byte(&bitmap[bitmap_offset]);
        bitmap_offset++;
      }

      if ((bits & 0x80U) != 0U)
      {
        int16_t pixel_x = (int16_t)(x + ((int16_t)glyph->xOffset + xx) * scale);
        int16_t pixel_y = (int16_t)(baseline + ((int16_t)glyph->yOffset + yy) * scale);
        SSD2119_FillRectClipped(pixel_x, pixel_y, scale, scale, color);
      }

      bits <<= 1;
      bit++;
    }
  }
}

void SSD2119_Reset(void)
{
  SSD2119_CS_High();
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(20U);
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(120U);
}

HAL_StatusTypeDef SSD2119_Init(SPI_HandleTypeDef *hspi)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (hspi == NULL)
  {
    return HAL_ERROR;
  }

  ssd2119_hspi = hspi;
  SSD2119_Reset();

  status |= SSD2119_WriteRegister(SSD2119_REG_SLEEP_MODE_1, 0x0001U);
  HAL_Delay(10U);
  status |= SSD2119_WriteRegister(SSD2119_REG_PWR_CTRL_5, 0x00BAU);
  status |= SSD2119_WriteRegister(SSD2119_REG_VCOM_OTP_1, 0x0006U);
  status |= SSD2119_WriteRegister(SSD2119_REG_OSC_START, 0x0001U);
  HAL_Delay(10U);
  status |= SSD2119_WriteRegister(SSD2119_REG_OUTPUT_CTRL, SSD2119_OUTPUT_CTRL_DLC035);
  status |= SSD2119_WriteRegister(SSD2119_REG_LCD_DRIVE_AC_CTRL, 0x0600U);
  status |= SSD2119_WriteRegister(SSD2119_REG_SLEEP_MODE_1, 0x0000U);
  HAL_Delay(30U);
  status |= SSD2119_WriteRegister(SSD2119_REG_ENTRY_MODE, 0x6830U);
  status |= SSD2119_WriteRegister(SSD2119_REG_SLEEP_MODE_2, 0x0999U);
  status |= SSD2119_WriteRegister(SSD2119_REG_GEN_IF_CTRL, 0x0000U);
  status |= SSD2119_WriteRegister(SSD2119_REG_GATE_SCAN_START, 0x0000U);
  status |= SSD2119_WriteRegister(SSD2119_REG_FRAME_CYCLE_CTRL, 0x5308U);
  status |= SSD2119_WriteRegister(SSD2119_REG_PWR_CTRL_2, 0x0003U);
  status |= SSD2119_WriteRegister(SSD2119_REG_PWR_CTRL_3, 0x000AU);
  status |= SSD2119_WriteRegister(SSD2119_REG_PWR_CTRL_4, 0x2E00U);
  status |= SSD2119_WriteRegister(SSD2119_REG_PWR_CTRL_1, 0x000AU);
  status |= SSD2119_WriteRegister(SSD2119_REG_RAM_WRITE_MASK_1, 0x0000U);
  status |= SSD2119_WriteRegister(SSD2119_REG_RAM_WRITE_MASK_2, 0x0000U);
  status |= SSD2119_WriteRegister(SSD2119_REG_FRAME_FREQ_CTRL, 0x8000U);
  status |= SSD2119_WriteRegister(SSD2119_REG_ANALOG_SET, 0x7800U);

  status |= SSD2119_WriteRegister(SSD2119_REG_GAMMA_CTRL_1, 0x0000U);
  status |= SSD2119_WriteRegister(SSD2119_REG_GAMMA_CTRL_2, 0x0104U);
  status |= SSD2119_WriteRegister(SSD2119_REG_GAMMA_CTRL_3, 0x0100U);
  status |= SSD2119_WriteRegister(SSD2119_REG_GAMMA_CTRL_4, 0x0305U);
  status |= SSD2119_WriteRegister(SSD2119_REG_GAMMA_CTRL_5, 0x0505U);
  status |= SSD2119_WriteRegister(SSD2119_REG_GAMMA_CTRL_6, 0x0305U);
  status |= SSD2119_WriteRegister(SSD2119_REG_GAMMA_CTRL_7, 0x0707U);
  status |= SSD2119_WriteRegister(SSD2119_REG_GAMMA_CTRL_8, 0x0300U);
  status |= SSD2119_WriteRegister(SSD2119_REG_GAMMA_CTRL_9, 0x1200U);
  status |= SSD2119_WriteRegister(SSD2119_REG_GAMMA_CTRL_10, 0x0800U);

  status |= SSD2119_WriteRegister(SSD2119_REG_DISPLAY_CTRL, 0x0033U);
  SSD2119_FillScreen(SSD2119_BLACK);

  return (status == HAL_OK) ? HAL_OK : HAL_ERROR;
}

HAL_StatusTypeDef SSD2119_WritePixels(const uint16_t *pixels, uint32_t count)
{
  uint8_t bytes[128];
  const uint32_t pixels_per_chunk = sizeof(bytes) / 2U;
  HAL_StatusTypeDef status = HAL_OK;

  if ((ssd2119_hspi == NULL) || ((pixels == NULL) && (count > 0U)))
  {
    return HAL_ERROR;
  }

  ssd2119_transfer_busy = 1U;
  SSD2119_CS_Low();
  SSD2119_DC_Data();

  while ((count > 0U) && (status == HAL_OK))
  {
    uint16_t chunk_pixels = (count > pixels_per_chunk) ? (uint16_t)pixels_per_chunk : (uint16_t)count;

    for (uint16_t i = 0U; i < chunk_pixels; i++)
    {
      uint16_t color = pixels[i];
      bytes[(i * 2U)] = (uint8_t)(color >> 8);
      bytes[(i * 2U) + 1U] = (uint8_t)(color & 0xFFU);
    }

    status = SSD2119_WriteBytes(bytes, (uint16_t)(chunk_pixels * 2U));
    pixels += chunk_pixels;
    count -= chunk_pixels;
  }

  SSD2119_CS_High();
  ssd2119_transfer_busy = 0U;

  return status;
}

HAL_StatusTypeDef SSD2119_WriteBitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *pixels)
{
  uint16_t src_stride = w;
  uint8_t bytes[128];
  const uint32_t pixels_per_chunk = sizeof(bytes) / 2U;
  uint32_t total_pixels;
  uint32_t sent_pixels = 0U;
  HAL_StatusTypeDef status;

  if ((pixels == NULL) || (w == 0U) || (h == 0U))
  {
    return HAL_ERROR;
  }
  if ((x >= SSD2119_WIDTH) || (y >= SSD2119_HEIGHT))
  {
    return HAL_ERROR;
  }

  if ((x + w) > SSD2119_WIDTH)
  {
    w = (uint16_t)(SSD2119_WIDTH - x);
  }
  if ((y + h) > SSD2119_HEIGHT)
  {
    h = (uint16_t)(SSD2119_HEIGHT - y);
  }

  status = SSD2119_SetWindow(x, y, w, h);
  if (status != HAL_OK)
  {
    return status;
  }

  /* SetWindow maps the mounted panel by 180 degrees, so bitmap blocks must be
     streamed bottom-right to top-left to preserve TouchGFX logical orientation. */
  total_pixels = (uint32_t)w * h;
  ssd2119_transfer_busy = 1U;
  SSD2119_CS_Low();
  SSD2119_DC_Data();

  while ((sent_pixels < total_pixels) && (status == HAL_OK))
  {
    uint16_t chunk_pixels = ((total_pixels - sent_pixels) > pixels_per_chunk) ?
                            (uint16_t)pixels_per_chunk :
                            (uint16_t)(total_pixels - sent_pixels);

    for (uint16_t i = 0U; i < chunk_pixels; i++)
    {
      uint32_t logical_index = sent_pixels + i;
      uint16_t src_y = (uint16_t)((h - 1U) - (logical_index / w));
      uint16_t src_x = (uint16_t)((w - 1U) - (logical_index % w));
      uint16_t color = pixels[((uint32_t)src_y * src_stride) + src_x];

      bytes[(i * 2U)] = (uint8_t)(color >> 8);
      bytes[(i * 2U) + 1U] = (uint8_t)(color & 0xFFU);
    }

    status = SSD2119_WriteBytes(bytes, (uint16_t)(chunk_pixels * 2U));
    sent_pixels += chunk_pixels;
  }

  SSD2119_CS_High();
  ssd2119_transfer_busy = 0U;

  return status;
}
uint8_t SSD2119_IsTransferBusy(void)
{
  return ssd2119_transfer_busy;
}

void SSD2119_TxCpltCallback(void)
{
  ssd2119_transfer_busy = 0U;
  SSD2119_CS_High();
}

void SSD2119_FillScreen(uint16_t color)
{
  SSD2119_FillRect(0U, 0U, SSD2119_WIDTH, SSD2119_HEIGHT, color);
}

void SSD2119_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
  if ((x >= SSD2119_WIDTH) || (y >= SSD2119_HEIGHT))
  {
    return;
  }

  SSD2119_SetWindow(x, y, 1U, 1U);
  SSD2119_WriteColorStream(color, 1U);
}

void SSD2119_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
  if ((x >= SSD2119_WIDTH) || (y >= SSD2119_HEIGHT) || (w == 0U) || (h == 0U))
  {
    return;
  }

  if ((x + w) > SSD2119_WIDTH)
  {
    w = (uint16_t)(SSD2119_WIDTH - x);
  }
  if ((y + h) > SSD2119_HEIGHT)
  {
    h = (uint16_t)(SSD2119_HEIGHT - y);
  }

  SSD2119_SetWindow(x, y, w, h);
  SSD2119_WriteColorStream(color, (uint32_t)w * h);
}

void SSD2119_DrawChar(uint16_t x, uint16_t y, char ch, uint16_t fg, uint16_t bg, uint8_t scale)
{
  uint8_t glyph[5];

  if (scale == 0U)
  {
    scale = 1U;
  }

  SSD2119_GetGlyph(ch, glyph);
  for (uint8_t col = 0U; col < 6U; col++)
  {
    uint8_t line = (col < 5U) ? glyph[col] : 0x00U;
    for (uint8_t row = 0U; row < 8U; row++)
    {
      uint16_t color = ((line & (1U << row)) != 0U) ? fg : bg;
      SSD2119_FillRect((uint16_t)(x + (col * scale)),
                       (uint16_t)(y + (row * scale)),
                       scale,
                       scale,
                       color);
    }
  }
}

void SSD2119_SetFont(const GFXfont *font)
{
  ssd2119_font = font;
}

const GFXfont *SSD2119_GetFont(void)
{
  return ssd2119_font;
}

void SSD2119_Print(uint16_t x, uint16_t y, const char *text, uint16_t fg, uint16_t bg, uint8_t scale)
{
  uint16_t cursor_x = x;
  uint16_t cursor_y = y;

  if (text == NULL)
  {
    return;
  }
  if (scale == 0U)
  {
    scale = 1U;
  }

  while (*text != '\0')
  {
    if (*text == '\n')
    {
      cursor_x = x;
      cursor_y = (uint16_t)(cursor_y + ((ssd2119_font != NULL) ? (ssd2119_font->yAdvance * scale) : (8U * scale)));
    }
    else if (ssd2119_font != NULL)
    {
      uint16_t advance = SSD2119_GFXCharAdvance(*text, scale);
      SSD2119_FillRectClipped((int16_t)cursor_x,
                              (int16_t)cursor_y,
                              advance,
                              (uint16_t)(ssd2119_font->yAdvance * scale),
                              bg);
      SSD2119_DrawGFXChar((int16_t)cursor_x,
                          (int16_t)(cursor_y + (ssd2119_font->yAdvance * scale)),
                          *text,
                          fg,
                          scale);
      cursor_x = (uint16_t)(cursor_x + advance);
    }
    else
    {
      SSD2119_DrawChar(cursor_x, cursor_y, *text, fg, bg, scale);
      cursor_x = (uint16_t)(cursor_x + (6U * scale));
    }
    text++;
  }
}






