#include "OpenPLC_LCD.h"
#include <SPI.h>
#include <string.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSerif18pt7b.h>
#include <Fonts/FreeSerif24pt7b.h>

enum OpenPLC_LCD_CommandType : uint8_t {
  OPENPLC_LCD_COMMAND_TEXT = 0U,
  OPENPLC_LCD_COMMAND_PAGE4 = 1U
};

struct OpenPLC_LCD_Command {
  OpenPLC_LCD_CommandType type;
  uint16_t x;
  uint16_t y[4];
  char text[4][OPENPLC_LCD_TEXT_MAX_LEN];
  uint8_t fontSize;
  uint16_t fontColor;
  uint16_t bgColor;
  bool clearBg;
};

OpenPLC_SSD2119 OpenPLC_LCD_Display(OPENPLC_LCD_CS, OPENPLC_LCD_DC, OPENPLC_LCD_RST);

static bool lcdReady = false;
static OpenPLC_LCD_Command textQueue[OPENPLC_LCD_TEXT_QUEUE_SIZE] = {};
static uint8_t textQueueHead = 0U;
static uint8_t textQueueTail = 0U;
static uint8_t textQueueCount = 0U;
static bool lastPageValid = false;
static OpenPLC_LCD_Command lastPage = {};

#if OPENPLC_LCD_FRAMEBUFFER_ENABLED

struct OpenPLC_LCD_DirtyRect {
  uint16_t x1;
  uint16_t y1;
  uint16_t x2;
  uint16_t y2;
  bool valid;
};

class OpenPLC_LCD_FrameCanvas : public Adafruit_GFX {
public:
  explicit OpenPLC_LCD_FrameCanvas(uint16_t *pixels)
    : Adafruit_GFX(SSD2119_WIDTH, SSD2119_HEIGHT), _pixels(pixels)
  {
  }

  void drawPixel(int16_t x, int16_t y, uint16_t color) override
  {
    if (x < 0 || y < 0 ||
        x >= (int16_t)SSD2119_WIDTH || y >= (int16_t)SSD2119_HEIGHT) {
      return;
    }
    _pixels[(uint32_t)y * SSD2119_WIDTH + (uint16_t)x] = color;
  }

private:
  uint16_t *_pixels;
};

alignas(4) static uint16_t lcdFrameBuffer[SSD2119_WIDTH * SSD2119_HEIGHT] = {};
static OpenPLC_LCD_FrameCanvas frameCanvas(lcdFrameBuffer);
static OpenPLC_LCD_DirtyRect frameDirty = {};
static OpenPLC_LCD_DirtyRect activeDirty = {};
static bool frameBuilding = false;
static bool frameReady = false;
static bool frameFlushing = false;
static bool framePanelSynced = false;
static uint16_t frameFlushY = 0U;

static void OpenPLC_LCD_ResetDirty(OpenPLC_LCD_DirtyRect &rect)
{
  rect.x1 = 0U;
  rect.y1 = 0U;
  rect.x2 = 0U;
  rect.y2 = 0U;
  rect.valid = false;
}

static void OpenPLC_LCD_MarkDirty(int16_t x, int16_t y, int16_t w, int16_t h)
{
  if (w <= 0 || h <= 0 ||
      x >= (int16_t)SSD2119_WIDTH || y >= (int16_t)SSD2119_HEIGHT ||
      (x + w) <= 0 || (y + h) <= 0) {
    return;
  }

  int16_t x1 = x < 0 ? 0 : x;
  int16_t y1 = y < 0 ? 0 : y;
  int16_t x2 = (int16_t)(x + w - 1);
  int16_t y2 = (int16_t)(y + h - 1);
  if (x2 >= (int16_t)SSD2119_WIDTH) x2 = (int16_t)SSD2119_WIDTH - 1;
  if (y2 >= (int16_t)SSD2119_HEIGHT) y2 = (int16_t)SSD2119_HEIGHT - 1;

  if (!frameDirty.valid) {
    frameDirty.x1 = (uint16_t)x1;
    frameDirty.y1 = (uint16_t)y1;
    frameDirty.x2 = (uint16_t)x2;
    frameDirty.y2 = (uint16_t)y2;
    frameDirty.valid = true;
    return;
  }

  if ((uint16_t)x1 < frameDirty.x1) frameDirty.x1 = (uint16_t)x1;
  if ((uint16_t)y1 < frameDirty.y1) frameDirty.y1 = (uint16_t)y1;
  if ((uint16_t)x2 > frameDirty.x2) frameDirty.x2 = (uint16_t)x2;
  if ((uint16_t)y2 > frameDirty.y2) frameDirty.y2 = (uint16_t)y2;
}

