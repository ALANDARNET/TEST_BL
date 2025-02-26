#include "inc.h"

const uint32_t flash_address_config = 0x0801F800;

float v_temperature_mesuree;
AppConfig_t v_config_system;

fifo_t usart2_fifo;
//__attribute__((section("BootloaderInfoSection"), used))  BootloaderInfo_t appInfoRAM;
AppConfig_t v_AppConfig;
const uint32_t flash_address = 0x0801F800;

float v_vitesse_vent;

TIM_HandleTypeDef htim2;
LPTIM_HandleTypeDef hlptim1;
volatile float flast_tick;
volatile float fcurrent_tick;
volatile float interval;
volatile float t1;
volatile float t2;
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
float f_temp_value;
uint16_t AD1_RES[4];

float v_ADC1_IN10;
float v_ADC2_IN10;
UART_HandleTypeDef hUART1;
UART_HandleTypeDef hUART2;
TIM_HandleTypeDef    TimHandle;
I2C_HandleTypeDef hi2c1;
float v_temperture_TM1075;


