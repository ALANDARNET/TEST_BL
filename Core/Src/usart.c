#include "inc.h"
extern volatile unsigned char received_char;
extern volatile unsigned char received_char2;

void UART2_Init(void) {
//	char RX_Buffer;
    __HAL_RCC_USART2_CLK_ENABLE();

	hUART2.Instance = USART2;
	hUART2.Init.BaudRate = 38400;
	hUART2.Init.WordLength = UART_WORDLENGTH_8B;
	hUART2.Init.StopBits = UART_STOPBITS_1;
	hUART2.Init.Parity = UART_PARITY_NONE;
	hUART2.Init.Mode = UART_MODE_TX_RX;
	hUART2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	hUART2.Init.OverSampling = UART_OVERSAMPLING_16;
	hUART2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	hUART2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
	if (HAL_UART_Init(&hUART2) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_UARTEx_SetTxFifoThreshold(&hUART2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_UARTEx_SetRxFifoThreshold(&hUART2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_UARTEx_DisableFifoMode(&hUART2) != HAL_OK)
	{
		Error_Handler();
	}
	
	HAL_UART_RegisterCallback(&hUART2,HAL_UART_RX_COMPLETE_CB_ID,HAL_UART_RxCpltCallback);	
	__HAL_UART_ENABLE_IT(&hUART2, UART_IT_RXNE);
	
	HAL_NVIC_SetPriority(USART2_IRQn, 0U, 0U);
	HAL_NVIC_EnableIRQ(USART2_IRQn);
	HAL_UART_Receive_IT(&hUART2, (uint8_t *)&received_char2, 1);	
}

/* USART1 init function */
/* USART2 init function */
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
	if(uartHandle->Instance==USART1) {

		PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
		PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
		{
		  Error_Handler();
		}

		/* USART2 clock enable */
		__HAL_RCC_USART2_CLK_ENABLE();

		__HAL_RCC_GPIOB_CLK_ENABLE();
		/**USART2 GPIO Configuration
		PB3     ------> USART2_TX
		PB4     ------> USART2_RX
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	}
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

	if(uartHandle->Instance==USART2) {
		__HAL_RCC_USART2_CLK_DISABLE();
		/**USART1 GPIO Configuration
		PB3     ------> USART1_TX
		PB4     ------> USART1_RX
		*/
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3|GPIO_PIN_4);
		/* USART1 interrupt Deinit */
		HAL_NVIC_DisableIRQ(USART2_IRQn);
	}
}
