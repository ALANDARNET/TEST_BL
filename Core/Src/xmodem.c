#include <stdint.h>
#include <string.h>
#include "inc.h"      /* Contient les définitions de fifo_t, HAL_GetTick(), fifo_get(), fifo_wait_for(), etc. */


/**
 * @brief Table de pré-calcul pour le CRC16 (polynôme 0x1021).
 */
static const uint16_t crc16_table[256] = {
    0x0000U, 0x1021U, 0x2042U, 0x3063U, 0x4084U, 0x50A5U, 0x60C6U, 0x70E7U,
    0x8108U, 0x9129U, 0xA14AU, 0xB16BU, 0xC18CU, 0xD1ADU, 0xE1CEU, 0xF1EFU,
    0x1231U, 0x0210U, 0x3273U, 0x2252U, 0x52B5U, 0x4294U, 0x72F7U, 0x62D6U,
    0x9339U, 0x8318U, 0xB37BU, 0xA35AU, 0xD3BDU, 0xC39CU, 0xF3FFU, 0xE3DEU,
    0x2462U, 0x3443U, 0x0420U, 0x1401U, 0x64E6U, 0x74C7U, 0x44A4U, 0x5485U,
    0xA56AU, 0xB54BU, 0x8528U, 0x9509U, 0xE5EEU, 0xF5CFU, 0xC5ACU, 0xD58DU,
    0x3653U, 0x2672U, 0x1611U, 0x0630U, 0x76D7U, 0x66F6U, 0x5695U, 0x46B4U,
    0xB75BU, 0xA77AU, 0x9719U, 0x8738U, 0xF7DFU, 0xE7FEU, 0xD79DU, 0xC7BCU,
    0x48C4U, 0x58E5U, 0x6886U, 0x78A7U, 0x0840U, 0x1861U, 0x2802U, 0x3823U,
    0xC9CCU, 0xD9EDU, 0xE98EU, 0xF9AFU, 0x8948U, 0x9969U, 0xA90AU, 0xB92BU,
    0x5AF5U, 0x4AD4U, 0x7AB7U, 0x6A96U, 0x1A71U, 0x0A50U, 0x3A33U, 0x2A12U,
    0xDBFDU, 0xCBDCU, 0xFBBFU, 0xEB9EU, 0x9B79U, 0x8B58U, 0xBB3BU, 0xAB1AU,
    0x6CA6U, 0x7C87U, 0x4CE4U, 0x5CC5U, 0x2C22U, 0x3C03U, 0x0C60U, 0x1C41U,
    0xEDAEU, 0xFD8FU, 0xCDECU, 0xDDCDU, 0xAD2AU, 0xBD0BU, 0x8D68U, 0x9D49U,
    0x7E97U, 0x6EB6U, 0x5ED5U, 0x4EF4U, 0x3E13U, 0x2E32U, 0x1E51U, 0x0E70U,
    0xFF9FU, 0xEFBEU, 0xDFDDU, 0xCFFCU, 0xBF1BU, 0xAF3AU, 0x9F59U, 0x8F78U,
    0x9188U, 0x81A9U, 0xB1CAU, 0xA1EBU, 0xD10CU, 0xC12DU, 0xF14EU, 0xE16FU,
    0x1080U, 0x00A1U, 0x30C2U, 0x20E3U, 0x5004U, 0x4025U, 0x7046U, 0x6067U,
    0x83B9U, 0x9398U, 0xA3FBU, 0xB3DAU, 0xC33DU, 0xD31CU, 0xE37FU, 0xF35EU,
    0x02B1U, 0x1290U, 0x22F3U, 0x32D2U, 0x4235U, 0x5214U, 0x6277U, 0x7256U,
    0xB5EAU, 0xA5CBU, 0x95A8U, 0x8589U, 0xF56EU, 0xE54FU, 0xD52CU, 0xC50DU,
    0x34E2U, 0x24C3U, 0x14A0U, 0x0481U, 0x7466U, 0x6447U, 0x5424U, 0x4405U,
    0xA7DBU, 0xB7FAU, 0x8799U, 0x97B8U, 0xE75FU, 0xF77EU, 0xC71DU, 0xD73CU,
    0x26D3U, 0x36F2U, 0x0691U, 0x16B0U, 0x6657U, 0x7676U, 0x4615U, 0x5634U,
    0xD94CU, 0xC96DU, 0xF90EU, 0xE92FU, 0x99C8U, 0x89E9U, 0xB98AU, 0xA9ABU,
    0x5844U, 0x4865U, 0x7806U, 0x6827U, 0x18C0U, 0x08E1U, 0x3882U, 0x28A3U,
    0xCB7DU, 0xDB5CU, 0xEB3FU, 0xFB1EU, 0x8BF9U, 0x9BD8U, 0xABBBU, 0xBB9AU,
    0x4A75U, 0x5A54U, 0x6A37U, 0x7A16U, 0x0AF1U, 0x1AD0U, 0x2AB3U, 0x3A92U,
    0xFD2EU, 0xED0FU, 0xDD6CU, 0xCD4DU, 0xBDAAU, 0xAD8BU, 0x9DE8U, 0x8DC9U,
    0x7C26U, 0x6C07U, 0x5C64U, 0x4C45U, 0x3CA2U, 0x2C83U, 0x1CE0U, 0x0CC1U,
    0xEF1FU, 0xFF3EU, 0xCF5DU, 0xDF7CU, 0xAF9BU, 0xBFBAU, 0x8FD9U, 0x9FF8U,
    0x6E17U, 0x7E36U, 0x4E55U, 0x5E74U, 0x2E93U, 0x3EB2U, 0x0ED1U, 0x1EF0U
};