static void OpenPLC_LCD_BufferFillRect(
  int16_t x,
  int16_t y,
  int16_t w,
  int16_t h,
  uint16_t color
)
{
  if (w <= 0 || h <= 0 ||
      x >= (int16_t)SSD2119_WIDTH || y >= (int16_t)SSD2119_HEIGHT ||
      (x + w) <= 0 || (y + h) <= 0) {
    return;
  }

  int16_t x1 = x < 0 ? 0 : x;
  int16_t y1 = y < 0 ? 0 : y;
  int16_t x2 = (int16_t)(x + w);
  int16_t y2 = (int16_t)(y + h);
  if (x2 > (int16_t)SSD2119_WIDTH) x2 = (int16_t)SSD2119_WIDTH;
  if (y2 > (int16_t)SSD2119_HEIGHT) y2 = (int16_t)SSD2119_HEIGHT;

  for (int16_t row = y1; row < y2; row++) {
    uint16_t *destination = &lcdFrameBuffer[(uint32_t)row * SSD2119_WIDTH + (uint16_t)x1];
    for (int16_t column = x1; column < x2; column++) {
      *destination++ = color;
    }
  }

  OpenPLC_LCD_MarkDirty(x1, y1, (int16_t)(x2 - x1), (int16_t)(y2 - y1));
}

static bool OpenPLC_LCD_FrameStateActive(void)
{
  return frameBuilding || frameReady || frameFlushing;
}

#endif

static const GFXfont *OpenPLC_LCD_SelectFont(uint8_t fontSize)
{
  if (fontSize >= 24U) {
    return &FreeSerif24pt7b;
  }
  if (fontSize >= 18U) {
    return &FreeSerif18pt7b;
  }
  return &FreeSerif12pt7b;
}

static void OpenPLC_LCD_ClearTextBounds(
  const char *text,
  uint16_t x,
  uint16_t y,
  uint16_t bgColor
)
{
  int16_t x1 = 0;
  int16_t y1 = 0;
  uint16_t w = 0U;
  uint16_t h = 0U;
  OpenPLC_LCD_Display.getTextBounds(text, x, y, &x1, &y1, &w, &h);
  if (w == 0U || h == 0U) {
    return;
  }

  const int16_t pad = 3;
  int16_t fillX = x1 - pad;
  int16_t fillY = y1 - pad;
  int16_t fillW = (int16_t)w + (pad * 2);
  int16_t fillH = (int16_t)h + (pad * 2);
  if (fillX < 0) fillX = 0;
  if (fillY < 0) fillY = 0;
  if ((fillX + fillW) > (int16_t)SSD2119_WIDTH) fillW = (int16_t)SSD2119_WIDTH - fillX;
  if ((fillY + fillH) > (int16_t)SSD2119_HEIGHT) fillH = (int16_t)SSD2119_HEIGHT - fillY;
  if (fillW > 0 && fillH > 0) {
    OpenPLC_LCD_Display.fillRect(fillX, fillY, fillW, fillH, bgColor);
  }
}

static void OpenPLC_LCD_InvalidateFramePanel(void)
{
#if OPENPLC_LCD_FRAMEBUFFER_ENABLED
  framePanelSynced = false;
#endif
}

static void OpenPLC_LCD_DrawTextCommand(const OpenPLC_LCD_Command &cmd)
{
  lastPageValid = false;
  OpenPLC_LCD_InvalidateFramePanel();
  OpenPLC_LCD_Display.setFont(OpenPLC_LCD_SelectFont(cmd.fontSize));
  OpenPLC_LCD_Display.setTextColor(cmd.fontColor);

  if (cmd.clearBg) {
    OpenPLC_LCD_ClearTextBounds(cmd.text[0], cmd.x, cmd.y[0], cmd.bgColor);
  }

  OpenPLC_LCD_Display.setCursor(cmd.x, cmd.y[0]);
  OpenPLC_LCD_Display.print(cmd.text[0]);
}

static bool OpenPLC_LCD_PageMatches(
  const OpenPLC_LCD_Command &left,
  const OpenPLC_LCD_Command &right
)
{
  if (left.x != right.x ||
      left.fontSize != right.fontSize ||
      left.fontColor != right.fontColor ||
      left.bgColor != right.bgColor ||
      left.clearBg != right.clearBg) {
    return false;
  }

  for (uint8_t i = 0U; i < 4U; i++) {
    if (left.y[i] != right.y[i] || strcmp(left.text[i], right.text[i]) != 0) {
      return false;
    }
  }
  return true;
}

