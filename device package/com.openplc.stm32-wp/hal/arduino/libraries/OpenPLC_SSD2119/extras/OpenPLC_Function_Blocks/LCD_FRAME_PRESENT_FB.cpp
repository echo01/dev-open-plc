/* OpenPLC C/C++ Function Block: LCD_FRAME_PRESENT

Variable table:
  EXEC      Input   BOOL
  DONE      Output  BOOL
  BUSY      Output  BOOL
  ERROR     Output  BOOL

Keep the Ladder rung enabled and apply a one-scan pulse to EXEC.
DONE pulses only after OpenPLC_LCD_Task() has flushed the pending dirty area
to the SSD2119. BUSY stays true while the frame is being transferred.
Use one LCD_FRAME_PRESENT instance per LCD.
*/

#include <stdint.h>

extern "C" uint8_t OpenPLC_LCD_FramePresent(void);
extern "C" uint8_t OpenPLC_LCD_FrameBusy(void);

static uint8_t lcdFramePresentActive = 0U;

void setup()
{
  lcdFramePresentActive = 0U;
  DONE = false;
  BUSY = false;
  ERROR = false;
}

void loop()
{
  DONE = false;
  ERROR = false;

  if (lcdFramePresentActive != 0U) {
    BUSY = OpenPLC_LCD_FrameBusy() != 0U;
    if (!BUSY) {
      lcdFramePresentActive = 0U;
      DONE = true;
    }
    return;
  }

  BUSY = false;
  if (!EXEC) {
    return;
  }

  uint8_t ok = OpenPLC_LCD_FramePresent();
  if (ok != 0U) {
    lcdFramePresentActive = 1U;
    BUSY = true;
  } else {
    ERROR = true;
  }
}