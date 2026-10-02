#include <SPI.h>
#include <OpenPLC_SSD2119.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSerif18pt7b.h>
#include <Fonts/FreeSerif24pt7b.h>

#ifndef LCD_CS
#define LCD_CS  PD14
#endif
#ifndef LCD_DC
#define LCD_DC  PD15
#endif
#ifndef LCD_RST
#define LCD_RST PF3
#endif

OpenPLC_SSD2119 lcd(LCD_CS, LCD_DC, LCD_RST);

void setup()
{
#if defined(ARDUINO_ARCH_STM32)
  SPI.setMOSI(PB5);
  SPI.setSCLK(PA5);
#endif
  SPI.begin();

  lcd.begin();
  lcd.fillScreen(SSD2119_BLACK);

  lcd.setFont(&FreeSerif9pt7b);
  lcd.setTextColor(SSD2119_WHITE, SSD2119_BLACK);
  lcd.setCursor(12, 28);
  lcd.print("Test library on Arduino IDE");

  lcd.setFont(&FreeSerif12pt7b);
  lcd.setTextColor(SSD2119_YELLOW, SSD2119_BLACK);
  lcd.setCursor(12, 64);
  lcd.print("FreeSerif 12pt");

  lcd.setFont(&FreeSerif18pt7b);
  lcd.setTextColor(SSD2119_CYAN, SSD2119_BLACK);
  lcd.setCursor(12, 116);
  lcd.print("FreeSerif 18pt");

  lcd.setFont(&FreeSerif24pt7b);
  lcd.setTextColor(SSD2119_ORANGE, SSD2119_BLACK);
  lcd.setCursor(12, 190);
  lcd.print("FreeSerif 24pt");
}

void loop()
{
}
