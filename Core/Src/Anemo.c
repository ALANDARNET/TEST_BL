#include "inc.h"



float TVraiInt = 0.0f;
volatile float wind_speed = 0.0f;
float calculer_vitesse_vent(float Tvrai);
const float initial_interrupt_rate = 1.0; // Ajustez en fonction de votre anémométre

/** Constantes */
#define ANTIREBOND 0.04f /* Temps en secondes équivalent à 100 km/h */


/**
 * @brief  Dernière capture de TIM2 (en µs).
 */
volatile uint32_t g_tim2_lastCapture = 0UL;
/**
 * @brief  Delta mesuré entre deux interruptions EXTI0 (en µs).
 */
volatile uint32_t g_exti0_delta_us   = 0UL;
volatile uint32_t g_exti0_currentCapture   = 0UL;
/**
 * @brief  Callback appelé à chaque front EXTI sur GPIO_PIN_0.
 * @param  GPIO_Pin : Pin ayant déclenché l’IT.
 * @return Aucun.
 */
//volatile uint32_t currentCapture;
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == ANEMO_IRQ_Pin) {
		/* Lire la valeur du compteur TIM2 (1 tick = 1 µs) */
		uint32_t currentCapture = __HAL_TIM_GET_COUNTER(&htim2);

		/* Calculer la différence (unsigned -> gère le débordement 32 bits) */
		uint32_t delta = currentCapture - g_tim2_lastCapture;

		/* Mettre à jour la valeur globale */
		g_exti0_delta_us = delta;
		g_tim2_lastCapture = currentCapture;	
	}
}

/**
 * @brief  Callback appelée quand l’Auto-Reload est atteint.
 * @param  hlptim : pointeur sur le handle du LPTIM
 * @return Aucun.
 */
// Callback appelé à chaque interruption de LPTIM1
extern const char w_tx_buffer[];// 	= "0R0,Dm=224D,Sm=%2.1fM,Ta=%2.1fC,Ua=59.9P,Pa=988.4H,Ri=%2.1fM,Th=28.4C,Vh=12.1N\x0d\x0";
char w_tx_bufferDec[150];

void HAL_LPTIM_AutoReloadMatchCallback(LPTIM_HandleTypeDef *hlptim) {
	(void)hlptim;

	TVraiInt = (float)get_time_us() / 1000000.0f; // Convertir en secondes

	// Calcul de la vitesse du vent
	float vitesse_vent = calculer_vitesse_vent(TVraiInt);
	v_vitesse_vent = vitesse_vent * v_config_system.CoefAnemo;
//	v_vitesse_vent = (uint16_t)(v_config_system.vitesse_vent * 10.0f);
//	v_vitesse_vent = (float)VentInt/10.0f;
}

/**
 * @brief Fonction pour obtenir le temps actuel en microsecondes.
 * @return Temps actuel en microsecondes (uint32_t).
 */
uint32_t get_time_us(void) {
    // Capturer le temps actuel en ticks et le convertir en microsecondes
	return(__HAL_TIM_GET_COUNTER(&htim2));
}

/**
 * @brief Programme principal pour le calcul de la vitesse du vent.
 * @param Tvrai Temps actuel selon l'horloge logger.
 * @return Vitesse du vent en km/h.
 */
float calculer_vitesse_vent(float Tvrai) {
//    float T1 = t1;
    float T2 = flast_tick;
    float rps;

    /* Utiliser l'interval calculé sous interruption */
    if ((Tvrai - T2) > (g_exti0_delta_us / 1000000.0f)) {
        rps = 1.0f / (Tvrai - T2);
    } else {
        rps = 1.0f / (g_exti0_delta_us / 1000000.0f);
    }
    return rps;
}
