#include "OpenPLC_SSD2119.h"

OpenPLC_SSD2119::OpenPLC_SSD2119(int8_t cs, int8_t dc, int8_t rst, SPIClass &spi)
  : Adafruit_GFX(SSD2119_WIDTH, SSD2119_HEIGHT),
    _cs(cs),
    _dc(dc),
    _rst(rst),
    _spi(&spi),
    _spiSettings(12000000UL, MSBFIRST, SPI_MODE0),
    _panelRotation180(true)
{
}

bool OpenPLC_SSD2119::begin(uint32_t spiFrequency)
{
  _spiSettings = SPISettings(spiFrequency, MSBFIRST, SPI_MODE0);

  pinMode(_cs, OUTPUT);
  pinMode(_dc, OUTPUT);
  pinMode(_rst, OUTPUT);
  digitalWrite(_cs, HIGH);
  digitalWrite(_dc, LOW);
  digitalWrite(_rst, HIGH);

  _spi->begin();
  reset();

  writeRegister(REG_SLEEP_MODE_1, 0x0001U);
  delay(10);
  writeRegister(REG_PWR_CTRL_5, 0x00BAU);
  writeRegister(REG_VCOM_OTP_1, 0x0006U);
  writeRegister(REG_OSC_START, 0x0001U);
  delay(10);
  writeRegister(REG_OUTPUT_CTRL, 0x72EFU);
  writeRegister(REG_LCD_DRIVE_AC_CTRL, 0x0600U);
  writeRegister(REG_SLEEP_MODE_1, 0x0000U);
  delay(30);
  writeRegister(REG_ENTRY_MODE, 0x6830U);
  writeRegister(REG_SLEEP_MODE_2, 0x0999U);
  writeRegister(REG_GEN_IF_CTRL, 0x0000U);
  writeRegister(REG_GATE_SCAN_START, 0x0000U);
  writeRegister(REG_FRAME_CYCLE_CTRL, 0x5308U);
  writeRegister(REG_PWR_CTRL_2, 0x0003U);
  writeRegister(REG_PWR_CTRL_3, 0x000AU);
  writeRegister(REG_PWR_CTRL_4, 0x2E00U);
  writeRegister(REG_PWR_CTRL_1, 0x000AU);
  writeRegister(REG_RAM_WRITE_MASK_1, 0x0000U);
  writeRegister(REG_RAM_WRITE_MASK_2, 0x0000U);
  writeRegister(REG_FRAME_FREQ_CTRL, 0x8000U);
  writeRegister(REG_ANALOG_SET, 0x7800U);

  writeRegister(REG_GAMMA_CTRL_1, 0x0000U);
  writeRegister(REG_GAMMA_CTRL_2, 0x0104U);
  writeRegister(REG_GAMMA_CTRL_3, 0x0100U);
  writeRegister(REG_GAMMA_CTRL_4, 0x0305U);
  writeRegister(REG_GAMMA_CTRL_5, 0x0505U);
  writeRegister(REG_GAMMA_CTRL_6, 0x0305U);
  writeRegister(REG_GAMMA_CTRL_7, 0x0707U);
  writeRegister(REG_GAMMA_CTRL_8, 0x0300U);
  writeRegister(REG_GAMMA_CTRL_9, 0x1200U);
  writeRegister(REG_GAMMA_CTRL_10, 0x0800U);

  writeRegister(REG_DISPLAY_CTRL, 0x0033U);
  fillScreen(SSD2119_BLACK);

  return true;
}

void OpenPLC_SSD2119::reset()
{
  digitalWrite(_cs, HIGH);
  digitalWrite(_rst, LOW);
  delay(20);
  digitalWrite(_rst, HIGH);
  delay(120);
}

void OpenPLC_SSD2119::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if ((x < 0) || (y < 0) || (x >= (int16_t)SSD2119_WIDTH) || (y >= (int16_t)SSD2119_HEIGHT)) {
    return;
  }

  setWindow((uint16_t)x, (uint16_t)y, 1U, 1U);
  writeColorStream(color, 1U);
}

void OpenPLC_SSD2119::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  if ((w <= 0) || (h <= 0) || (x >= (int16_t)SSD2119_WIDTH) || (y >= (int16_t)SSD2119_HEIGHT)) {
    return;
  }

  if (x < 0) {
    w += x;
    x = 0;
  }
  if (y < 0) {
    h += y;
    y = 0;
  }
  if (w <= 0 || h <= 0) {
    return;
  }
  if ((x + w) > (int16_t)SSD2119_WIDTH) {
    w = (int16_t)SSD2119_WIDTH - x;
  }
  if ((y + h) > (int16_t)SSD2119_HEIGHT) {
    h = (int16_t)SSD2119_HEIGHT - y;
  }

  setWindow((uint16_t)x, (uint16_t)y, (uint16_t)w, (uint16_t)h);
  writeColorStream(color, (uint32_t)w * (uint32_t)h);
}

void OpenPLC_SSD2119::fillScreen(uint16_t color)
{
  fillRect(0, 0, SSD2119_WIDTH, SSD2119_HEIGHT, color);
}

