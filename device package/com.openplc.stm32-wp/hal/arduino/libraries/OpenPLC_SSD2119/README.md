# OpenPLC_SSD2119 Arduino Library

Adafruit_GFX-compatible SSD2119 320x240 SPI TFT driver ported from the STM32CubeIDE HAL reference project.

## Default NUCLEO-H563ZI Pinout

```cpp
MOSI = PB5
SCK  = PA5
MISO = PG9 // not used by this write-only display driver
CS   = PD14
DC   = PD15
RST  = PF3
```

## Dependencies

- Arduino `SPI`
- Adafruit `Adafruit_GFX`

Install "Adafruit GFX Library" from Arduino Library Manager before compiling.

## Basic Use

```cpp
#include <SPI.h>
#include <OpenPLC_SSD2119.h>
#include <Fonts/FreeSerif18pt7b.h>

OpenPLC_SSD2119 lcd(PD14, PD15, PF3);

void setup() {
  SPI.setMOSI(PB5);
  SPI.setSCLK(PA5);
  SPI.begin();

  lcd.begin();
  lcd.fillScreen(SSD2119_BLUE);
  lcd.setFont(&FreeSerif18pt7b);
  lcd.setTextColor(SSD2119_WHITE, SSD2119_BLUE);
  lcd.setCursor(12, 70);
  lcd.print("OpenPLC");
}

void loop() {}
```

## OpenPLC Text Command API

The wrapper exposes a C-linkage API for Function Blocks. The API only queues a draw command; the real SPI drawing is performed later by `OpenPLC_LCD_Task()` so PLC scan work is not forced to draw the display inline.

```cpp
uint8_t OpenPLC_LCD_Text(
  uint16_t x,
  uint16_t y,
  const char *text,
  uint8_t fontSize,
  uint16_t fontColor,
  uint16_t bgColor,
  uint8_t clearBg
);
```

- Returns `1` when the text command was queued.
- Returns `0` when the queue is full.
- `fontSize` maps to `FreeSerif12pt7b`, `FreeSerif18pt7b`, or `FreeSerif24pt7b`.
- The text queue defaults to 4 commands, and text is truncated to 63 characters plus terminator.
- `clearBg != 0` clears only the text bounds before printing.

## Full Framebuffer API

Version 0.2.1 adds an optional 320x240 RGB565 framebuffer. It is enabled automatically for `ARDUINO_NUCLEO_H563ZI` and can also be enabled explicitly with:

```cpp
#define OPENPLC_LCD_FRAMEBUFFER_ENABLED 1
```

The buffer uses 153,600 bytes and is enabled only for NUCLEO-H563ZI WP in the VPP manifest. Function Blocks compose the frame in RAM; `OpenPLC_LCD_Task()` then flushes at most two rows or 1 ms per call.

```cpp
uint8_t OpenPLC_LCD_FrameBegin(uint16_t backgroundColor, uint8_t clearFrame);
uint8_t OpenPLC_LCD_FrameFillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
uint8_t OpenPLC_LCD_FrameText(uint16_t x, uint16_t y, const char *text, uint8_t fontSize, uint16_t fontColor, uint16_t backgroundColor, uint8_t clearTextBackground);
uint8_t OpenPLC_LCD_FramePresent(void);
uint8_t OpenPLC_LCD_FrameAbort(void);
uint8_t OpenPLC_LCD_FrameBusy(void);
uint8_t OpenPLC_LCD_FrameReady(void);
uint8_t OpenPLC_LCD_FrameEnabled(void);
```

Rules:

- Call `FrameBegin()` before any frame drawing API.
- Use `clearFrame = 1` for a complete new page.
- Use `clearFrame = 0` only when the framebuffer is synchronized with the panel.
- Call `FramePresent()` once after all drawing calls.
- Do not start another frame while `FrameBusy()` returns 1.
- A full-frame clear changes RAM only; it does not blank the physical LCD.
- For partial updates, use `FrameBegin(..., 0)`, clear a known row with `FrameFillRect()`, draw the new text, and call `FramePresent()`.
- The legacy `OpenPLC_LCD_Text()` and `OpenPLC_LCD_Page4()` APIs remain available, but must not be mixed with an active framebuffer transaction.

See `examples/NUCLEO_H563ZI_Framebuffer_Page4` and `extras/OpenPLC_Function_Blocks/LCD_FRAME_PAGE4_FB.cpp`.
### Board define note

VPP `hal.define` values are emitted into the generated sketch `defines.h`. Arduino libraries compile as separate translation units and do not automatically include that file. The framebuffer therefore also detects the STM32duino compiler macro `ARDUINO_NUCLEO_H563ZI`, which is present for every library source file on this board.