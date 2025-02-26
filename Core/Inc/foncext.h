#ifndef __FONCEXT_H
#define __FONCEXT_H

#ifdef __cplusplus
extern "C" {
#endif
void MX_GPIO_Init(void);
void Flash_Erase(uint32_t start_address, uint32_t end_address);

void Config_Read(AppConfig_t * const config);
//void Config_Write(const AppConfig_t * const config);
void Write_Structure_To_Flash(uint32_t address, void *data, size_t size);

float GetTemperatureSensorReading(void);
int Ymodem_ReceivePacket(uint8_t *p_data, uint16_t *p_length, uint8_t *p_packet_number, uint32_t timeout);
int YMODEM_Receive(void);
void Bootloader_JumpToApplication(void);
void Bootloader_Menu(void);

void XMODEM_Init(void);
//void xmodem_receive(void);
int xmodem_receive_1k_blockwise(fifo_t *fifo, xmodem_block_callback_t callback);
void flash_write_callback(const uint8_t *block, uint32_t block_number, uint16_t received_crc);
void MX_TIM2_Init_1us(void);
uint32_t get_time_us(void);
void MX_GPIO_EXTI0_Init(void);
void MX_LPTIM1_Init(void);
double TMPSENSOR_Read(void);
void MX_ADC1_Init(void);
void MX_ADC2_Init(void);
void Error_Handler(void);
void SendStringFTDI(char *Chaine);
void SendCharFTDI(char Chaine);
void UART2_Init(void);
void MX_ADC_MultiMode_Init(void);
void Read_ADC_Values(void);
bool fifo_is_empy(fifo_t *fifo);
float TMP1075_ReadTemperature(void);
void MX_I2C1_Init(void);
void Read_Structure_From_Flash(uint32_t address, void *data, size_t size);
#ifdef __cplusplus
}
#endif

#endif /* __FONCEXT_H */


