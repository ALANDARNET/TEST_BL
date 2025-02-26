#include "inc.h"

/* Variables pour stocker les résultats */
uint16_t adc1_value = 0;
uint16_t adc2_value = 0;

/*=========================================================================*/
/*                     INITIALISATION ADC1 & ADC2                          */
/*=========================================================================*/

/**
 * @brief Initialise l’ADC1 pour la lecture de l’entrée analogique sur PF0.
 *
 * Configure l’ADC1 en mode conversion unique, sans DMA ni interruption.
 *
 * @note Le canal utilisé ici est ADC_CHANNEL_10 pour PF0 (ADC1_IN10).
 */
void MX_ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }
    
    /* Configuration du canal de l’ADC1 : ADC_CHANNEL_10 pour PF0 (ADC1_IN10) */
    sConfig.Channel = ADC_CHANNEL_10;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief Initialise l’ADC2 pour la lecture de l’entrée analogique sur PF1.
 *
 * Configure l’ADC2 en mode conversion unique, sans DMA ni interruption.
 *
 * @note Le canal utilisé ici est ADC_CHANNEL_10 pour PF1 (ADC2_IN10).
 */
void MX_ADC2_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    
    hadc2.Instance = ADC2;
    hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
    hadc2.Init.Resolution = ADC_RESOLUTION_12B;
    hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc2.Init.LowPowerAutoWait = DISABLE;
    hadc2.Init.ContinuousConvMode = DISABLE;
    hadc2.Init.NbrOfConversion = 1;
    hadc2.Init.DiscontinuousConvMode = DISABLE;
    hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    if (HAL_ADC_Init(&hadc2) != HAL_OK)
    {
        Error_Handler();
    }
    
    /* Configuration du canal de l’ADC2 : ADC_CHANNEL_10 pour PF1 (ADC2_IN10) */
    sConfig.Channel = ADC_CHANNEL_10;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
    if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
}


/*=========================================================================*/
/*                      INITIALISATION MSP ADC                             */
/*=========================================================================*/

/**
  * @brief  Initialisation des GPIO et horloges pour ADC1 et ADC2.
  */
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (hadc->Instance == ADC1 || hadc->Instance == ADC2)
    {
        /* Activer l'horloge des GPIO et ADC */
        __HAL_RCC_GPIOF_CLK_ENABLE();
        __HAL_RCC_ADC12_CLK_ENABLE();

        /* Configuration de PA10 en mode analogique (ADC1 IN10 et ADC2 IN10) */
        GPIO_InitStruct.Pin = GPIO_PIN_0;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
        /* Configuration de PA10 en mode analogique (ADC1 IN10 et ADC2 IN10) */
        GPIO_InitStruct.Pin = GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
    }
}

/*=========================================================================*/
/*                  LECTURE MANUELLE DES ADC EN MODE BLOQUANT              */
/*=========================================================================*/

/**
  * @brief  Lit ADC1 IN10 et ADC2 IN10 en mode bloquant.
  */
void Read_ADC_Values(void) {
	/* --- ADC1 sur PF0 --- */
	if (HAL_ADC_Start(&hadc1) != HAL_OK)
	{
		Error_Handler();
	}
	/* Attente de la fin de conversion (timeout de 10ms) */
	if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
	{
		adc1_value = HAL_ADC_GetValue(&hadc1);
	}
	HAL_ADC_Stop(&hadc1);
	
	/* --- ADC2 sur PF1 --- */
	if (HAL_ADC_Start(&hadc2) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_ADC_PollForConversion(&hadc2, 10) == HAL_OK)
	{
		adc2_value = HAL_ADC_GetValue(&hadc2);
	}
    HAL_ADC_Stop(&hadc2);
	
	v_ADC1_IN10 = (float)adc1_value * (3.3f/4096.0f) * 1.511f;
	v_ADC2_IN10 = (float)adc2_value * (3.3f/4096.0f) * 1.511f;
	
}

///*=========================================================================*/
///*                     FONCTION PRINCIPALE                                  */
///*=========================================================================*/

//int main(void)
//{
//    HAL_Init();
//    SystemClock_Config();

//    MX_ADC1_Init();
//    MX_ADC2_Init();
//    MX_ADC_MultiMode_Init();

//    while (1)
//    {
//        Read_ADC_Values();

//        /* adc1_value contient ADC1 IN10 */
//        /* adc2_value contient ADC2 IN10 */
//    }
//}
