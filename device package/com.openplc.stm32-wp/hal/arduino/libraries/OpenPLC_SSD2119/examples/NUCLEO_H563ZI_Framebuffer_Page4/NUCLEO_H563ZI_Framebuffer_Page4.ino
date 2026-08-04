#include <SPI.h>
#include <OpenPLC_LCD.h>

static uint32_t lastFrameMs = 0U;
static uint32_t frameNumber = 0U;

void setup()
{
  OpenPLC_LCD_Init();
}

void loop()
{
  OpenPLC_LCD_Task();

  uint32_t now = millis();
  if ((now - lastFrameMs) < 1000UL || OpenPLC_LCD_FrameBusy() != 0U) {
    return;
  }
  lastFrameMs = now;
  frameNumber++;

  char counterText[32];
  snprintf(counterText, sizeof(counterText), "Frame %lu", (unsigned long)frameNumber);

  uint8_t ok = OpenPLC_LCD_FrameBegin(SSD2119_BLACK, 1U);
  ok &= OpenPLC_LCD_FrameText(10U, 50U, "OpenPLC H563", 24U, SSD2119_ORANGE, SSD2119_BLACK, 0U);
  ok &= OpenPLC_LCD_FrameText(10U, 105U, "Full framebuffer", 18U, SSD2119_ORANGE, SSD2119_BLACK, 0U);
  ok &= OpenPLC_LCD_FrameText(10U, 160U, counterText, 18U, SSD2119_ORANGE, SSD2119_BLACK, 0U);
  ok &= OpenPLC_LCD_FrameText(10U, 215U, "SSD2119", 24U, SSD2119_ORANGE, SSD2119_BLACK, 0U);

  if (ok != 0U) {
    ok &= OpenPLC_LCD_FramePresent();
  }
  if (ok == 0U) {
    OpenPLC_LCD_FrameAbort();
  }
}