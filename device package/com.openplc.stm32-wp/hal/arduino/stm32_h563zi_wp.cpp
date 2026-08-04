#include <stdlib.h>

extern "C" {
#include "openplc.h"
}

#include "Arduino.h"
#include "defines.h"

/*
 * OpenPLC HAL for NUCLEO-H563ZI WP.
 *
 * Board notes:
 * - Online debug:
 *   USART3 through ST-LINK VCP. In STM32duino this is the default Serial
 *   instance with MCU TX on PD8 and MCU RX on PD9.
 * - RS485 field bus:
 *   USART2 uses Serial2 on PD5 TX and PD6 RX. PD4 is reserved for RS485
 *   DE/RE control when a separate user/device Modbus channel is added.
 * - LCD SSD2119:
 *   SPI1 uses SCK PA5 and MOSI PB5. PG9 can be MISO but SSD2119 does not
 *   need it. PD14 is LCD_CS, PD15 is LCD_DC, and PF3 is LCD_RST.
 * - AD7792:
 *   SPI2 uses SCK PA12, MOSI PB15, MISO PB14, with CS pins PA11, PB12,
 *   PA3, and PB4 for four chips.
 * - Analog inputs are single-ended Arduino analogRead() channels.
 *
 * The actual OpenPLC pin list is generated from the editor pin mapping table
 * into PINMASK_* macros in defines.h.
 */

uint8_t pinMask_DIN[] = {PINMASK_DIN};
uint8_t pinMask_AIN[] = {PINMASK_AIN};
uint8_t pinMask_DOUT[] = {PINMASK_DOUT};
uint8_t pinMask_AOUT[] = {PINMASK_AOUT};

void hardwareInit()
{
    analogReadResolution(12);
    analogWriteResolution(12);

    for (int i = 0; i < NUM_DISCRETE_INPUT; i++)
    {
        pinMode(pinMask_DIN[i], INPUT);
    }

    for (int i = 0; i < NUM_ANALOG_INPUT; i++)
    {
        pinMode(pinMask_AIN[i], INPUT_ANALOG);
    }

    for (int i = 0; i < NUM_DISCRETE_OUTPUT; i++)
    {
        pinMode(pinMask_DOUT[i], OUTPUT);
    }

    for (int i = 0; i < NUM_ANALOG_OUTPUT; i++)
    {
        pinMode(pinMask_AOUT[i], OUTPUT);
    }

#ifdef WP_RS485_DE
    pinMode(WP_RS485_DE, OUTPUT);
    digitalWrite(WP_RS485_DE, LOW);
#endif

#ifdef WP_LCD_CS
    pinMode(WP_LCD_CS, OUTPUT);
    digitalWrite(WP_LCD_CS, HIGH);
#endif
#ifdef WP_LCD_DC
    pinMode(WP_LCD_DC, OUTPUT);
    digitalWrite(WP_LCD_DC, LOW);
#endif
#ifdef WP_LCD_RST
    pinMode(WP_LCD_RST, OUTPUT);
    digitalWrite(WP_LCD_RST, HIGH);
#endif

#ifdef WP_AD7792_CS0
    pinMode(WP_AD7792_CS0, OUTPUT);
    digitalWrite(WP_AD7792_CS0, HIGH);
#endif
#ifdef WP_AD7792_CS1
    pinMode(WP_AD7792_CS1, OUTPUT);
    digitalWrite(WP_AD7792_CS1, HIGH);
#endif
#ifdef WP_AD7792_CS2
    pinMode(WP_AD7792_CS2, OUTPUT);
    digitalWrite(WP_AD7792_CS2, HIGH);
#endif
#ifdef WP_AD7792_CS3
    pinMode(WP_AD7792_CS3, OUTPUT);
    digitalWrite(WP_AD7792_CS3, HIGH);
#endif
}

void updateInputBuffers()
{
    for (int i = 0; i < NUM_DISCRETE_INPUT; i++)
    {
        if (bool_input[i / 8][i % 8] != NULL)
        {
            *bool_input[i / 8][i % 8] = digitalRead(pinMask_DIN[i]);
        }
    }

    for (int i = 0; i < NUM_ANALOG_INPUT; i++)
    {
        if (int_input[i] != NULL)
        {
            *int_input[i] = analogRead(pinMask_AIN[i]) * 16;
        }
    }
}

void updateOutputBuffers()
{
    for (int i = 0; i < NUM_DISCRETE_OUTPUT; i++)
    {
        if (bool_output[i / 8][i % 8] != NULL)
        {
            digitalWrite(pinMask_DOUT[i], *bool_output[i / 8][i % 8]);
        }
    }

    for (int i = 0; i < NUM_ANALOG_OUTPUT; i++)
    {
        if (int_output[i] != NULL)
        {
            analogWrite(pinMask_AOUT[i], *int_output[i] / 16);
        }
    }
}
