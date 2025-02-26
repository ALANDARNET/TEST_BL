#include <inc.h>


void SystemClock_Config2MZ(void);
void SystemClock_Config(void);


#define BOOTLOADER_INFO_ADDRESS 0x8011000U
uint8_t UpdateEvent = 0;
bool w_presence_USB = false;
uint32_t unique_id[3];	

//void Read_UniqueID(uint32_t *id) {
//    id[0] = *(uint32_t *)0x1FFF7590;  // Lire le premier mot de l'ID
//    id[1] = *(uint32_t *)0x1FFF7594;  // Lire le deuxième mot de l'ID
//    id[2] = *(uint32_t *)0x1FFF7598;  // Lire le troisième mot de l'ID
//}
//void Bootloader_CopyInfoToRAM(BootloaderInfo_t *pRamInfo)
//{
//    const BootloaderInfo_t *pFlashInfo = (const BootloaderInfo_t *)BOOTLOADER_INFO_ADDRESS;
//    
//    if (pRamInfo != (void *)0)
//    {
//        (void)memcpy(pRamInfo, pFlashInfo, sizeof(BootloaderInfo_t));
//    }
//}
AppConfig_t config_read_back;
int main(void) {
//    uint32_t start_tick;
    uint8_t received_char;
//    bool enter_bootloader = false;
    uint32_t app_stack;
    /* Initialisation du système et configuration */
    HAL_Init();
    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
	SystemClock_Config2MZ(); 
    MX_GPIO_Init();
	MX_ADC1_Init();
	MX_ADC2_Init();	
	HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);
	HAL_ADCEx_Calibration_Start(&hadc2,ADC_SINGLE_ENDED);
	Read_ADC_Values();	
    Read_Structure_From_Flash(flash_address, &config_read_back, sizeof(AppConfig_t));
	Read_Structure_From_Flash(flash_address, &v_AppConfig, sizeof(AppConfig_t));
//    Read_Structure_From_Flash(flash_address, &config_read_back, sizeof(AppConfig_t));
//	if (config_read_back.uniqueID0 == 0xFFFFFFFF) {
//		Read_UniqueID(unique_id);		
//		config_read_back.uniqueID0  = unique_id[0];
//		config_read_back.uniqueID1  = unique_id[1];
//		config_read_back.uniqueID2  = unique_id[2];
//		config_read_back.CoefAnemo  = 1.1176f; 
//		config_read_back.CoefPluvio = 0.2f;
//		config_read_back.Temp_A = 1.0f;
//		config_read_back.Temp_B = 0.0f;
//		config_read_back.MAJEUR_VERSION  = 0x00;
//		config_read_back.MINEUR_VERSION  = 0x00;
//		config_read_back.RELEASE_VERSION = 0x00;
//        config_read_back.Presence = 0x12345678,
//        strcpy(config_read_back.Heure_Compile,__TIME__);
//        strcpy(config_read_back.Date_Compile, __DATE__);
//        strcpy(config_read_back.Version_Compile, "2.0.1");
//        strcpy(config_read_back.Boot_Bootloader, "NONE");
////		config_read_back.Reserved = {0xFF}  // Initialisation du remplissage à 0xFF		
//		Write_Structure_To_Flash(flash_address, &config_read_back, sizeof(AppConfig_t));
//	}
//	Read_Structure_From_Flash(flash_address, &v_AppConfig, sizeof(AppConfig_t));
	
//	if (v_ADC1_IN10 > 4.8) {
//		__NOP();
//	}
//	if (strncmp(config_read_back.Boot_Bootloader, "BOOT", 4) != 0) {
	if (v_ADC1_IN10 < 4.5)  {
		// BOOT FIRMWARE DIRECTEMENT
//		if (config_read_back.Presence == 0x12345678) {
			/* Lecture du pointeur de pile initial dans le flash de l'application */
			app_stack = *((volatile uint32_t *)APPLICATION_ADDRESS);
			/* Vérification simple que l'adresse du pointeur de pile est dans la plage RAM
			   (ici, 0x20000000 est l'adresse de base de la RAM sur beaucoup de STM32, 
			   à adapter selon votre microcontrôleur) */
			if ((app_stack & 0x2FFE0000U) == 0x20000000U)
			{
				Bootloader_JumpToApplication();
			}
//		}		
	} else {
		// BOOTLOADER MISE A JOURT PARAMETTRE
		SystemClock_Config();
		fifo_init(&usart2_fifo);
		
		UART2_Init();	
		HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);
		HAL_ADCEx_Calibration_Start(&hadc2,ADC_SINGLE_ENDED);
		Read_ADC_Values();
		MX_I2C1_Init();
		MX_OPAMP1_Init();
		MX_OPAMP2_Init();
		MX_TIM2_Init_1us();
		MX_GPIO_EXTI0_Init();
		MX_LPTIM1_Init();
		ADC12_COMMON->CCR |= ADC_CCR_VREFEN;	
		HAL_TIM_Base_Start_IT(&htim2);
		while (1) {
			/* Démarrage du compte à rebours de 10 secondes. La boucle s'exécute pendant 10 secondes, quel que soit l'état du CDC. */
			if (fifo_wait_for(&usart2_fifo, 1, BOOTLOADER_WAIT_TIME_MS) == FIFO_OK) {
				fifo_get(&usart2_fifo, &received_char);
				if (received_char == ' ') {
		//			enter_bootloader = true;
					Bootloader_Menu();
				}
			}
		}
	}
    return 0; /* Conformément à MISRA, retourner toujours 0 */
}
/* USER CODE BEGIN 4 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV2;
  RCC_OscInitStruct.PLL.PLLN = 42;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV6;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_8);

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }  
}
/* USER CODE END 4 */

//// Configuration de l'horloge du système
void SystemClock_Config2MZ(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    // Configurer l'oscillateur HSI
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI48;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        // Gestion d'erreur
        while (1);
    }

    // Configurer les horloges SYSCLK, HCLK, PCLK1 et PCLK2
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;

    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV8;  // SYSCLK = 2 MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
       while (1);
    }
	
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
	
//    HAL_PWREx_EnableLowPowerRunMode();	
	__HAL_RCC_LPTIM1_CONFIG(RCC_LPTIM1CLKSOURCE_LSI);  	
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}



#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
