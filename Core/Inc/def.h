#ifndef __DEF_H
#define __DEF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Définition de la taille Flash et des adresses */
/* Pour STM32G431KB, on suppose 128 Ko de Flash */
#define FLASH_BASE_ADDRESS   (0x08000000U)
//#define FLASH_SIZE           (0x20000U)        /* 128 Ko */
#define FLASH_END_ADDRESS    (FLASH_BASE_ADDRESS + FLASH_SIZE - 1U)

/**
 * @brief Adresse de stockage de la configuration dans la dernière page Flash.
 */
#define APP_CONFIG_ADDRESS   (FLASH_END_ADDRESS - (FLASH_PAGE_SIZE) + 1U)

/**
 * @brief Adresse de démarrage de l’application.
 *
 * La plage d’écriture pour la mise à jour se situe de APPLICATION_ADDRESS jusqu’à
 * APP_CONFIG_ADDRESS - 1.
 */
#define APPLICATION_ADDRESS  (0x08010000U)

/* Limites pour les coefficients */
#define COEF_ANEMO_MIN   (0.0f)
#define COEF_ANEMO_MAX   (5.0f)
#define COEF_PLUVIO_MIN  (0.0f)
#define COEF_PLUVIO_MAX  (2.0f)

/* Temps d'attente pour entrer en mode bootloader (en millisecondes) */
#define BOOTLOADER_WAIT_TIME_MS  (10000U)

/* FLASH_PAGE_SIZE : définir selon la mémoire flash du STM32G431KB (exemple : 2 Ko) */
#ifndef FLASH_PAGE_SIZE
#define FLASH_PAGE_SIZE  (0x800U)
#endif



/* Définitions des tailles et timeout */
#define PACKET_SIZE_128     (128U)     /**< Taille d'un paquet à 128 octets */
#define PACKET_SIZE_1024    (1024U)    /**< Taille d'un paquet à 1024 octets */
#define PACKET_OVERHEAD     (5U)       /**< 3 octets d'en-tête + 2 octets de CRC */
#define PACKET_TIMEOUT      (1000U)    /**< Timeout pour chaque octet en ms */
#define MAX_ERRORS          (10U)      /**< Nombre maximum d'erreurs tolérées */

#define MAX_FIFO_SIZE 2048


#define FLASH_APP_START_ADDRESS ((uint32_t)0x08010000u)
#define FLASH_APP_END_ADDRESS   ((uint32_t)FLASH_BANK1_END-0x800)

#define ANTIREBOND 0.04f /* Temps en secondes éivalent ࠱00 km/h */

/** 
 * @def FIFO_BUFFER_SIZE
 * @brief Taille du tampon du FIFO.
 */
#define FIFO_BUFFER_SIZE    (2048U)
/** 
 * @brief Codes de retour pour les fonctions du FIFO.
 */
#define FIFO_OK             (0)
#define FIFO_ERROR          (-1)

#ifdef __cplusplus
}
#endif

#endif /* __DEF_H */


