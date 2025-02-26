/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    lptim.c
  * @brief   This file provides code for the configuration
  *          of the LPTIM instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "inc.h"

/* LPTIM1 init function */
void MX_LPTIM1_Init(void)
{
	__HAL_RCC_LSI_ENABLE();
	  /* Attendre la stabilité du LSI */
	while (READ_BIT(RCC->CSR, RCC_CSR_LSIRDY) == 0UL) {
		/* On attend que le bit LSIRDY (LSI Ready) soit positionné */
	}
	MODIFY_REG(RCC->CCIPR, RCC_CCIPR_LPTIM1SEL, (1UL << RCC_CCIPR_LPTIM1SEL_Pos));
	
    __HAL_RCC_LPTIM1_CLK_ENABLE();
	
	hlptim1.Instance             = LPTIM1;
	hlptim1.Init.Clock.Source    = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
	hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
//	hlptim1.Init.Trigger.Source  = LPTIM_TRIGSOURCE_SOFTWARE;
//	hlptim1.Init.OutputPolarity  = LPTIM_OUTPUTPOLARITY_HIGH;
	hlptim1.Init.UpdateMode      = LPTIM_UPDATE_IMMEDIATE;
	hlptim1.Init.CounterSource   = LPTIM_COUNTERSOURCE_INTERNAL;
	hlptim1.Init.Input1Source    = LPTIM_INPUT1SOURCE_GPIO;
	hlptim1.Init.Input2Source    = LPTIM_INPUT2SOURCE_GPIO;
    if (HAL_LPTIM_Init(&hlptim1) != HAL_OK) {
        Error_Handler();
    }

	/* 5) Configurer et démarrer le LPTIM en mode Counter + interruption -------- */
	/* Supposons un LSI à 32 kHz : pour ~1 seconde, on choisit un ARR à 31999 */
	const uint32_t periodTicks = 31999UL;	

	/* -- Activer l'interruption overflow (ARRM) -- */
	__HAL_LPTIM_ENABLE_IT(&hlptim1, LPTIM_IT_ARRM);

	/* -- Configurer la priorité dans le NVIC -- */
	HAL_NVIC_SetPriority(LPTIM1_IRQn, 0U, 0U);
	HAL_NVIC_EnableIRQ(LPTIM1_IRQn);

	/* -- Démarrer le compteur en mode IT, la valeur passée est le "Auto-Reload" -- */
	if (HAL_LPTIM_Counter_Start_IT(&hlptim1, periodTicks) != HAL_OK)
	{
		/* Gérer l'erreur */
		Error_Handler();
	}



//	
//	/* 5) Configurer l'Auto-Reload pour 1 seconde ------------------------------ */
//	/* Si LSI ~ 32 kHz, nous souhaitons 32000 ticks environ.
//	* On positionne l'ARR à 31999 pour avoir 32000 incréments (de 0 à 31999).
//	*/
//	if (HAL_LPTIM_SetAutoReload(&hlptim1, (uint32_t)31999) != HAL_OK)
//	{
//	/* Gérer l'erreur */
//	Error_Handler();
//	}	
//	__HAL_LPTIM_ENABLE_IT(&hlptim1, LPTIM_IT_ARRM);  
//	HAL_NVIC_SetPriority(LPTIM1_IRQn, 1, 0);
//	HAL_NVIC_EnableIRQ(LPTIM1_IRQn);
//	/* 7) Démarrer le timer en mode Continu (Free running) ---------------------- */
//	if (HAL_LPTIM_TimeOut_Start_IT(&hlptim1, (uint32_t)31999, (uint32_t)31999) != HAL_OK)
//	{
//	/* Gérer l'erreur */
//	Error_Handler();
//	}  
}

void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef* lptimHandle) {
  if(lptimHandle->Instance==LPTIM1)
  {
  }
}

void HAL_LPTIM_MspDeInit(LPTIM_HandleTypeDef* lptimHandle)
{

  if(lptimHandle->Instance==LPTIM1)
  {
  /* USER CODE BEGIN LPTIM1_MspDeInit 0 */

  /* USER CODE END LPTIM1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_LPTIM1_CLK_DISABLE();

    /* LPTIM1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(LPTIM1_IRQn);
  /* USER CODE BEGIN LPTIM1_MspDeInit 1 */

  /* USER CODE END LPTIM1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