void OpenPLC_SSD2119::writeRGB565Row(
  uint16_t x,
  uint16_t y,
  const uint16_t *pixels,
  uint16_t pixelCount
)
{
  if (pixels == nullptr || pixelCount == 0U ||
      x >= SSD2119_WIDTH || y >= SSD2119_HEIGHT) {
    return;
  }

  if (((uint32_t)x + pixelCount) > SSD2119_WIDTH) {
    pixelCount = (uint16_t)(SSD2119_WIDTH - x);
  }

  setWindow(x, y, pixelCount, 1U);
  writePixelStream(pixels, pixelCount, _panelRotation180);
}

void OpenPLC_SSD2119::setPanelRotation180(bool enabled)
{
  _panelRotation180 = enabled;
}

bool OpenPLC_SSD2119::getPanelRotation180() const
{
  return _panelRotation180;
}

void OpenPLC_SSD2119::select()
{
  _spi->beginTransaction(_spiSettings);
  digitalWrite(_cs, LOW);
}

void OpenPLC_SSD2119::deselect()
{
  digitalWrite(_cs, HIGH);
  _spi->endTransaction();
}

void OpenPLC_SSD2119::commandMode()
{
  digitalWrite(_dc, LOW);
}

void OpenPLC_SSD2119::dataMode()
{
  digitalWrite(_dc, HIGH);
}

void OpenPLC_SSD2119::writeCommand(uint8_t command)
{
  select();
  commandMode();
  _spi->transfer(command);
  deselect();
}

void OpenPLC_SSD2119::writeData16(uint16_t data)
{
  select();
  dataMode();
  _spi->transfer((uint8_t)(data >> 8));
  _spi->transfer((uint8_t)(data & 0xFFU));
  deselect();
}

void OpenPLC_SSD2119::writeRegister(uint8_t reg, uint16_t data)
{
  writeCommand(reg);
  writeData16(data);
}

void OpenPLC_SSD2119::setWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  uint16_t xEnd = x + w - 1U;
  uint16_t yEnd = y + h - 1U;
  uint16_t physX;
  uint16_t physY;
  uint16_t physXEnd;
  uint16_t physYEnd;

  if (xEnd >= SSD2119_WIDTH) {
    xEnd = SSD2119_WIDTH - 1U;
  }
  if (yEnd >= SSD2119_HEIGHT) {
    yEnd = SSD2119_HEIGHT - 1U;
  }

  if (_panelRotation180) {
    physX = (uint16_t)((SSD2119_WIDTH - 1U) - xEnd);
    physY = (uint16_t)((SSD2119_HEIGHT - 1U) - yEnd);
    physXEnd = (uint16_t)((SSD2119_WIDTH - 1U) - x);
    physYEnd = (uint16_t)((SSD2119_HEIGHT - 1U) - y);
  } else {
    physX = x;
    physY = y;
    physXEnd = xEnd;
    physYEnd = yEnd;
  }

  writeRegister(REG_V_RAM_POS, (uint16_t)((physYEnd << 8) | physY));
  writeRegister(REG_H_RAM_START, physX);
  writeRegister(REG_H_RAM_END, physXEnd);
  writeRegister(REG_X_RAM_ADDR, physX);
  writeRegister(REG_Y_RAM_ADDR, physY);
  writeCommand(REG_RAM_DATA);
}

void OpenPLC_SSD2119::writeColorStream(uint16_t color, uint32_t count)
{
  uint8_t buffer[64];
  const uint32_t pixelsPerChunk = sizeof(buffer) / 2U;
  const uint8_t colorHi = (uint8_t)(color >> 8);
  const uint8_t colorLo = (uint8_t)(color & 0xFFU);

  select();
  dataMode();
  while (count > 0U) {
    uint16_t chunkPixels = (count > pixelsPerChunk) ? (uint16_t)pixelsPerChunk : (uint16_t)count;

    /* Arduino SPI.transfer(buffer, size) overwrites the TX buffer with RX data.
       LCD MISO is not connected on this hardware, so refill every chunk. */
    for (uint16_t i = 0; i < chunkPixels; i++) {
      buffer[i * 2U] = colorHi;
      buffer[(i * 2U) + 1U] = colorLo;
    }

    _spi->transfer(buffer, (size_t)chunkPixels * 2U);
    count -= chunkPixels;
  }
  deselect();
}

void OpenPLC_SSD2119::writePixelStream(
  const uint16_t *pixels,
  uint16_t pixelCount,
  bool reverseOrder
)
{
  uint8_t buffer[64];
  const uint16_t pixelsPerChunk = (uint16_t)(sizeof(buffer) / 2U);
  uint16_t transferred = 0U;

  select();
  dataMode();
  while (transferred < pixelCount) {
    uint16_t remaining = (uint16_t)(pixelCount - transferred);
    uint16_t chunkPixels = remaining > pixelsPerChunk ? pixelsPerChunk : remaining;

    for (uint16_t i = 0U; i < chunkPixels; i++) {
      uint16_t sourceIndex = reverseOrder
        ? (uint16_t)(pixelCount - 1U - transferred - i)
        : (uint16_t)(transferred + i);
      uint16_t color = pixels[sourceIndex];
      buffer[i * 2U] = (uint8_t)(color >> 8);
      buffer[(i * 2U) + 1U] = (uint8_t)(color & 0xFFU);
    }

    _spi->transfer(buffer, (size_t)chunkPixels * 2U);
    transferred = (uint16_t)(transferred + chunkPixels);
  }
  deselect();
}