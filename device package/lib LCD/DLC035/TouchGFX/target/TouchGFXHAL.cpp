/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : TouchGFXHAL.cpp
  ******************************************************************************
  * This file was created by TouchGFX Generator 4.22.0. This file is only
  * generated once! Delete this file from your project and re-generate code
  * using STM32CubeMX or change this file manually to update it.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#include <TouchGFXHAL.hpp>

/* USER CODE BEGIN TouchGFXHAL.cpp */
#include <touchgfx/hal/GPIO.hpp>
#include <touchgfx/hal/OSWrappers.hpp>
#include <KeySampler.hpp>
#include "main.h"
#include "FreeRTOS.h"
#include "ssd2119.h"

using namespace touchgfx;

static uint16_t* TFTframebuffer = 0;

extern "C" SPI_HandleTypeDef hspi1;
extern "C" void touchgfxSignalVSync(void);

namespace touchgfx
{
void startNewTransfer();
}

KeySampler btnctrl;

void initLCD(void);

#ifndef LCD_COLOR_TEST_HOLD_MS
#define LCD_COLOR_TEST_HOLD_MS 5000U
#endif

static void showLCDColorTest()
{
#if LCD_COLOR_TEST_HOLD_MS > 0
    // Top: solid-fill path. Bottom: the bitmap path used by TouchGFX.
    static const uint16_t colors[] = {
        0xFFFFU, 0xFFE0U, 0x07FFU, 0x07E0U,
        0xF81FU, 0xF800U, 0x001FU, 0x0000U
    };
    uint16_t line[SSD2119_WIDTH];
    const uint16_t barWidth = SSD2119_WIDTH / 8U;
    const uint16_t halfHeight = SSD2119_HEIGHT / 2U;

    for (uint16_t bar = 0U; bar < 8U; ++bar)
    {
        SSD2119_FillRect(bar * barWidth, 0U, barWidth, halfHeight, colors[bar]);
    }
    for (uint16_t x = 0U; x < SSD2119_WIDTH; ++x)
    {
        line[x] = colors[x / barWidth];
    }
    for (uint16_t y = halfHeight; y < SSD2119_HEIGHT; ++y)
    {
        if (SSD2119_WriteBitmap(0U, y, SSD2119_WIDTH, 1U, line) != HAL_OK)
        {
            Error_Handler();
        }
    }
    HAL_Delay(LCD_COLOR_TEST_HOLD_MS);
    SSD2119_FillScreen(0x0000U);
#endif
}

void TouchGFXHAL::initialize()
{
    initLCD();
    showLCDColorTest();
    setButtonController(&btnctrl);

    instrumentation.init();
    setMCUInstrumentation(&instrumentation);
    enableMCULoadCalculation(true);

    TouchGFXGeneratedHAL::initialize();
}

uint16_t* TouchGFXHAL::getTFTFrameBuffer() const
{
    return TFTframebuffer;
}

void TouchGFXHAL::setTFTFrameBuffer(uint16_t* address)
{
    TFTframebuffer = address;
    TouchGFXGeneratedHAL::setTFTFrameBuffer(address);
}

void TouchGFXHAL::flushFrameBuffer(const touchgfx::Rect& rect)
{
    TouchGFXGeneratedHAL::flushFrameBuffer(rect);
}

bool TouchGFXHAL::blockCopy(void* RESTRICT dest, const void* RESTRICT src, uint32_t numBytes)
{
    return TouchGFXGeneratedHAL::blockCopy(dest, src, numBytes);
}

void TouchGFXHAL::configureInterrupts()
{
    TouchGFXGeneratedHAL::configureInterrupts();
}

void TouchGFXHAL::enableInterrupts()
{
    TouchGFXGeneratedHAL::enableInterrupts();
}

void TouchGFXHAL::disableInterrupts()
{
    TouchGFXGeneratedHAL::disableInterrupts();
}

void TouchGFXHAL::enableLCDControllerInterrupt()
{
    /* SSD2119 is driven over SPI and no TE/VSYNC pin is connected. */
}

bool TouchGFXHAL::beginFrame()
{
    return TouchGFXGeneratedHAL::beginFrame();
}

void TouchGFXHAL::endFrame()
{
    TouchGFXGeneratedHAL::endFrame();
}

void initLCD(void)
{
    if (SSD2119_Init(&hspi1) != HAL_OK)
    {
        __disable_irq();
        while (1)
        {
        }
    }
}

extern "C" int touchgfxDisplayDriverTransmitActive(void)
{
    return SSD2119_IsTransferBusy();
}

extern "C" void touchgfxDisplayDriverTransmitBlock(const uint8_t* pixels, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    (void)SSD2119_WriteBitmap(x, y, w, h, reinterpret_cast<const uint16_t*>(pixels));
    touchgfx::startNewTransfer();
}

extern "C"
{
    portBASE_TYPE IdleTaskHook(void* p)
    {
        if ((int)p) // idle task scheduled out
        {
            touchgfx::HAL::getInstance()->setMCUActive(true);
        }
        else // idle task scheduled in
        {
            touchgfx::HAL::getInstance()->setMCUActive(false);
        }
        return pdTRUE;
    }
}

/* USER CODE END TouchGFXHAL.cpp */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
