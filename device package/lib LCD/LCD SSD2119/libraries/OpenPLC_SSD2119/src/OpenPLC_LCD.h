#ifndef OPENPLC_LCD_H
#define OPENPLC_LCD_H

#include <Arduino.h>
#include "OpenPLC_SSD2119.h"

#ifndef OPENPLC_LCD_CS
#if defined(PD14)
#define OPENPLC_LCD_CS PD14
#else
#define OPENPLC_LCD_CS 10
#endif
#endif

#ifndef OPENPLC_LCD_DC
#if defined(PD15)
#define OPENPLC_LCD_DC PD15
#else
#define OPENPLC_LCD_DC 9
#endif
#endif

#ifndef OPENPLC_LCD_RST
#if defined(PF3)
#define OPENPLC_LCD_RST PF3
#else
#define OPENPLC_LCD_RST 8
#endif
#endif

#ifndef OPENPLC_LCD_REFRESH_MS
#define OPENPLC_LCD_REFRESH_MS 100UL
#endif

#ifndef OPENPLC_LCD_TEXT_MAX_LEN
#define OPENPLC_LCD_TEXT_MAX_LEN 64U
#endif

#ifndef OPENPLC_LCD_TEXT_QUEUE_SIZE
#define OPENPLC_LCD_TEXT_QUEUE_SIZE 4U
#endif

#ifndef OPENPLC_LCD_FRAMEBUFFER_ENABLED
#if defined(ARDUINO_NUCLEO_H563ZI)
#define OPENPLC_LCD_FRAMEBUFFER_ENABLED 1
#else
#define OPENPLC_LCD_FRAMEBUFFER_ENABLED 0
#endif
#endif

#ifndef OPENPLC_LCD_TASK_BUDGET_US
#define OPENPLC_LCD_TASK_BUDGET_US 1000UL
#endif

#ifndef OPENPLC_LCD_ROWS_PER_TASK
#define OPENPLC_LCD_ROWS_PER_TASK 2U
#endif

extern OpenPLC_SSD2119 OpenPLC_LCD_Display;

#ifdef __cplusplus
extern "C" {
#endif

void OpenPLC_LCD_Init(void);
void OpenPLC_LCD_Task(void);
void OpenPLC_LCD_RequestRefresh(void);

uint8_t OpenPLC_LCD_Text(
  uint16_t x,
  uint16_t y,
  const char *text,
  uint8_t fontSize,
  uint16_t fontColor,
  uint16_t bgColor,
  uint8_t clearBg
);

uint8_t OpenPLC_LCD_Page4(
  uint16_t x,
  uint16_t y1,
  uint16_t y2,
  uint16_t y3,
  uint16_t y4,
  const char *line1,
  const char *line2,
  const char *line3,
  const char *line4,
  uint8_t fontSize,
  uint16_t fontColor,
  uint16_t bgColor,
  uint8_t clearBg
);

uint8_t OpenPLC_LCD_FrameBegin(
  uint16_t backgroundColor,
  uint8_t clearFrame
);

uint8_t OpenPLC_LCD_FrameFillRect(
  uint16_t x,
  uint16_t y,
  uint16_t width,
  uint16_t height,
  uint16_t color
);

uint8_t OpenPLC_LCD_FrameText(
  uint16_t x,
  uint16_t y,
  const char *text,
  uint8_t fontSize,
  uint16_t fontColor,
  uint16_t backgroundColor,
  uint8_t clearTextBackground
);

uint8_t OpenPLC_LCD_FramePresent(void);
uint8_t OpenPLC_LCD_FrameAbort(void);
uint8_t OpenPLC_LCD_FrameBusy(void);
uint8_t OpenPLC_LCD_FrameReady(void);
uint8_t OpenPLC_LCD_FrameEnabled(void);

#ifdef __cplusplus
}
#endif

#endif /* OPENPLC_LCD_H */