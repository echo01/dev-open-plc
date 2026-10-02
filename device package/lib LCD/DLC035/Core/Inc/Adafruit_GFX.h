#ifndef __ADAFRUIT_GFX_COMPAT_H__
#define __ADAFRUIT_GFX_COMPAT_H__

#include <stdint.h>

#ifndef PROGMEM
#define PROGMEM
#endif

#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const uint8_t *)(addr))
#endif

#ifndef pgm_read_word
#define pgm_read_word(addr) (*(const uint16_t *)(addr))
#endif

#ifndef pgm_read_pointer
#define pgm_read_pointer(addr) (*(const void * const *)(addr))
#endif

typedef struct {
  uint16_t bitmapOffset;
  uint8_t width;
  uint8_t height;
  uint8_t xAdvance;
  int8_t xOffset;
  int8_t yOffset;
} GFXglyph;

typedef struct {
  uint8_t *bitmap;
  GFXglyph *glyph;
  uint16_t first;
  uint16_t last;
  uint8_t yAdvance;
} GFXfont;

#endif /* __ADAFRUIT_GFX_COMPAT_H__ */
