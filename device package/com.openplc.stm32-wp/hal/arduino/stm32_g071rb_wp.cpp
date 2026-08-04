#include <stdlib.h>

extern "C" {
#include "openplc.h"
}

#include "Arduino.h"
#include "defines.h"

/*
 * OpenPLC HAL for STM32G071RB WP.
 *
 * Board notes:
 * - Analog inputs:
 *   %IW0..%IW4 -> PA0, PA1, PA4, PA6, PA7
 * - Digital inputs:
 *   %IX0.0..%IX0.6 -> PB8, PB2, PB3, PB4, PB5, PB9, PB10
 * - Digital outputs:
 *   %QX0.0..%QX0.6 -> PB12, PB13, PB14, PB15, PC13, PC14, PC15
 * - USART2:
 *   PA2/PA3 connected to ST-LINK RX/TX. Keep it for Serial debug/logging.
 * - USART1:
 *   PC4/PC5 intended for Modbus RTU device communication.
 * - I2C1:
 *   SDA PA10, SCL PA9.
 * - SPI1:
 *   MOSI PA12, MISO PA11, SCK PD8, CS PD9.
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
        pinMode(pinMask_AIN[i], INPUT);
    }

    for (int i = 0; i < NUM_DISCRETE_OUTPUT; i++)
    {
        pinMode(pinMask_DOUT[i], OUTPUT);
    }

    for (int i = 0; i < NUM_ANALOG_OUTPUT; i++)
    {
        pinMode(pinMask_AOUT[i], OUTPUT);
    }
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
