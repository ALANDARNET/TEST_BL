/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32g4xx_hal.h"

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
#define ANEMO_IRQ_Pin GPIO_PIN_0
#define ANEMO_IRQ_GPIO_Port GPIOA
#define ANEMO_IRQ_EXTI_IRQn EXTI0_IRQn
#define PLUVIO_IRQ_Pin GPIO_PIN_4
#define PLUVIO_IRQ_GPIO_Port GPIOA
#define PLUVIO_IRQ_EXTI_IRQn EXTI4_IRQn

/* USER CODE BEGIN Private defines */
/* Definition for TIMx clock resources */
#define TIMx                             TIM4
#define TIMx_CLK_ENABLE                  __HAL_RCC_TIM4_CLK_ENABLE
#define TIMx_FORCE_RESET()               __HAL_RCC_TIM4_FORCE_RESET()
#define TIMx_RELEASE_RESET()             __HAL_RCC_TIM4_RELEASE_RESET()

/* Definition for TIMx's NVIC */
#define TIMx_IRQn                        TIM4_IRQn
#define TIMx_IRQHandler                  TIM4_IRQHandler
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