/* Macros XMODEM identiques aux versions précédentes */
#define XMODEM_STX            0x02U   /**< Start Of Text pour blocs de 1024 octets (XMODEM-1K) */
#define XMODEM_EOT            0x04U   /**< End Of Transmission */
#define XMODEM_ACK            0x06U   /**< Acknowledge */
#define XMODEM_NAK            0x15U   /**< Negative Acknowledge */
#define XMODEM_CAN            0x18U   /**< Cancel */

#define XMODEM_HEADER_TIMEOUT_MS    1000U  /**< Délai pour l'en-tête (ms) */
#define XMODEM_BYTE_TIMEOUT_MS      1000U   /**< Délai pour un octet (ms) */
#define XMODEM_MAX_RETRIES          50U    /**< Nombre maximal d'essais */
#define XMODEM_1K_BLOCK_SIZE        1024U  /**< Taille d'un bloc XMODEM 1K */


/* Adresse de départ en flash pour l'écriture (à adapter selon votre configuration) */
#define FLASH_APP_ADDRESS   0x08010000U

/* Taille d'un bloc XMODEM 1K */
#define XMODEM_1K_BLOCK_SIZE        1024U

/* Taille d'un paquet (deux blocs de 1024 octets) */
#define FLASH_PACKET_SIZE           (2U * XMODEM_1K_BLOCK_SIZE)


/* Prototypes des fonctions externes */
extern uint32_t HAL_GetTick(void);      /**< Retourne le tick système */

/* Prototypes des fonctions flash (à adapter selon votre plateforme) */
extern int flash_write(uint32_t address, const uint8_t *data, uint32_t length);
extern int flash_read(uint32_t address, uint8_t *data, uint32_t length);
/**
 * @brief Calcule le CRC16 (polynôme 0x1021) sur un bloc de données (méthode XMODEM).
 *
 * @param[in] data   Pointeur sur le bloc de données.
 * @param[in] length Longueur du bloc.
 * @return uint16_t CRC16 calculé.
 */
static uint16_t xmodem_compute_crc16(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0U;
    uint16_t i;
    uint8_t j;
    uint8_t byte;

    for (i = 0U; i < length; i++)
    {
        byte = data[i];
        crc ^= ((uint16_t)byte << 8U);
        for (j = 0U; j < 8U; j++)
        {
            if ((crc & 0x8000U) != 0U)
            {
                crc = (uint16_t)((crc << 1U) ^ 0x1021U);
            }
            else
            {
                crc <<= 1U;
            }
        }
    }
    return crc;
}



/**
 * @brief Réception XMODEM 1K en mode blockwise.
 *
 * Cette fonction implémente le protocole XMODEM 1K en mode réception, sans stocker
 * la totalité des paquets reçus en RAM. Pour chaque bloc valide, elle appelle la fonction
 * de rappel fournie.
 *
 * @param[in,out] fifo      Pointeur vers la FIFO d'où seront extraits les octets reçus.
 * @param[in]     callback  Fonction de rappel appelée pour traiter chaque bloc.
 * @return int  0 si la transmission s'est correctement terminée (EOT reçu), -1 en cas d'erreur.
 */
