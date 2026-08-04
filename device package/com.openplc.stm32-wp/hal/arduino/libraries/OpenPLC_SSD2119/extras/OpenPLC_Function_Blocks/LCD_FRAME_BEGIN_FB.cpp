/* OpenPLC C/C++ Function Block: LCD_FRAME_BEGIN

Variable table:
  EXEC      Input   BOOL
  BG_COLOR  Input   UINT
  CLEAR     Input   BOOL
  DONE      Output  BOOL
  BUSY      Output  BOOL
  ERROR     Output  BOOL

Keep the Ladder rung enabled and apply a one-scan pulse to EXEC.
DONE pulses when the framebuffer transaction starts.
*/

#include <stdint.h>

extern "C" uint8_t OpenPLC_LCD_FrameBegin(
  uint16_t backgroundColor,
  uint8_t clearFrame
);
extern "C" uint8_t OpenPLC_LCD_FrameBusy(void);

void setup()
{
  DONE = false;
  BUSY = false;
  ERROR = false;
}

void loop()
{
  DONE = false;
  ERROR = false;
  BUSY = OpenPLC_LCD_FrameBusy() != 0U;

  if (!EXEC || BUSY) {
    return;
  }

  uint8_t ok = OpenPLC_LCD_FrameBegin(
    (uint16_t)BG_COLOR,
    CLEAR ? 1U : 0U
  );

  if (ok != 0U) {
    DONE = true;
  } else {
    ERROR = true;
  }
}