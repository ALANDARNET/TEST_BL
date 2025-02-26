#ifndef __RAMEXT_H
#define __RAMEXT_H

#ifdef __cplusplus
extern "C" {
#endif
extern const uint32_t flash_address_config;

extern float v_temperature_mesuree;
extern AppConfig_t v_config_system;
extern AppConfig_t v_AppConfig;
extern const uint32_t flash_address;
extern fifo_t usart2_fifo;
//extern BootloaderInfo_t appInfoRAM;
extern float v_vitesse_vent;
//extern TIM_HandleTypeDef htim3;

extern TIM_HandleTypeDef htim2;
extern LPTIM_HandleTypeDef hlptim1;
extern volatile float flast_tick;
extern volatile float fcurrent_tick;
extern volatile float interval;
extern volatile float t1;
extern volatile float t2;
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern float f_temp_value;
extern uint16_t AD1_RES[4];

extern float v_ADC1_IN10;
extern float v_ADC2_IN10;
extern UART_HandleTypeDef hUART1;
extern UART_HandleTypeDef hUART2;
extern I2C_HandleTypeDef hi2c1;
extern float v_temperture_TM1075;

#ifdef __cplusplus
}
#endif

#endif /* __RAMEXT_H */