static bool OpenPLC_LCD_PageLayoutMatches(
  const OpenPLC_LCD_Command &left,
  const OpenPLC_LCD_Command &right
)
{
  if (left.x != right.x ||
      left.fontSize != right.fontSize ||
      left.bgColor != right.bgColor ||
      left.clearBg != right.clearBg) {
    return false;
  }

  for (uint8_t i = 0U; i < 4U; i++) {
    if (left.y[i] != right.y[i]) {
      return false;
    }
  }
  return true;
}

static void OpenPLC_LCD_DrawPage4Command(const OpenPLC_LCD_Command &cmd)
{
  if (lastPageValid && OpenPLC_LCD_PageMatches(cmd, lastPage)) {
    return;
  }

  OpenPLC_LCD_InvalidateFramePanel();
  OpenPLC_LCD_Display.setFont(OpenPLC_LCD_SelectFont(cmd.fontSize));
  OpenPLC_LCD_Display.setTextColor(cmd.fontColor);

  const bool redrawAll = !lastPageValid ||
    !OpenPLC_LCD_PageLayoutMatches(cmd, lastPage) ||
    cmd.fontColor != lastPage.fontColor;

  if (redrawAll && cmd.clearBg) {
    OpenPLC_LCD_Display.fillScreen(cmd.bgColor);
  }

  for (uint8_t i = 0U; i < 4U; i++) {
    const bool lineChanged = redrawAll || strcmp(cmd.text[i], lastPage.text[i]) != 0;
    if (!lineChanged) {
      continue;
    }

    if (!redrawAll && cmd.clearBg) {
      OpenPLC_LCD_ClearTextBounds(lastPage.text[i], lastPage.x, lastPage.y[i], cmd.bgColor);
      OpenPLC_LCD_ClearTextBounds(cmd.text[i], cmd.x, cmd.y[i], cmd.bgColor);
    }

    OpenPLC_LCD_Display.setCursor(cmd.x, cmd.y[i]);
    OpenPLC_LCD_Display.print(cmd.text[i]);
  }

  lastPage = cmd;
  lastPageValid = true;
}

static bool OpenPLC_LCD_DequeueText(OpenPLC_LCD_Command &cmd)
{
  if (textQueueCount == 0U) {
    return false;
  }

  cmd = textQueue[textQueueTail];
  textQueueTail = (uint8_t)((textQueueTail + 1U) % OPENPLC_LCD_TEXT_QUEUE_SIZE);
  textQueueCount--;
  return true;
}

void OpenPLC_LCD_Init(void)
{
#if defined(ARDUINO_ARCH_STM32)
#if defined(PB5)
  SPI.setMOSI(PB5);
#endif
#if defined(PA5)
  SPI.setSCLK(PA5);
#endif
#endif

  SPI.begin();
  lcdReady = OpenPLC_LCD_Display.begin();
  textQueueHead = 0U;
  textQueueTail = 0U;
  textQueueCount = 0U;
  lastPageValid = false;

#if OPENPLC_LCD_FRAMEBUFFER_ENABLED
  for (uint32_t i = 0U; i < ((uint32_t)SSD2119_WIDTH * SSD2119_HEIGHT); i++) {
    lcdFrameBuffer[i] = SSD2119_BLACK;
  }
  OpenPLC_LCD_ResetDirty(frameDirty);
  OpenPLC_LCD_ResetDirty(activeDirty);
  frameBuilding = false;
  frameReady = false;
  frameFlushing = false;
  framePanelSynced = lcdReady;
  frameFlushY = 0U;
#endif
}

void OpenPLC_LCD_RequestRefresh(void)
{
  // Rendering is command driven. This function is kept for API compatibility.
}

