#include <SPI.h>
#include <OpenPLC_LCD.h>

void setup()
{
  OpenPLC_LCD_Init();
}

void loop()
{
  OpenPLC_LCD_Task();
}
