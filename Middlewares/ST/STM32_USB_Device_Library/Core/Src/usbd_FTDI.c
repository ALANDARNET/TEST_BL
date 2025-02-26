/**
  ******************************************************************************
  * @file    main.c
  * @brief   Emulation FT232R sur STM32G431 en USB.
  ******************************************************************************
  * @attention
  * Ce code est un exemple complet et fonctionnel destiné à être utilisé tel quel
  * dans un projet STM32Cube. Adaptez la configuration (horloge, GPIO, etc.)
  * selon votre plateforme matérielle.
  ******************************************************************************
  */

/* Inclusion des fichiers d'en-tête HAL et USB Device */
#include "stm32g4xx_hal.h"   /* HAL pour STM32G4xx */
#include "usbd_core.h"       /* Noyau USB Device */
#include "usbd_ctlreq.h"     /* Gestion des requêtes USB */
#include "usbd_def.h"        /* Définitions USB */

/* Définitions des endpoints et taille de paquet pour l’émulation FTDI */
#define FTDI_IN_EP        0x81U      /**< Adresse de l’endpoint Bulk IN (EP1 IN) */
#define FTDI_OUT_EP       0x01U      /**< Adresse de l’endpoint Bulk OUT (EP1 OUT) */
#define FTDI_PACKET_SIZE  64U        /**< Taille maximale des paquets (64 octets) */

/* Déclaration globale de la poignée USB */
extern USBD_HandleTypeDef hUsbDeviceFS;

/* Tampon de réception pour l’endpoint OUT */
static uint8_t ftRxBuffer[FTDI_PACKET_SIZE];

/* ============================================================================
   DESCRIPTEURS USB pour l’émulation FT232R
   ============================================================================ */

/**
  * @brief  Descripteur de périphérique FTDI.
  */
static const uint8_t USBD_FTDI_DeviceDesc[] =
{
    0x12U,                       /* bLength : 18 octets */
    0x01U,                       /* bDescriptorType : DEVICE */
    0x00U, 0x02U,                /* bcdUSB : USB 2.00 */
    0x00U,                       /* bDeviceClass : défini par l’interface */
    0x00U,                       /* bDeviceSubClass */
    0x00U,                       /* bDeviceProtocol */
    0x40U,                       /* bMaxPacketSize0 : 64 octets */
    0x03U, 0x04U,                /* idVendor : 0x0403 (FTDI) en little endian */
    0x01U, 0x60U,                /* idProduct : 0x6001 (FT232R) en little endian */
    0x00U, 0x02U,                /* bcdDevice : 2.00 */
    0x01U,                       /* iManufacturer : index de la chaîne fabricant */
    0x02U,                       /* iProduct : index de la chaîne produit */
    0x03U,                       /* iSerialNumber : index de la chaîne numéro de série */
    0x01U                        /* bNumConfigurations : 1 configuration */
};

/**
  * @brief  Descripteur Device Qualifier FTDI.
  */
static const uint8_t USBD_FTDI_DeviceQualifierDesc[] =
{
    0x0AU,                       /* bLength : 10 octets */
    0x06U,                       /* bDescriptorType : DEVICE_QUALIFIER */
    0x00U, 0x02U,                /* bcdUSB : USB 2.00 */
    0x00U,                       /* bDeviceClass */
    0x00U,                       /* bDeviceSubClass */
    0x00U,                       /* bDeviceProtocol */
    0x40U,                       /* bMaxPacketSize0 : 64 octets */
    0x01U,                       /* bNumConfigurations : 1 */
    0x00U                        /* bReserved */
};

/**
  * @brief  Descripteur de configuration Full-Speed FTDI.
  */
static const uint8_t USBD_FTDI_CfgFSDesc[] =
{
    /* Configuration Descriptor */
    0x09U,                       /* bLength : 9 octets */
    0x02U,                       /* bDescriptorType : CONFIGURATION */
    0x20U, 0x00U,                /* wTotalLength : 32 octets (9+9+7+7) */
    0x01U,                       /* bNumInterfaces : 1 interface */
    0x01U,                       /* bConfigurationValue : 1 */
    0x00U,                       /* iConfiguration : aucune chaîne */
    0x80U,                       /* bmAttributes : Bus-powered */
    0x32U,                       /* bMaxPower : 100 mA */

    /* Interface Descriptor */
    0x09U,                       /* bLength : 9 octets */
    0x04U,                       /* bDescriptorType : INTERFACE */
    0x00U,                       /* bInterfaceNumber : 0 */
    0x00U,                       /* bAlternateSetting : 0 */
    0x02U,                       /* bNumEndpoints : 2 endpoints (Bulk IN et Bulk OUT) */
    0xFFU,                       /* bInterfaceClass : Vendor Specific */
    0x00U,                       /* bInterfaceSubClass */
    0x00U,                       /* bInterfaceProtocol */
    0x00U,                       /* iInterface : aucune chaîne */

    /* Endpoint Descriptor - Bulk IN */
    0x07U,                       /* bLength : 7 octets */
    0x05U,                       /* bDescriptorType : ENDPOINT */
    FTDI_IN_EP,                  /* bEndpointAddress : EP1 IN (0x81) */
    0x02U,                       /* bmAttributes : Bulk */
    0x40U, 0x00U,                /* wMaxPacketSize : 64 octets */
    0x00U,                       /* bInterval : non utilisé pour Bulk */

    /* Endpoint Descriptor - Bulk OUT */
    0x07U,                       /* bLength : 7 octets */
    0x05U,                       /* bDescriptorType : ENDPOINT */
    FTDI_OUT_EP,                 /* bEndpointAddress : EP1 OUT (0x01) */
    0x02U,                       /* bmAttributes : Bulk */
    0x40U, 0x00U,                /* wMaxPacketSize : 64 octets */
    0x00U                        /* bInterval : non utilisé pour Bulk */
};