uint8_t OpenPLC_LCD_Text(
  uint16_t x,
  uint16_t y,
  const char *text,
  uint8_t fontSize,
  uint16_t fontColor,
  uint16_t bgColor,
  uint8_t clearBg
) {
#if OPENPLC_LCD_FRAMEBUFFER_ENABLED
  if (OpenPLC_LCD_FrameStateActive()) {
    return 0U;
  }
#endif
  if (!lcdReady || textQueueCount >= OPENPLC_LCD_TEXT_QUEUE_SIZE) {
    return 0U;
  }

  OpenPLC_LCD_Command &cmd = textQueue[textQueueHead];
  cmd.type = OPENPLC_LCD_COMMAND_TEXT;
  cmd.x = x;
  cmd.y[0] = y;
  cmd.fontSize = fontSize;
  cmd.fontColor = fontColor;
  cmd.bgColor = bgColor;
  cmd.clearBg = clearBg != 0U;
  strncpy(cmd.text[0], text != nullptr ? text : "", OPENPLC_LCD_TEXT_MAX_LEN - 1U);
  cmd.text[0][OPENPLC_LCD_TEXT_MAX_LEN - 1U] = '\0';

  textQueueHead = (uint8_t)((textQueueHead + 1U) % OPENPLC_LCD_TEXT_QUEUE_SIZE);
  textQueueCount++;
  return 1U;
}

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
) {
#if OPENPLC_LCD_FRAMEBUFFER_ENABLED
  if (OpenPLC_LCD_FrameStateActive()) {
    return 0U;
  }
#endif
  if (!lcdReady || textQueueCount >= OPENPLC_LCD_TEXT_QUEUE_SIZE) {
    return 0U;
  }

  OpenPLC_LCD_Command &cmd = textQueue[textQueueHead];
  const char *lines[4] = { line1, line2, line3, line4 };
  cmd.type = OPENPLC_LCD_COMMAND_PAGE4;
  cmd.x = x;
  cmd.y[0] = y1;
  cmd.y[1] = y2;
  cmd.y[2] = y3;
  cmd.y[3] = y4;
  cmd.fontSize = fontSize;
  cmd.fontColor = fontColor;
  cmd.bgColor = bgColor;
  cmd.clearBg = clearBg != 0U;

  for (uint8_t i = 0U; i < 4U; i++) {
    strncpy(cmd.text[i], lines[i] != nullptr ? lines[i] : "", OPENPLC_LCD_TEXT_MAX_LEN - 1U);
    cmd.text[i][OPENPLC_LCD_TEXT_MAX_LEN - 1U] = '\0';
  }

  textQueueHead = (uint8_t)((textQueueHead + 1U) % OPENPLC_LCD_TEXT_QUEUE_SIZE);
  textQueueCount++;
  return 1U;
}

uint8_t OpenPLC_LCD_FrameBegin(uint16_t backgroundColor, uint8_t clearFrame)
{
#if OPENPLC_LCD_FRAMEBUFFER_ENABLED
  if (!lcdReady || OpenPLC_LCD_FrameStateActive() || textQueueCount > 0U) {
    return 0U;
  }
  if (clearFrame == 0U && !framePanelSynced) {
    return 0U;
  }

  OpenPLC_LCD_ResetDirty(frameDirty);
  frameBuilding = true;
  if (clearFrame != 0U) {
    OpenPLC_LCD_BufferFillRect(0, 0, SSD2119_WIDTH, SSD2119_HEIGHT, backgroundColor);
  }
  return 1U;
#else
  (void)backgroundColor;
  (void)clearFrame;
  return 0U;
#endif
}

uint8_t OpenPLC_LCD_FrameFillRect(
  uint16_t x,
  uint16_t y,
  uint16_t width,
  uint16_t height,
  uint16_t color
)
{
#if OPENPLC_LCD_FRAMEBUFFER_ENABLED
  if (!frameBuilding || width == 0U || height == 0U ||
      x >= SSD2119_WIDTH || y >= SSD2119_HEIGHT) {
    return 0U;
  }

  uint16_t clippedWidth = width;
  uint16_t clippedHeight = height;
  if (((uint32_t)x + clippedWidth) > SSD2119_WIDTH) {
    clippedWidth = (uint16_t)(SSD2119_WIDTH - x);
  }
  if (((uint32_t)y + clippedHeight) > SSD2119_HEIGHT) {
    clippedHeight = (uint16_t)(SSD2119_HEIGHT - y);
  }

  OpenPLC_LCD_BufferFillRect(
    (int16_t)x,
    (int16_t)y,
    (int16_t)clippedWidth,
    (int16_t)clippedHeight,
    color
  );
  return 1U;
#else
  (void)x;
  (void)y;
  (void)width;
  (void)height;
  (void)color;
  return 0U;
#endif
}

