/* OpenPLC C/C++ Function Block: LCD_FRAME_PAGE4
Inputs: EN BOOL, LINE1..LINE4 STRING
Outputs: DONE BOOL, BUSY BOOL, ERROR BOOL
*/

#include <Arduino.h>
#include <stdint.h>

extern "C" uint8_t OpenPLC_LCD_FrameBegin(uint16_t backgroundColor, uint8_t clearFrame);
extern "C" uint8_t OpenPLC_LCD_FrameText(
  uint16_t x,
  uint16_t y,
  const char *text,
  uint8_t fontSize,
  uint16_t fontColor,
  uint16_t backgroundColor,
  uint8_t clearTextBackground
);
extern "C" uint8_t OpenPLC_LCD_FramePresent(void);
extern "C" uint8_t OpenPLC_LCD_FrameAbort(void);
extern "C" uint8_t OpenPLC_LCD_FrameBusy(void);

static uint32_t lastUpdateMs = 0U;
static bool firstUpdate = true;

void setup()
{
  lastUpdateMs = 0U;
  firstUpdate = true;
  DONE = false;
  BUSY = false;
  ERROR = false;
}

void loop()
{
  DONE = false;
  ERROR = false;
  BUSY = OpenPLC_LCD_FrameBusy() != 0U;

  if (!EN || BUSY) {
    if (!EN) {
      firstUpdate = true;
    }
    return;
  }

  uint32_t now = millis();
  if (!firstUpdate && ((now - lastUpdateMs) < 1000UL)) {
    return;
  }

  uint8_t ok = OpenPLC_LCD_FrameBegin(0x0000U, 1U);
  ok &= OpenPLC_LCD_FrameText(10U, 50U, LINE1.c_str(), 24U, 0xFBA1U, 0x0000U, 0U);
  ok &= OpenPLC_LCD_FrameText(10U, 105U, LINE2.c_str(), 24U, 0xFBA1U, 0x0000U, 0U);
  ok &= OpenPLC_LCD_FrameText(10U, 160U, LINE3.c_str(), 24U, 0xFBA1U, 0x0000U, 0U);
  ok &= OpenPLC_LCD_FrameText(10U, 215U, LINE4.c_str(), 24U, 0xFBA1U, 0x0000U, 0U);

  if (ok != 0U) {
    ok &= OpenPLC_LCD_FramePresent();
  }

  if (ok != 0U) {
    lastUpdateMs = now;
    firstUpdate = false;
    DONE = true;
    BUSY = true;
  } else {
    OpenPLC_LCD_FrameAbort();
    ERROR = true;
  }
}