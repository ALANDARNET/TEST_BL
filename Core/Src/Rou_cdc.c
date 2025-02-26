#include "inc.h"

/* Variable globale générée par CubeMX */
extern USBD_HandleTypeDef hUsbDeviceFS;



/**
 * @brief Buffer circulaire pour la réception via USB CDC.
 */

/**
 * @brief Indice d'insertion dans le buffer.
 */
volatile uint16_t CDC_RxHead = 0U;

/**
 * @brief Indice de lecture dans le buffer.
 */
volatile uint16_t CDC_RxTail = 0U;

/**
 * @brief Vérifie si l'USB CDC est initialisé et configuré.
 *
 * Cette fonction vérifie que l'état de l'USB CDC est USBD_STATE_CONFIGURED.
 *
 * @return true Si le port USB CDC est opérationnel.
 * @return false Sinon.
 */
bool CDC_IsInitialized(void) {
	if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED) {
		return(true);
	} else {
		return(false);
	}
}


/**
 * @brief Récupère un caractère reçu via l'USB CDC.
 *
 * Cette fonction lit le buffer circulaire de réception et, s'il contient des données,
 * retourne par l'intermédiaire de *p_char* le prochain caractère disponible.
 *
 * @param[out] p_char Pointeur sur la variable où stocker le caractère lu.
 * @return true Un caractère a été récupéré.
 * @return false Aucun caractère n'est disponible.
 */
/**
 * @brief Récupère un caractère depuis le tampon CDC de manière non bloquante.
 *
 * Cette fonction vérifie immédiatement si un caractère est disponible dans le tampon de réception.
 * Si c'est le cas, l'octet est copié dans la variable pointée par @p p_char, l'indice du tampon est mis à jour,
 * et la fonction retourne true. Sinon, elle retourne false sans attendre.
 *
 * @param[out] p_char Pointeur vers la variable qui recevra l'octet lu.
 * @return bool true si un caractère a été lu, false sinon.
 */
bool CDC_GetChar(uint8_t *p_char)
{
    bool ret = false;

    /* Désactivation temporaire des interruptions pour garantir un accès atomique au tampon */
    __disable_irq();

    /* Si le tampon n'est pas vide (l'indice d'écriture diffère de l'indice de lecture) */
    if (fifo_is_empty(&cdc_fifo) == 0) {
        if (fifo_get(&cdc_fifo, p_char)) {
        }
    }	
    /* Réactivation des interruptions */
    __enable_irq();

    return ret;
}

/**
 * @brief Envoie une chaîne de caractères via l'USB CDC.
 *
 * Cette fonction utilise la fonction CDC_Transmit_FS() générée par STM32CubeMX pour
 * transmettre la chaîne *p_str* sur le port USB CDC.
 *
 * @param[in] p_str Pointeur sur la chaîne de caractères à envoyer (terminée par '\0').
 * @return true Si la transmission a été initiée avec succès.
 * @return false Sinon.
 */
bool CDC_SendString(const char *p_str) {
    uint16_t length = (uint16_t)strlen(p_str);

	while (CDC_Transmit_FS((uint8_t *)p_str, length) != USBD_OK) {
	}
	cdcTxComplete = true;
	return cdcTxComplete;
}

bool CDC_SendMem(const char *p_str, uint16_t length) {
	while (CDC_Transmit_FS((uint8_t *)p_str, length) != USBD_OK) {
	}
	cdcTxComplete = true;
	return cdcTxComplete;
}

	
//    if (CDC_Transmit_FS((uint8_t *)p_str, length) == USBD_OK) {
//		while(cdcTxComplete == false) {
//			/* Vous pouvez ajouter un timeout pour éviter un blocage infini */
//		}		
//		cdcTxComplete = true;
//        return cdcTxComplete;
//    }
    
//}

/**
 * @brief Callback de réception des données USB CDC.
 *
 * Cette fonction doit être appelée depuis CDC_Receive_FS() (déjà générée par CubeMX)
 * afin d'insérer les données reçues dans le buffer circulaire.
 *
 * @param[in] Buf Pointeur sur les données reçues.
 * @param[in] Len Nombre d'octets reçus.
 * @return int Retourne USBD_OK.
 */
int CDC_ReceiveCallback(uint8_t *Buf, uint32_t Len) {
    uint32_t i;
    for (i = 0U; i < Len; i++) {
		fifo_put(&cdc_fifo, Buf[i]);
    }

    /* La fonction CDC_Receive_FS() doit relancer la réception, c'est géré dans le fichier usbd_cdc_if.c */
    return USBD_OK;
}

/**
 * @brief Envoie un caractère via l'interface USB CDC.
 *
 * @param[in] ch Caractère à envoyer.
 */
void CDC_PutChar(uint8_t ch) {
    /* Pour envoyer un seul caractère, on l'envoie dans une chaîne de deux octets */
    char buf[2];
    buf[0] = (char)ch;
    buf[1] = '\0';
    CDC_SendString(buf);
}