int xmodem_receive_1k_blockwise(fifo_t *fifo, xmodem_block_callback_t callback)
{
    uint8_t block_expected = 1U;
    uint8_t retry = 0U;
    uint8_t header;
    uint8_t block_num;
    uint8_t block_num_comp;
    uint8_t data[XMODEM_1K_BLOCK_SIZE];
    uint8_t crc_msb;
    uint8_t crc_lsb;
    uint16_t rx_crc;
    uint16_t calc_crc;
    uint32_t i;
    int status;


    /* Envoi initial de 'C' pour signaler l'utilisation du CRC */
    SendCharFTDI((uint8_t)'C');

    for (;;) {
        /* Attente de réception d'au moins un octet */
        if (fifo_wait_for(fifo, 1U, XMODEM_HEADER_TIMEOUT_MS) == FIFO_ERROR) {
            retry++;
            if (retry >= XMODEM_MAX_RETRIES) {
                SendCharFTDI(XMODEM_CAN);
                SendCharFTDI(XMODEM_CAN);
                return -1;
            }
            SendCharFTDI((uint8_t)'C');
            continue;
        }

        /* Récupération de l'en-tête */
//		while(fifo_is_empy(&usart2_fifo)) {};
        status = fifo_get(fifo, &header);
        if (status == FIFO_ERROR) {
            continue;
        }

        if (header == XMODEM_EOT) {
            SendCharFTDI(XMODEM_ACK);
            break;
        } else if (header == XMODEM_STX) {
            /* Bloc XMODEM 1K : STX, blk, ~blk, 1024 octets, CRC16 (2 octets) */
            if (fifo_wait_for(fifo, 2U + XMODEM_1K_BLOCK_SIZE + 2U, XMODEM_HEADER_TIMEOUT_MS) == 0) {
                SendCharFTDI(XMODEM_NAK);
                continue;
            }
			while(fifo_is_empy(&usart2_fifo)) {};
            (void)fifo_get(fifo, &block_num);
			while(fifo_is_empy(&usart2_fifo)) {};
            (void)fifo_get(fifo, &block_num_comp);
            if (((uint8_t)(block_num + block_num_comp)) != 0xFFU) {
                SendCharFTDI(XMODEM_NAK);
                continue;
            }
            /* Réception des 1024 octets */
            for (i = 0U; i < XMODEM_1K_BLOCK_SIZE; i++) {
				while(fifo_is_empy(&usart2_fifo)) {};
                status = fifo_get(fifo, &data[i]);
                if (status == 0) {
                    break;
                }
            }
            if (i != XMODEM_1K_BLOCK_SIZE) {
                SendCharFTDI(XMODEM_NAK);
                continue;
            }
            /* Réception du CRC16 (MSB puis LSB) */
			while(fifo_is_empy(&usart2_fifo)) {};
            if (fifo_get(fifo, &crc_msb) == 0) {
                SendCharFTDI(XMODEM_NAK);
                continue;
            }
			while(fifo_is_empy(&usart2_fifo)) {};
            if (fifo_get(fifo, &crc_lsb) == 0) {
                SendCharFTDI(XMODEM_NAK);
                continue;
            }
            rx_crc = (uint16_t)(((uint16_t)crc_msb << 8U) | crc_lsb);
            calc_crc = xmodem_compute_crc16(data, XMODEM_1K_BLOCK_SIZE);
            if (calc_crc != rx_crc) {
                SendCharFTDI(XMODEM_NAK);
                continue;
            }
            /* Gestion des numéros de bloc */
			if (block_num == block_expected) {
				/* Appel de la fonction de callback pour traiter le bloc en passant le CRC reçu */
				callback(data, block_expected, rx_crc);
				block_expected++;
				SendCharFTDI(XMODEM_ACK);
			} else if (block_num == (uint8_t)(block_expected - 1U)) {
                /* Bloc dupliqué : renvoi d'un ACK */
                SendCharFTDI(XMODEM_ACK);
            } else {
                SendCharFTDI(XMODEM_NAK);
            }
            retry = 0U;
        } else if (header == XMODEM_CAN) {
            if (fifo_wait_for(fifo, 1U, XMODEM_BYTE_TIMEOUT_MS) != 0) {
				while(fifo_is_empy(&usart2_fifo)) {};
                (void)fifo_get(fifo, &header);
                if (header == XMODEM_CAN) {
                    SendCharFTDI(XMODEM_ACK);
                    return -1;
                }
            }
            SendCharFTDI(XMODEM_NAK);
        } else {
            SendCharFTDI(XMODEM_NAK);
        }
    }
    return 0;
}


/**
 * @brief Calcule le CRC16 (polynôme 0x1021) sur un bloc de données à l'aide d'une table pré‑calculée.
 *
 * @param[in] data   Pointeur sur le buffer de données.
 * @param[in] length Longueur des données en octets.
 * @return uint16_t CRC16 calculé.
 */