uint8_t OpenPLC_LCD_FrameText(
  uint16_t x,
  uint16_t y,
  const char *text,
  uint8_t fontSize,
  uint16_t fontColor,
  uint16_t backgroundColor,
  uint8_t clearTextBackground
)
{
#if OPENPLC_LCD_FRAMEBUFFER_ENABLED
  if (!frameBuilding || x >= SSD2119_WIDTH || y >= SSD2119_HEIGHT) {
    return 0U;
  }

  const char *safeText = text != nullptr ? text : "";
  frameCanvas.setFont(OpenPLC_LCD_SelectFont(fontSize));
  frameCanvas.setTextColor(fontColor);

  int16_t x1 = 0;
  int16_t y1 = 0;
  uint16_t w = 0U;
  uint16_t h = 0U;
  frameCanvas.getTextBounds(safeText, x, y, &x1, &y1, &w, &h);

  if (w > 0U && h > 0U) {
    const int16_t pad = 3;
    if (clearTextBackground != 0U) {
      OpenPLC_LCD_BufferFillRect(
        x1 - pad,
        y1 - pad,
        (int16_t)w + (pad * 2),
        (int16_t)h + (pad * 2),
        backgroundColor
      );
    }

    frameCanvas.setCursor(x, y);
    frameCanvas.print(safeText);
    OpenPLC_LCD_MarkDirty(x1, y1, (int16_t)w, (int16_t)h);
  }
  return 1U;
#else
  (void)x;
  (void)y;
  (void)text;
  (void)fontSize;
  (void)fontColor;
  (void)backgroundColor;
  (void)clearTextBackground;
  return 0U;
#endif
}

uint8_t OpenPLC_LCD_FramePresent(void)
{
#if OPENPLC_LCD_FRAMEBUFFER_ENABLED
  if (!frameBuilding) {
    return 0U;
  }

  frameBuilding = false;
  if (!frameDirty.valid) {
    return 1U;
  }

  frameReady = true;
  return 1U;
#else
  return 0U;
#endif
}

uint8_t OpenPLC_LCD_FrameAbort(void)
{
#if OPENPLC_LCD_FRAMEBUFFER_ENABLED
  if (!frameBuilding) {
    return 0U;
  }

  frameBuilding = false;
  framePanelSynced = false;
  OpenPLC_LCD_ResetDirty(frameDirty);
  return 1U;
#else
  return 0U;
#endif
}

uint8_t OpenPLC_LCD_FrameBusy(void)
{
#if OPENPLC_LCD_FRAMEBUFFER_ENABLED
  return OpenPLC_LCD_FrameStateActive() ? 1U : 0U;
#else
  return 0U;
#endif
}

uint8_t OpenPLC_LCD_FrameReady(void)
{
#if OPENPLC_LCD_FRAMEBUFFER_ENABLED
  return (frameReady || frameFlushing) ? 1U : 0U;
#else
  return 0U;
#endif
}

uint8_t OpenPLC_LCD_FrameEnabled(void)
{
#if OPENPLC_LCD_FRAMEBUFFER_ENABLED
  return 1U;
#else
  return 0U;
#endif
}

void OpenPLC_LCD_Task(void)
{
  if (!lcdReady) {
    return;
  }

#if OPENPLC_LCD_FRAMEBUFFER_ENABLED
  if (frameReady) {
    activeDirty = frameDirty;
    OpenPLC_LCD_ResetDirty(frameDirty);
    frameReady = false;
    frameFlushing = activeDirty.valid;
    frameFlushY = activeDirty.y1;
  }

  if (frameFlushing) {
    uint32_t startedAt = micros();
    uint8_t rowsSent = 0U;
    uint16_t width = (uint16_t)(activeDirty.x2 - activeDirty.x1 + 1U);

    while (frameFlushY <= activeDirty.y2 && rowsSent < OPENPLC_LCD_ROWS_PER_TASK) {
      const uint16_t *rowPixels = &lcdFrameBuffer[
        (uint32_t)frameFlushY * SSD2119_WIDTH + activeDirty.x1
      ];
      OpenPLC_LCD_Display.writeRGB565Row(
        activeDirty.x1,
        frameFlushY,
        rowPixels,
        width
      );
      frameFlushY++;
      rowsSent++;

      if ((micros() - startedAt) >= OPENPLC_LCD_TASK_BUDGET_US) {
        break;
      }
    }

    if (frameFlushY > activeDirty.y2) {
      frameFlushing = false;
      framePanelSynced = true;
      OpenPLC_LCD_ResetDirty(activeDirty);
    }
    return;
  }
#endif

  static uint32_t lastMs = 0U;
  uint32_t now = millis();
  if ((now - lastMs) < OPENPLC_LCD_REFRESH_MS) {
    return;
  }
  lastMs = now;

  OpenPLC_LCD_Command cmd = {};
  if (OpenPLC_LCD_DequeueText(cmd)) {
    if (cmd.type == OPENPLC_LCD_COMMAND_PAGE4) {
      OpenPLC_LCD_DrawPage4Command(cmd);
    } else {
      OpenPLC_LCD_DrawTextCommand(cmd);
    }
  }
}