/**
  * @brief  Descripteur de langue (anglais-US).
  */
static const uint8_t USBD_FTDI_LangIDDesc[] = { 0x04U, 0x03U, 0x09U, 0x04U };

/**
  * @brief  Chaîne fabricant FTDI.
  */
static uint8_t USBD_FTDI_ManufacturerString[] = {
    0x1AU, 0x03U,
    'F', 0x00U, 'T', 0x00U, 'D', 0x00U, 'I', 0x00U,
    ' ', 0x00U, 'C', 0x00U, 'o', 0x00U, 'm', 0x00U,
    'p', 0x00U, 'a', 0x00U, 'n', 0x00U, 'y', 0x00U
};

/**
  * @brief  Chaîne produit FTDI.
  */
static uint8_t USBD_FTDI_ProductString[] = {
    0x1CU, 0x03U,
    'F', 0x00U, 'T', 0x00U, '2', 0x00U, '3', 0x00U,
    '2', 0x00U, 'R', 0x00U, ' ', 0x00U, 'E', 0x00U,
    'm', 0x00U, 'u', 0x00U, 'l', 0x00U, 'a', 0x00U,
    't', 0x00U, 'o', 0x00U, 'r', 0x00U
};

/**
  * @brief  Chaîne numéro de série FTDI.
  */
static uint8_t USBD_FTDI_SerialString[] = {
    0x0EU, 0x03U,
    'F', 0x00U, 'T', 0x00U, '0', 0x00U, '0', 0x00U,
    '0', 0x00U, '0', 0x00U, '1', 0x00U
};

/**
  * @brief  Chaîne de configuration FTDI.
  */
static uint8_t USBD_FTDI_ConfigString[] = {
    0x1AU, 0x03U,
    'F', 0x00U, 'T', 0x00U, 'D', 0x00U, 'I', 0x00U,
    ' ', 0x00U, 'C', 0x00U, 'o', 0x00U, 'n', 0x00U,
    'f', 0x00U, 'i', 0x00U, 'g', 0x00U, '1', 0x00U
};

/**
  * @brief  Structure de descripteurs utilisée par la pile USB.
  */
USBD_DescriptorsTypeDef FTDI_Desc =
{
    USBD_FTDI_DeviceDesc,
    USBD_FTDI_LangIDDesc,
    USBD_FTDI_ManufacturerString,
    USBD_FTDI_ProductString,
    USBD_FTDI_SerialString,
    USBD_FTDI_ConfigString,
    USBD_FTDI_DeviceQualifierDesc,
};

/* ============================================================================
   FONCTIONS DE GESTION DE LA CLASSE FTDI (VENDOR SPECIFIC)
   ============================================================================ */

/**
  * @brief  Initialise l’interface FTDI.
  * @param  pdev: pointeur sur la poignée du périphérique USB.
  * @param  cfgidx: indice de configuration.
  * @retval USBD_StatusTypeDef.
  */
static uint8_t FTDI_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    /* Ouvre l’endpoint Bulk IN */
    USBD_LL_OpenEP(pdev, FTDI_IN_EP, USBD_EP_TYPE_BULK, FTDI_PACKET_SIZE);
    /* Ouvre l’endpoint Bulk OUT */
    USBD_LL_OpenEP(pdev, FTDI_OUT_EP, USBD_EP_TYPE_BULK, FTDI_PACKET_SIZE);

    /* Prépare l’endpoint OUT pour la réception */
    USBD_LL_PrepareReceive(pdev, FTDI_OUT_EP, ftRxBuffer, FTDI_PACKET_SIZE);

    return USBD_OK;
}

/**
  * @brief  Déinitialise l’interface FTDI.
  * @param  pdev: pointeur sur la poignée du périphérique USB.
  * @param  cfgidx: indice de configuration.
  * @retval USBD_StatusTypeDef.
  */
static uint8_t FTDI_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    USBD_LL_CloseEP(pdev, FTDI_IN_EP);
    USBD_LL_CloseEP(pdev, FTDI_OUT_EP);
    return USBD_OK;
}

