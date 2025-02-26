#include <inc.h>

//void Read_Structure_From_Flash(uint32_t address, void *data, size_t size) {
//    // Copier les données de la Flash vers la structure
//    memcpy(data, (void *)address, size);
//}


void SendCharFTDI(char Carac) {
	(void)HAL_UART_Transmit(&hUART2, (uint8_t *)&Carac, 1, HAL_MAX_DELAY);
}


void SendStringFTDI(char *Chaine) {
	(void)HAL_UART_Transmit(&hUART2, (uint8_t *)Chaine, strlen(Chaine), HAL_MAX_DELAY);
}
/**
 * @brief Initialise la broche PA0 (par exemple) en mode EXTI (front montant).
 * @return Aucun.
 */
void MX_GPIO_EXTI0_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Activer l'horloge du GPIOA (si PA0) */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* Config de la pin PA0 en mode interrupt rising */
  GPIO_InitStruct.Pin  = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Configurer la priorité EXTI0 et activer l’IRQ */
  HAL_NVIC_SetPriority(EXTI0_IRQn, 2U, 0U);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}


/**
 * @brief Initialise TIM2 pour un tick de 1 µs.
 *
 * Cette fonction configure TIM2 avec un préscaler calculé à partir de SystemCoreClock.
 * La fréquence d'horloge du compteur TIM2 est ainsi réglée sur 1 MHz (1 µs par tick).
 *
 * @note Il est impératif que SystemCoreClock contienne la fréquence réelle du processeur.
 */
void MX_TIM2_Init_1us(void)
{
    uint32_t uwPrescalerValue = 0U;

    /* Activer l'horloge pour TIM2 */
    __HAL_RCC_TIM2_CLK_ENABLE();

    /* Calculer le préscaler pour obtenir une fréquence de 1 MHz.
     * SystemCoreClock correspond à la fréquence du processeur.
     * On souhaite que : f_timer = SystemCoreClock / (uwPrescalerValue + 1) = 1 MHz.
     * Donc, uwPrescalerValue = (SystemCoreClock / 1e6) - 1.
     */
    uwPrescalerValue = (uint32_t)(SystemCoreClock / 1000000U) - 1U;

    /* Configuration du timer TIM2 */
    htim2.Instance               = TIM2;
    htim2.Init.Prescaler         = uwPrescalerValue;
    htim2.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim2.Init.Period            = 0xFFFFFFFFUL;  /* Compteur 32 bits */
    htim2.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    /* Initialisation du timer */
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        /* Gestion de l'erreur */
        Error_Handler();
    }

    /* Lancer le timer en mode base */
    if (HAL_TIM_Base_Start(&htim2) != HAL_OK)
    {
        /* Gestion de l'erreur */
        Error_Handler();
    }
}

///**
// * @brief  Initialise TIM2 pour qu'il incrémente à 1 MHz (1 µs par tick).
// *         APB1 = 16 MHz => Prescaler = 15 => 1 MHz.
// * @return Aucun.
// */
//void MX_TIM2_Init_1us(void)
//{
//  /* Activer l'horloge pour TIM2 */
//  __HAL_RCC_TIM2_CLK_ENABLE();

//  /* Configurer le handle htim2 */
//  htim2.Instance               = TIM2;
//  htim2.Init.Prescaler         = (uint32_t)(2UL - 1UL); /* PSC = 15 pour diviser 16 MHz par 16 => 1 MHz */
//  htim2.Init.CounterMode       = TIM_COUNTERMODE_UP;
//  htim2.Init.Period            = 0xFFFFFFFFUL;           /* 32 bits entier */
//  htim2.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
//  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

//  /* Initialisation du timer */
//  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
//  {
//    /* Gérer l'erreur */
//    Error_Handler();
//  }

//  /* Lancer le timer en mode libre (base) */
//  if (HAL_TIM_Base_Start(&htim2) != HAL_OK)
//  {
//    /* Gérer l'erreur */
//    Error_Handler();
//  }
//}


// Initialisation de Timer2 en mode Input Capture
void MX_TIM2_Init(void) {
 /* Activer l’horloge de TIM2 */
  __HAL_RCC_TIM2_CLK_ENABLE();

  /* Préparer le handle */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler         = (uint32_t)(16UL - 1UL); /* APB1=16 MHz => division par 16 => 1 MHz => 1 tick/us */
  htim2.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim2.Init.Period            = 0xFFFFFFFFUL;           /* 32 bits plein */
  htim2.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  /* Initialisation */
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    /* Gérer l'erreur */
    Error_Handler();
  }

  /* Lancer le timer en mode "base" (libre) */
  if (HAL_TIM_Base_Start(&htim2) != HAL_OK)
  {
    /* Gérer l'erreur */
    Error_Handler();
  }
}

void MX_TIM2_IC_CH1_Init(void)
{
  TIM_IC_InitTypeDef sConfigIC;

  /* TIM2 est déjà initialisé (MX_TIM2_Init_1us).
   * On le re-configure en mode Input Capture sur la Channel 1.
   */

  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    /* Erreur */
    Error_Handler();
  }

  /* Configuration de la Channel 1 en capture sur front montant */
  (void)memset((void*)&sConfigIC, 0, sizeof(sConfigIC));
  sConfigIC.ICPolarity  = TIM_ICPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter    = 0U;

  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    /* Erreur */
    Error_Handler();
  }

  /* Démarrer la capture sur CH1 (on peut, ou non, activer l’interruption timer) */
  if (HAL_TIM_IC_Start(&htim2, TIM_CHANNEL_1) != HAL_OK)
  {
    /* Erreur */
    Error_Handler();
  }
}


