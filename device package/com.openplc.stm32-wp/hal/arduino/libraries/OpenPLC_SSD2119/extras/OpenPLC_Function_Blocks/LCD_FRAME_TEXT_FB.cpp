/* OpenPLC C/C++ Function Block: LCD_FRAME_TEXT

Variable table:
  EXEC        Input   BOOL
  X           Input   UINT
  Y           Input   UINT
  TEXT        Input   STRING
  FONT_SIZE   Input   USINT
  FONT_COLOR  Input   UINT
  BG_COLOR    Input   UINT
  CLEAR_BG    Input   BOOL
  DONE        Output  BOOL
  ERROR       Output  BOOL

Keep the Ladder rung enabled and apply a one-scan pulse to EXEC. Create any
number of instances. This FB draws into RAM only; it does not write SPI and
requires an active frame started by LCD_FRAME_BEGIN.
*/

#include <stdint.h>

extern "C" uint8_t OpenPLC_LCD_FrameText(
  uint16_t x,
  uint16_t y,
  const char *text,
  uint8_t fontSize,
  uint16_t fontColor,
  uint16_t backgroundColor,
  uint8_t clearTextBackground
);

void setup()
{
  DONE = false;
  ERROR = false;
}

void loop()
{
  DONE = false;
  ERROR = false;

  if (!EXEC) {
    return;
  }

  uint8_t ok = OpenPLC_LCD_FrameText(
    (uint16_t)X,
    (uint16_t)Y,
    TEXT.c_str(),
    (uint8_t)FONT_SIZE,
    (uint16_t)FONT_COLOR,
    (uint16_t)BG_COLOR,
    CLEAR_BG ? 1U : 0U
  );

  if (ok != 0U) {
    DONE = true;
  } else {
    ERROR = true;
  }
}