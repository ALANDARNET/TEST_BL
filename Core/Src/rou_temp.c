#include "inc.h"
#include "adc.h"
#include "stm32g4xx_hal_adc.h"

//static uint32_t w_etat_temp_ana = 0L;
//static uint32_t v_gest_temp_trait = L_INIT; 

//static uint8_t w_ptr_pwm  = 0xff;
//static int32_t temp;
//static float w_temperature = 0.0f;
//static uint32_t adcValueTemp = 0;
//static uint32_t adcValueVref = 0;
//static uint32_t 	numOfSamples = 2;
//static float vdda = 3.0f; // Tension d'alimentation par défaut en volts
//static uint16_t tempCal1; // TS_CAL1
//static uint16_t tempCal2; // TS_CAL2
ADC_Common_TypeDef *adcCommon;// = __HAL_ADC_COMMON_INSTANCE(&hadc1);
ADC_ChannelConfTypeDef sConfig = {0};

/* Constant values BEGIN */
#define TMPSENSOR_AVGSLOPE 2.5 /* mV/°C */
#define TMPSENSOR_V25  0.76 /* V (entre 25-35 °C)  */

#define TMPSENSOR_ADCMAX 4095.0 /* 12-bit ADC maximum value (12^2)-1)  */
#define TMPSENSOR_ADCREFVOL  3.3 /* Typical reference voltage, V  */
#define TMPSENSOR_ADCVREFINT  1.21 /* Internal reference voltage, V  */
/* Constant values END */

volatile uint32_t w_nbr_acq1_temp = 0;
volatile uint32_t w_nbr_acq2_temp = 0;
uint32_t adc_sensor_temp = 0;
uint32_t adc_intref = 0;
uint32_t adc1_in10 = 0;
uint32_t adc2_in10 = 0;

uint16_t v_moy_adc_sensor_temp = 0;
uint16_t v_moy_adc_intref = 0;
uint16_t v_moy_adc1_in10 = 0;
uint16_t v_moy_adc2_in10 = 0;
float v_temperture_global = 0.0f;
#define L_NB_ACQ_TEMP 100
float w_ADC1_IN10 = 0.0f;
float w_ADC2_IN10 = 0.0f;
const float w_unit = 3.3/4096.0;
/**
  * @brief Callback appelé lorsque le buffer de conversion ADC est rempli.
  * @param hadc Pointeur sur le handle de l'ADC.
  */
  
#define TMP1075_I2C_ADDRESS         (0x48U)  /**< Adresse 7 bits du TMP1075N sur le bus I2C (à ajuster si nécessaire) */
#define TMP1075_REG_TEMPERATURE     (0x00U)  /**< Adresse du registre de température */

/**
  * @brief  Lit la température depuis le capteur TMP1075N via I2C1.
  * @return La température en degrés Celsius (type float). En cas d'erreur,
  *         la fonction retourne -273.15°C (valeur indicative).
  */
float TMP1075_ReadTemperature(void) {
    HAL_StatusTypeDef status;
    uint8_t temp_data[2U] = {0U, 0U};
    int16_t raw_temp;
    float temperature;

    /* Lecture de 2 octets à partir du registre de température */
    status = HAL_I2C_Mem_Read(&hi2c1,
                              (TMP1075_I2C_ADDRESS << 1U),
                              TMP1075_REG_TEMPERATURE,
                              I2C_MEMADD_SIZE_8BIT,
                              temp_data,
                              2U,
                              HAL_MAX_DELAY);
    if (status != HAL_OK)
    {
        /* Gestion d'erreur (à adapter selon vos besoins) */
        temperature = -273.15F; /* Retourne une valeur indicative en cas d'erreur */
    }
    else
    {
        /* Reconstitution de la donnée sur 16 bits */
//        raw_temp = (int16_t)(((uint16_t)temp_data[0U] << 8U) | temp_data[1U]);
		raw_temp = (((temp_data[0] << 8) | temp_data[1]) >> 4);
        /* Conversion de la valeur brute en température en °C.
         * Le facteur de conversion (ici 0.0078F) est donné à titre d'exemple.
         * Consultez la datasheet du TMP1075N pour connaître la conversion exacte.
         */
        temperature = (float)raw_temp;
		v_temperture_TM1075 = (float)temperature;
		v_temperture_TM1075 *= 0.0625f;
    }
	

    return v_temperture_TM1075;
}  
  
  
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    /* Vérifier que c'est bien le canal DMA associé à l'ADC */
	if (hadc->Instance == ADC1) {
		if ( w_nbr_acq1_temp < L_NB_ACQ_TEMP) {
			adc1_in10 += AD1_RES[0];
			adc2_in10 += AD1_RES[1];
			w_nbr_acq1_temp++;
		} else {
			v_moy_adc1_in10 = adc1_in10 / L_NB_ACQ_TEMP;
			v_moy_adc2_in10 = adc2_in10 / L_NB_ACQ_TEMP;
			w_ADC1_IN10 = (float)v_moy_adc1_in10 * w_unit * 1.511;
			w_ADC2_IN10 = (float)v_moy_adc2_in10 * w_unit * 1.511;

			v_moy_adc1_in10 = 0;
			v_moy_adc2_in10 = 0;
			w_nbr_acq1_temp = 0L;
			adc1_in10 = 0;
			adc2_in10 = 0;
		}
	}
}


float Read_Temperature(void) {
	v_temperture_TM1075 = TMP1075_ReadTemperature();
	v_temperture_TM1075 *= 0.0625f;
	return v_temperture_TM1075;	

}


/**
  * @brief Initialisation de I2C1 pour le TMP1075N.
  * @note  Cette fonction configure I2C1 en mode 7 bits, avec un timing adapté pour une
  *        communication à 100 kHz. Les paramètres de configuration peuvent être ajustés en
  *        fonction de la configuration de votre système.
  * @retval None
  */
void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x00707CBB;  /* Timing pour 100 kHz, à adapter selon votre configuration */
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    
    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        /* En cas d'erreur, appeler une fonction de gestion d'erreur */
        Error_Handler();
    }
    
    /* Configuration du filtre analogique */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        Error_Handler();
    }
    
    /* Configuration du filtre numérique (0 = désactivation) */
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
    {
        Error_Handler();
    }
}


/**
  * @brief Initialise le MSP pour I2C1.
  * @param hi2c Pointeur vers la structure de gestion de l'I2C.
  * @retval None
  */
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
    GPIO_InitTypeDef  GPIO_InitStruct = {0};

    if(hi2c->Instance == I2C1) {
        /* Activation des horloges pour les ports GPIOA et GPIOB */
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        /* Configuration de PA15 pour I2C1 SCL */
        GPIO_InitStruct.Pin = GPIO_PIN_15;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;           /**< Mode open-drain pour I2C */
        GPIO_InitStruct.Pull = GPIO_PULLUP;                /**< Pull-up activé */
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;           /**< Fonction alternative I2C1 */
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* Configuration de PB7 pour I2C1 SDA */
        GPIO_InitStruct.Pin = GPIO_PIN_7;
        /* Mode, Pull, Speed et Alternate restent identiques */
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* Activation de l'horloge du périphérique I2C1 */
        __HAL_RCC_I2C1_CLK_ENABLE();
    }
}

