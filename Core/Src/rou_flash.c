#include "inc.h"

#define L_MAJEUR_VERSION 0L
#define L_MINEUR_VERSION 0L
#define L_RELEASE_VERSION 'a'


void Read_UniqueID(uint32_t *id) {
    id[0] = *(uint32_t *)0x1FFF7590;  // Lire le premier mot de l'ID
    id[1] = *(uint32_t *)0x1FFF7594;  // Lire le deuxième mot de l'ID
    id[2] = *(uint32_t *)0x1FFF7598;  // Lire le troisième mot de l'ID
}

void Read_Structure_From_Flash(uint32_t address, void *data, size_t size) {
    // Copier les données de la Flash vers la structure
    memcpy(data, (void *)address, size);
}

/**
 * @brief Lit la configuration stockée en Flash.
 *
 * Copie la structure AppConfig_t depuis l’adresse APP_CONFIG_ADDRESS.
 *
 * @param[out] config Pointeur sur une structure AppConfig_t dans laquelle la configuration sera copiée.
 */
void Config_Read(AppConfig_t * const config) {
	uint32_t unique_id[3];	
	AppConfig_t config_read_back;
//    const AppConfig_t * flash_config = (const AppConfig_t *)APP_CONFIG_ADDRESS;
    Read_Structure_From_Flash(flash_address_config, &config_read_back, sizeof(AppConfig_t));
	

	if (config_read_back.uniqueID0 == 0xFFFFFFFF) {
		Read_UniqueID(unique_id);	
		config_read_back.uniqueID0  = unique_id[0];
		config_read_back.uniqueID1  = unique_id[1];
		config_read_back.uniqueID2  = unique_id[2];
		config_read_back.CoefAnemo  = 1.1176f; 
		config_read_back.CoefPluvio = 0.2f;
		config_read_back.Temp_A =  1.0f;
		config_read_back.Temp_B = 0.0f;
		config_read_back.MAJEUR_VERSION  = L_MAJEUR_VERSION;
		config_read_back.MINEUR_VERSION  = L_MINEUR_VERSION;
		config_read_back.RELEASE_VERSION = L_RELEASE_VERSION;
		Write_Structure_To_Flash(flash_address_config, &config_read_back, sizeof(AppConfig_t));
	}
	(void)memcpy(config, &config_read_back, sizeof(AppConfig_t));

}

void Write_Structure_To_Flash(uint32_t address, void *data, size_t size) {
    // Débloquer la Flash
    HAL_FLASH_Unlock();

    // Effacer la page contenant l'adresse (optionnel si réécriture nécessaire)
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PageError = 0;

    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Banks = FLASH_BANK_1;
    EraseInitStruct.Page = (address - FLASH_BASE) / FLASH_PAGE_SIZE; // Page cible
    EraseInitStruct.NbPages = 1;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK) {
        // Gestion d'erreur
        HAL_FLASH_Lock();
        return;
    }

    // Écrire les données en blocs de 64 bits (8 octets)
    uint64_t *data_as_doubleword = (uint64_t *)data;
    size_t num_doublewords = (size + 7) / 8; // Arrondir au multiple de 8 octets

    for (size_t i = 0; i < num_doublewords; i++) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address + i * 8, data_as_doubleword[i]) != HAL_OK) {
            // Gestion d'erreur
            HAL_FLASH_Lock();
            return;
        }
    }

    // Verrouiller la Flash
    HAL_FLASH_Lock();
}

/**
 * @brief Efface une plage de mémoire Flash.
 *
 * @param[in] start_address Adresse de début.
 * @param[in] end_address   Adresse de fin.
 */
void Flash_Erase(uint32_t start_address, uint32_t end_address) {
    FLASH_EraseInitTypeDef erase_init;
    uint32_t page_error = 0U;
    uint32_t nb_pages;

    nb_pages = ((end_address - start_address) + 1U) / FLASH_PAGE_SIZE;
    erase_init.TypeErase   = FLASH_TYPEERASE_PAGES;
    erase_init.Page        = start_address;
    erase_init.NbPages     = nb_pages;

    if (HAL_FLASHEx_Erase(&erase_init, &page_error) != HAL_OK)
    {
        SendStringFTDI("Flash erase error.\r\n");

    }
}

/**
 * @brief Efface le secteur (page) de 2 ko dans la mémoire Flash.
 *
 * Cette fonction efface la page de flash correspondant à l'adresse donnée.
 * Le numéro de page est calculé à partir de l'adresse cible.
 *
 * @param[in] address Adresse située dans le secteur à effacer.
 *                    L'adresse doit être comprise entre FLASH_BASE_ADDRESS et la fin de la Flash.
 * @return int  0 en cas de succès, une valeur négative en cas d'erreur.
 */
static int flash_erase_sector(uint32_t address)
{
    HAL_StatusTypeDef status;
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SectorError = 0U;
    uint32_t page;

    /* Calcul du numéro de page à effacer */
    page = (address - FLASH_BASE_ADDRESS) / FLASH_PAGE_SIZE;

    /* Configuration de l'effacement */
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Page        = page;       /* Numéro de la page à effacer */
    EraseInitStruct.NbPages     = 1U;

    status = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
    if (status != HAL_OK)
    {
        return -1;
    }
    return 0;
}

/**
 * @brief Écrit des données en Flash pour le STM32G431.
 *
 * Cette fonction efface le secteur de 2 ko contenant l'adresse cible,
 * puis programme les données par double mot (64 bits, 8 octets) via HAL_FLASH_Program().
 * La zone flash doit être effacée au préalable ou être effacée par cette fonction.
 *
 * @param[in] address Adresse de début en flash (doit être alignée sur 8 octets et se trouver sur un secteur de 2 ko).
 * @param[in] data    Pointeur vers le buffer source contenant les données à écrire.
 * @param[in] length  Longueur des données à écrire en octets (doit être multiple de 8 et ≤ 2048).
 * @return int  0 en cas de succès, une valeur négative en cas d'erreur.
 */
int flash_write(uint32_t address, const uint8_t *data, uint32_t length)
{
    HAL_StatusTypeDef status;
    uint32_t addr = address;
    uint32_t end_addr = address + length;
    uint64_t dword;
    
    /* Effacer le secteur de 2 ko contenant l'adresse */
    if (flash_erase_sector(address) != 0)
    {
        return -1;
    }

    /* Déverrouillage de la Flash */
    status = HAL_FLASH_Unlock();
    if (status != HAL_OK)
    {
        return -2;
    }

    /* Programmation par double mot (8 octets à la fois) */
    while (addr < end_addr)
    {
        /* Copie de 8 octets depuis le buffer source dans une variable locale */
        (void)memcpy(&dword, data, sizeof(dword));

        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, dword);
        if (status != HAL_OK)
        {
            (void)HAL_FLASH_Lock();
            return -3;
        }
        addr += sizeof(dword);
        data += sizeof(dword);
    }

    /* Verrouillage de la Flash */
    (void)HAL_FLASH_Lock();

    return 0;
}

/**
 * @brief Lit des données depuis la Flash.
 *
 * Cette fonction copie les données à partir de la mémoire Flash vers un buffer en RAM.
 *
 * @param[in]  address Adresse de début en Flash.
 * @param[out] data    Pointeur vers le buffer de destination.
 * @param[in]  length  Nombre d'octets à lire.
 * @return int  0 en cas de succès, une valeur négative en cas d'erreur.
 */
int flash_read(uint32_t address, uint8_t *data, uint32_t length)
{
    /* La mémoire Flash est mappée en lecture, on effectue donc un memcpy */
    (void)memcpy(data, (const void *)address, length);
    return 0;
}
