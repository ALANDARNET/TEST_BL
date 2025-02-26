#ifndef __STRUCT_H
#define __STRUCT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Structure de configuration de l’application stockée en Flash.
 */
// Définition de la structure alignée à 2048 octets
typedef struct {
    uint32_t uniqueID0;
    uint32_t uniqueID1;
    uint32_t uniqueID2;
    bool     permit_transmit;
    float    CoefAnemo;
    float    CoefPluvio;
    float    Temp_A;
    float    Temp_B;
    float    vitesse_vent;
    float    Pluvio;
    float    Temperature;
    uint32_t MAJEUR_VERSION;
    uint32_t MINEUR_VERSION;
    uint32_t RELEASE_VERSION;
    uint32_t Presence;	
    char Heure_Compile[32];
    char Date_Compile[32];
    char Version_Compile[32];
    char Boot_Bootloader[4];
    uint8_t Reserved[2048 - (4*3 + 1 + 4*9 + 4*3 + 32 + 32 + 32 + 4)];  // Ajustement final
} __attribute__((aligned(2048))) AppConfig_t;


/**
 * @brief Structure représentant le FIFO.
 */
typedef struct
{
    uint8_t buffer[FIFO_BUFFER_SIZE]; /**< Tableau de stockage des octets. */
    volatile uint32_t head;           /**< Indice de la tête (position d'écriture). */
    volatile uint32_t tail;           /**< Indice de la queue (position de lecture). */
} fifo_t;


/**
 * @brief Type de fonction de rappel pour le traitement d'un bloc reçu.
 *
 * Cette fonction sera appelée dès qu'un bloc XMODEM 1K valide est reçu.
 *
 * @param[in] block         Pointeur sur le bloc de données (1024 octets).
 * @param[in] block_number  Numéro du bloc reçu.
 */
typedef void (*xmodem_block_callback_t)(const uint8_t *block, uint32_t block_number, uint16_t received_crc);




#ifdef __cplusplus
}
#endif

#endif /* __STRUCT_H */


