/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h5xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define KEY_LEFT_Pin GPIO_PIN_3
#define KEY_LEFT_GPIO_Port GPIOE
#define BUTTON_Pin GPIO_PIN_13
#define BUTTON_GPIO_Port GPIOC
#define KEY_RIGHT_Pin GPIO_PIN_7
#define KEY_RIGHT_GPIO_Port GPIOF
#define KEY_DOWN_Pin GPIO_PIN_8
#define KEY_DOWN_GPIO_Port GPIOF
#define KEY_UP_Pin GPIO_PIN_9
#define KEY_UP_GPIO_Port GPIOF
#define KEY_CENTER_Pin GPIO_PIN_0
#define KEY_CENTER_GPIO_Port GPIOA
#define LED_RED_Pin GPIO_PIN_2
#define LED_RED_GPIO_Port GPIOG
#define LED_GREEN_Pin GPIO_PIN_7
#define LED_GREEN_GPIO_Port GPIOG
#define RS485_CS_Pin GPIO_PIN_4
#define RS485_CS_GPIO_Port GPIOD
#define VSYNC_FREQ_Pin GPIO_PIN_12
#define VSYNC_FREQ_GPIO_Port GPIOG
#define FRAME_RATE_Pin GPIO_PIN_14
#define FRAME_RATE_GPIO_Port GPIOG
#define LCD_DC_Pin GPIO_PIN_15
#define LCD_DC_GPIO_Port GPIOG
#define LCD_CS_Pin GPIO_PIN_6
#define LCD_CS_GPIO_Port GPIOB
#define LCD_RST_Pin GPIO_PIN_7
#define LCD_RST_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