static uint16_t flash_compute_crc16(const uint8_t *data, uint32_t length)
{
    uint16_t crc = 0U;
    uint32_t i;

    if (data == NULL)
    {
        /* En cas d'erreur, vous pouvez gérer le problème ici (ex. retourner 0 ou lever une erreur) */
        return 0U;
    }

    for (i = 0U; i < length; i++)
    {
        crc = (uint16_t)((crc << 8U) ^ crc16_table[((crc >> 8U) ^ data[i]) & 0xFFU]);
    }
    return crc;
}
/**
 * @brief Callback pour accumuler deux blocs et écrire 2048 octets en flash.
 *
 * Cette fonction est appelée pour chaque bloc de 1024 octets reçu.
 * Elle accumule les blocs dans un tampon statique. Lorsque deux blocs ont été
 * réceptionnés, elle écrit les 2048 octets en flash à l'adresse FLASH_APP_ADDRESS
 * (mise à jour au fur et à mesure), relit les données écrites, recalcule le CRC
 * sur chaque segment de 1024 octets et compare avec les CRC reçus.
 *
 * @param[in] block         Pointeur sur le bloc de 1024 octets reçu.
 * @param[in] block_number  Numéro du bloc reçu (commençant par 1).
 * @param[in] received_crc  CRC calculé lors de la réception pour ce bloc.
 */
static uint8_t flash_buffer[FLASH_PACKET_SIZE] = {0};
static uint8_t verify_buffer[FLASH_PACKET_SIZE] = {0};
void flash_write_callback(const uint8_t *block, uint32_t block_number, uint16_t received_crc) {
    /* Tampon statique pour accumuler deux blocs (2048 octets) */
    /* Tableau statique pour mémoriser les CRC reçus pour chaque bloc */
    static uint16_t rx_crc[2] = {0U, 0U};
    /* Compteur de blocs accumulés dans le paquet courant (0 ou 1) */
    static uint8_t block_index = 0U;
    /* Adresse de flash courante, initialisée au départ défini par FLASH_APP_ADDRESS */
    static uint32_t flash_current_address = FLASH_APP_ADDRESS;
    int ret;
    uint16_t calc_crc;
	(void)block_number;
    /* Copie du bloc reçu dans le tampon correspondant */
    memcpy(&flash_buffer[block_index * XMODEM_1K_BLOCK_SIZE], block, XMODEM_1K_BLOCK_SIZE);
    rx_crc[block_index] = received_crc;
    block_index++;

    /* Si deux blocs ont été accumulés, écrire en flash */
    if (block_index == 2U) {
        /* Écriture en flash de 2048 octets à l'adresse flash_current_address */
        ret = flash_write(flash_current_address, flash_buffer, FLASH_PACKET_SIZE);
        if (ret != 0) {
//            printf("Erreur d'écriture flash à l'adresse 0x%08lX\r\n", (unsigned long)flash_current_address);
            /* Réinitialiser block_index pour tenter de reprendre la transmission */
            block_index = 0U;
            return;
        }

        /* Lecture de la zone flash pour vérification */
        ret = flash_read(flash_current_address, verify_buffer, FLASH_PACKET_SIZE);
        if (ret != 0)
        {
//            printf("Erreur de lecture flash à l'adresse 0x%08lX\r\n", (unsigned long)flash_current_address);
            block_index = 0U;
            return;
        }

        /* Calcul et comparaison du CRC sur le premier bloc (1024 octets) */
        calc_crc = flash_compute_crc16(&verify_buffer[0], XMODEM_1K_BLOCK_SIZE);
        if (calc_crc != rx_crc[0])
        {
//            printf("Erreur CRC sur bloc 1: attendu 0x%04X, calculé 0x%04X\r\n", rx_crc[0], calc_crc);
            block_index = 0U;
            return;
        }

        /* Calcul et comparaison du CRC sur le deuxième bloc (1024 octets) */
        calc_crc = flash_compute_crc16(&verify_buffer[XMODEM_1K_BLOCK_SIZE], XMODEM_1K_BLOCK_SIZE);
        if (calc_crc != rx_crc[1])
        {
//            printf("Erreur CRC sur bloc 2: attendu 0x%04X, calculé 0x%04X\r\n", rx_crc[1], calc_crc);
            block_index = 0U;
            return;
        }

        /* Si la vérification est réussie, mise à jour de l'adresse flash pour le prochain paquet */
//        printf("Paquet de 2048 octets écrit et vérifié à l'adresse 0x%08lX\r\n", (unsigned long)flash_current_address);
        flash_current_address += FLASH_PACKET_SIZE;
        /* Réinitialisation du compteur de blocs pour le prochain paquet */
        block_index = 0U;
    }
}
