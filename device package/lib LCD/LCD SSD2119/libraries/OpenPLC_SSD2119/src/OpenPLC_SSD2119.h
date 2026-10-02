#ifndef OPENPLC_SSD2119_H
#define OPENPLC_SSD2119_H

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>

#define SSD2119_WIDTH   320U
#define SSD2119_HEIGHT  240U

#define SSD2119_BLACK   0x0000U
#define SSD2119_WHITE   0xFFFFU
#define SSD2119_RED     0xF800U
#define SSD2119_GREEN   0x0F21U
#define SSD2119_BLUE    0x001FU
#define SSD2119_YELLOW  0xFFE0U
#define SSD2119_ORANGE  0xFBA1U
#define SSD2119_CYAN    0x07FFU
#define SSD2119_MAGENTA 0xF81FU

class OpenPLC_SSD2119 : public Adafruit_GFX {
public:
  OpenPLC_SSD2119(int8_t cs, int8_t dc, int8_t rst, SPIClass &spi = SPI);

  bool begin(uint32_t spiFrequency = 12000000UL);
  void reset();

  void drawPixel(int16_t x, int16_t y, uint16_t color) override;
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void fillScreen(uint16_t color);
  void writeRGB565Row(
    uint16_t x,
    uint16_t y,
    const uint16_t *pixels,
    uint16_t pixelCount
  );

  void setPanelRotation180(bool enabled);
  bool getPanelRotation180() const;

private:
  static constexpr uint8_t REG_OSC_START         = 0x00;
  static constexpr uint8_t REG_OUTPUT_CTRL       = 0x01;
  static constexpr uint8_t REG_LCD_DRIVE_AC_CTRL = 0x02;
  static constexpr uint8_t REG_PWR_CTRL_1        = 0x03;
  static constexpr uint8_t REG_DISPLAY_CTRL      = 0x07;
  static constexpr uint8_t REG_FRAME_CYCLE_CTRL  = 0x0B;
  static constexpr uint8_t REG_PWR_CTRL_2        = 0x0C;
  static constexpr uint8_t REG_PWR_CTRL_3        = 0x0D;
  static constexpr uint8_t REG_PWR_CTRL_4        = 0x0E;
  static constexpr uint8_t REG_GATE_SCAN_START   = 0x0F;
  static constexpr uint8_t REG_SLEEP_MODE_1      = 0x10;
  static constexpr uint8_t REG_ENTRY_MODE        = 0x11;
  static constexpr uint8_t REG_SLEEP_MODE_2      = 0x12;
  static constexpr uint8_t REG_GEN_IF_CTRL       = 0x15;
  static constexpr uint8_t REG_PWR_CTRL_5        = 0x1E;
  static constexpr uint8_t REG_RAM_DATA          = 0x22;
  static constexpr uint8_t REG_RAM_WRITE_MASK_1  = 0x23;
  static constexpr uint8_t REG_RAM_WRITE_MASK_2  = 0x24;
  static constexpr uint8_t REG_FRAME_FREQ_CTRL   = 0x25;
  static constexpr uint8_t REG_ANALOG_SET        = 0x26;
  static constexpr uint8_t REG_VCOM_OTP_1        = 0x28;
  static constexpr uint8_t REG_GAMMA_CTRL_1      = 0x30;
  static constexpr uint8_t REG_GAMMA_CTRL_2      = 0x31;
  static constexpr uint8_t REG_GAMMA_CTRL_3      = 0x32;
  static constexpr uint8_t REG_GAMMA_CTRL_4      = 0x33;
  static constexpr uint8_t REG_GAMMA_CTRL_5      = 0x34;
  static constexpr uint8_t REG_GAMMA_CTRL_6      = 0x35;
  static constexpr uint8_t REG_GAMMA_CTRL_7      = 0x36;
  static constexpr uint8_t REG_GAMMA_CTRL_8      = 0x37;
  static constexpr uint8_t REG_GAMMA_CTRL_9      = 0x3A;
  static constexpr uint8_t REG_GAMMA_CTRL_10     = 0x3B;
  static constexpr uint8_t REG_V_RAM_POS         = 0x44;
  static constexpr uint8_t REG_H_RAM_START       = 0x45;
  static constexpr uint8_t REG_H_RAM_END         = 0x46;
  static constexpr uint8_t REG_X_RAM_ADDR        = 0x4E;
  static constexpr uint8_t REG_Y_RAM_ADDR        = 0x4F;

  int8_t _cs;
  int8_t _dc;
  int8_t _rst;
  SPIClass *_spi;
  SPISettings _spiSettings;
  bool _panelRotation180;

  void select();
  void deselect();
  void commandMode();
  void dataMode();
  void writeCommand(uint8_t command);
  void writeData16(uint16_t data);
  void writeRegister(uint8_t reg, uint16_t data);
  void setWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
  void writeColorStream(uint16_t color, uint32_t count);
  void writePixelStream(
    const uint16_t *pixels,
    uint16_t pixelCount,
    bool reverseOrder
  );
};

#endif /* OPENPLC_SSD2119_H */