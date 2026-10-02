#include "KeySampler.hpp"
#include "string.h"
#include "main.h"
#include "TouchGFXHAL.hpp"

using namespace touchgfx;

/**
 * @brief Buttons Types Definition
 */
typedef enum
{
    BUTTON_NONE  = 0xFF,
    JOY_SEL   = '5',
    JOY_LEFT  = '4',
    JOY_RIGHT = '6',
    JOY_DOWN  = '2',
    JOY_UP    = '8',
    BUTTON_USER = '0'
} ButtonState_TypeDef;

uint8_t lastKey = BUTTON_NONE;
uint32_t lastTick = 0;

void KeySampler::init()
{
}

bool KeySampler::sample(uint8_t& key)
{
    bool buttonPressed = true;

    if (HAL_GPIO_ReadPin(KEY_CENTER_GPIO_Port, KEY_CENTER_Pin) == GPIO_PIN_RESET)
    {
        key = JOY_SEL;
    }
    else if (HAL_GPIO_ReadPin(KEY_LEFT_GPIO_Port, KEY_LEFT_Pin) == GPIO_PIN_RESET)
    {
        key = HAL::getInstance()->getDisplayOrientation() == ORIENTATION_LANDSCAPE ? JOY_UP : JOY_LEFT;
    }
    else if (HAL_GPIO_ReadPin(KEY_RIGHT_GPIO_Port, KEY_RIGHT_Pin) == GPIO_PIN_RESET)
    {
        key = HAL::getInstance()->getDisplayOrientation() == ORIENTATION_LANDSCAPE ? JOY_DOWN : JOY_RIGHT;
    }
    else if (HAL_GPIO_ReadPin(KEY_DOWN_GPIO_Port, KEY_DOWN_Pin) == GPIO_PIN_RESET)
    {
        key = HAL::getInstance()->getDisplayOrientation() == ORIENTATION_LANDSCAPE ? JOY_LEFT : JOY_DOWN;
    }
    else if (HAL_GPIO_ReadPin(KEY_UP_GPIO_Port, KEY_UP_Pin) == GPIO_PIN_RESET)
    {
        key = HAL::getInstance()->getDisplayOrientation() == ORIENTATION_LANDSCAPE ? JOY_RIGHT : JOY_UP;
    }
    else if (HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin) == GPIO_PIN_SET)
    {
        key = BUTTON_USER;
    }
    else
    {
        key = BUTTON_NONE;
        buttonPressed = false;
    }

    /* Account for continuous press */
    if (key == lastKey && ((HAL_GetTick() - lastTick) < 200))
    {
        buttonPressed = false;
    }
    if (buttonPressed)
    {
        lastKey = key;
        lastTick = HAL_GetTick();
    }

    return buttonPressed;
}