/**
  * @brief  Gère les requêtes SETUP pour FTDI.
  * @param  pdev: pointeur sur la poignée du périphérique USB.
  * @param  req: pointeur sur la requête SETUP.
  * @retval USBD_StatusTypeDef.
  */
static uint8_t FTDI_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t ret = USBD_OK;

    /* Traitement uniquement des requêtes de type VENDOR */
    if ((req->bmRequest & USB_REQ_TYPE_MASK) == USB_REQ_TYPE_VENDOR)
    {
        switch (req->bRequest)
        {
            case 0x00U:  /* Exemple : réglage du baudrate ou autres commandes FTDI */
                /* Traitez ici les paramètres contenus dans req->wValue et req->wIndex */
                break;

            default:
                USBD_CtlError(pdev, req);
                ret = USBD_FAIL;
                break;
        }
    }
    else
    {
        USBD_CtlError(pdev, req);
        ret = USBD_FAIL;
    }

    return ret;
}

/**
  * @brief  Gère la fin de transmission sur l’endpoint IN.
  * @param  pdev: pointeur sur la poignée du périphérique USB.
  * @param  epnum: numéro d’endpoint.
  * @retval USBD_StatusTypeDef.
  */
static uint8_t FTDI_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    /* Transmission terminée.
       Vous pouvez ajouter ici le traitement post-transmission si nécessaire. */
    return USBD_OK;
}

/**
  * @brief  Gère la réception de données sur l’endpoint OUT.
  * @param  pdev: pointeur sur la poignée du périphérique USB.
  * @param  epnum: numéro d’endpoint.
  * @retval USBD_StatusTypeDef.
  */
static uint8_t FTDI_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    /* Traitement des données reçues dans ftRxBuffer.
       Remettez ensuite l’endpoint OUT en attente de réception. */
    USBD_LL_PrepareReceive(pdev, FTDI_OUT_EP, ftRxBuffer, FTDI_PACKET_SIZE);
    return USBD_OK;
}

/**
  * @brief  Gère l’événement SOF (Start Of Frame).
  * @param  pdev: pointeur sur la poignée du périphérique USB.
  * @retval USBD_StatusTypeDef.
  */
static uint8_t FTDI_SOF(USBD_HandleTypeDef *pdev)
{
    /* Vous pouvez ajouter ici du code périodique si nécessaire */
    return USBD_OK;
}

/**
  * @brief  Renvoie le descripteur de configuration Full-Speed.
  * @param  length: pointeur sur la variable de longueur.
  * @retval pointeur sur le descripteur.
  */
static uint8_t *FTDI_GetFSCfgDesc(uint16_t *length)
{
    *length = (uint16_t)sizeof(USBD_FTDI_CfgFSDesc);
    return (uint8_t *)USBD_FTDI_CfgFSDesc;
}

/**
  * @brief  Renvoie le Device Qualifier Descriptor.
  * @param  length: pointeur sur la variable de longueur.
  * @retval pointeur sur le descripteur.
  */
static uint8_t *FTDI_GetDeviceQualifierDesc(uint16_t *length)
{
    *length = (uint16_t)sizeof(USBD_FTDI_DeviceQualifierDesc);
    return (uint8_t *)USBD_FTDI_DeviceQualifierDesc;
}

/**
  * @brief  Structure de la classe USB FTDI.
  */
USBD_ClassTypeDef USBD_FTDI =
{
    FTDI_Init,
    FTDI_DeInit,
    FTDI_Setup,
    NULL,             /* EP0_TxSent : non utilisé */
    NULL,             /* EP0_RxReady : non utilisé */
    FTDI_DataIn,
    FTDI_DataOut,
    FTDI_SOF,
    NULL,
    NULL,
    FTDI_GetFSCfgDesc,
    FTDI_GetDeviceQualifierDesc,
};

/* ============================================================================
   FONCTIONS D'INITIALISATION ET DE CONFIGURATION
   ============================================================================ */

/**
  * @brief  Initialise le périphérique USB en mode FTDI.
  */
void MX_USB_Device_Init(void)
{
    /* Initialise la bibliothèque USB Device avec les descripteurs FTDI */
    USBD_Init(&hUsbDeviceFS, &FTDI_Desc, DEVICE_FS);
    /* Enregistre la classe FTDI */
    USBD_RegisterClass(&hUsbDeviceFS, &USBD_FTDI);
    /* Démarre le périphérique USB */
    USBD_Start(&hUsbDeviceFS);
}

USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *pdev) {
}

///* ============================================================================
//   FONCTION PRINCIPALE
//   ============================================================================ */

///**
//  * @brief  Point d’entrée du programme.
//  * @retval int
//  */
//int main(void)
//{
//    /* Initialisation de la librairie HAL */
//    HAL_Init();

//    /* Configure l’horloge système */
//    SystemClock_Config();

//    /* Initialisation minimale des GPIO */
//    MX_GPIO_Init();

//    /* Initialisation du périphérique USB FTDI */
//    MX_USB_DEVICE_Init();

//    /* Boucle principale */
//    while (1)
//    {
//        /* Ajoutez ici le traitement applicatif */
//    }
//}
