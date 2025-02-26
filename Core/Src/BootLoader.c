/**
 * @file BootLoader.c
 * @brief Code principal du bootloader sur STM32G431KB.
 *
 * Ce module implémente un menu interactif en VT100 pour le bootloader.
 * L'affichage se fait à des positions absolues (définies par des macros),
 * et le menu propose plusieurs options (navigation via flèches, validation par ENTRÉE).
 *
 * Ce fichier utilise les fonctions CDC_SendString, CDC_GetChar,
 * Config_Read, xmodem_receive_1k_blockwise, flash_write_callback,
 * ainsi que la gestion d'un FIFO (fifo_t, fifo_get, etc.),
 * qui doivent être déclarées dans "inc.h".
 */

#include "inc.h"          /* Déclarations de CDC_SendString, CDC_GetChar, Config_Read, etc. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>      /* Pour le type bool */

/* ------------------------------------------------------------------------- */
/*                            Constantes et macros                           */
/* ------------------------------------------------------------------------- */

/* Taille maximale pour les buffers d'affichage */
#define BUFFER_SIZE       (256U)

/* Nombre total d'options du menu */
#define MENU_OPTIONS      (8U)  /* Augmenté de 7 à 8 */

/* --- Définition des numéros de ligne pour l'affichage VT100 --- */
/* Pour éviter les chevauchements, on définit des plages distinctes : */
/* L'ASCII art sera affiché sur les lignes 1 à 5, */
/* L'en-tête et le menu à partir de la ligne 7. */
#define ASCII_ART_LINE_1_NUMBER   1
#define ASCII_ART_LINE_2_NUMBER   2
#define ASCII_ART_LINE_3_NUMBER   3
#define ASCII_ART_LINE_4_NUMBER   4
#define ASCII_ART_LINE_5_NUMBER   5

#define HEADER_LINE_1_NUMBER      7    /**< Ligne pour l'affichage de l'ID */
#define HEADER_LINE_2_NUMBER      8    /**< Ligne pour l'affichage de la version */
#define HEADER_LINE_3_NUMBER      9    /**< Ligne vide sous l'en-tête */
#define MENU_INFO_LINE_NUMBER     10   /**< Ligne pour les instructions du menu */
#define MENU_START_LINE_NUMBER    11   /**< Ligne de départ pour les options du menu */
#define INPUT_PROMPT_LINE_NUMBER  20   /**< Ligne d'affichage de l'invite de saisie */
#define INPUT_LINE_NUMBER         21   /**< Ligne d'affichage de la saisie utilisateur */
#define MENU_EXIT_LINE_NUMBER     23   /**< Ligne d'affichage de la sortie du menu */

/* --- Macros pour convertir un nombre en chaîne --- */
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

/* --- Commandes VT100 pour l'ASCII art --- */
#define VT100_ASCII_LINE_1 "\033[" STR(ASCII_ART_LINE_1_NUMBER) ";1H"
#define VT100_ASCII_LINE_2 "\033[" STR(ASCII_ART_LINE_2_NUMBER) ";1H"
#define VT100_ASCII_LINE_3 "\033[" STR(ASCII_ART_LINE_3_NUMBER) ";1H"
#define VT100_ASCII_LINE_4 "\033[" STR(ASCII_ART_LINE_4_NUMBER) ";1H"
#define VT100_ASCII_LINE_5 "\033[" STR(ASCII_ART_LINE_5_NUMBER) ";1H"

/* --- Commandes VT100 pour l'en-tête et le menu --- */
#define VT100_CURSOR_HOME       "\033[H"
#define VT100_HEADER_LINE_1     "\033[" STR(HEADER_LINE_1_NUMBER) ";1H"
#define VT100_HEADER_LINE_2     "\033[" STR(HEADER_LINE_2_NUMBER) ";1H"
#define VT100_HEADER_LINE_3     "\033[" STR(HEADER_LINE_3_NUMBER) ";1H"
#define VT100_MENU_INFO_LINE    "\033[" STR(MENU_INFO_LINE_NUMBER) ";1H"
#define VT100_INPUT_PROMPT_LINE "\033[" STR(INPUT_PROMPT_LINE_NUMBER) ";1H"
#define VT100_INPUT_LINE        "\033[" STR(INPUT_LINE_NUMBER) ";1H"
#define VT100_MENU_EXIT_LINE    "\033[" STR(MENU_EXIT_LINE_NUMBER) ";1H"

/* --- Autres commandes VT100 --- */
#define VT100_GRAY              "\033[37m"
#define VT100_RESET             "\033[0m"
#define VT100_CURSOR_SHOW       "\033[?25h"
#define VT100_CURSOR_HIDE       "\033[?25l"
#define VT100_CLEAR_LINE        "\033[K"

/* --- Macro pour concaténer deux chaînes littérales --- */
#define CONCAT_STRINGS(a, b) a b

/* Macros pour effacer la ligne d'invite et la ligne de saisie */
#define VT100_INPUT_CLEAR   CONCAT_STRINGS(VT100_INPUT_LINE, VT100_CLEAR_LINE)
#define VT100_PROMPT_CLEAR  CONCAT_STRINGS(VT100_INPUT_PROMPT_LINE, VT100_CLEAR_LINE)

/* ------------------------------------------------------------------------- */
/*                Déclarations de variables et types locaux                */
/* ------------------------------------------------------------------------- */

/* Variable globale indiquant si un firmware valide est présent.
 * Utilisée dans plusieurs fonctions du bootloader. */
static bool firmware_ok = false;

/* Définition d'un type fonction pour le saut vers l'application.
 * pFunction est un pointeur vers une fonction ne prenant aucun paramètre et ne retournant rien. */
typedef void (*pFunction)(void);

/* ------------------------------------------------------------------------- */
/*              Tableaux de chaînes pour l'affichage VT100 (ASCII art)       */
/* ------------------------------------------------------------------------- */

/**
 * @brief ASCII art affiché en haut de l'écran.
 *
 * Chaque ligne commence par une commande VT100 positionnant le curseur
 * à la ligne correspondante (définie par VT100_ASCII_LINE_x).
 */
const char* ascii_art[] = {
    VT100_ASCII_LINE_1 "__     _______ _   _    _  _____ _   _ _____ ____ \n\r",
    VT100_ASCII_LINE_2 "\\ \\   / / ____| \\ | |  / \\|_   _| | | | ____/ ___|\n\r",
    VT100_ASCII_LINE_3 " \\ \\ / /|  _| |  \\| | / _ \\ | | | |_| |  _|| |    \n\r",
    VT100_ASCII_LINE_4 "  \\ V / | |___| |\\  |/ ___ \\| | |  _  | |__| |___ \n\r",
    VT100_ASCII_LINE_5 "   \\_/  |_____|_| \\_/_/   \\_\\_| |_| |_|_____\\____|\n\r"
};

/**
 * @brief Tableau constant des options du menu.
 *
 * L'ordre des options est désormais :
 *   0 : Modifier CoefAnemo  
 *   1 : Modifier CoefPluvio  
 *   2 : Modifier Temp_A  
 *   3 : Modifier Temp_B  
 *   4 : Lecture Anémomètre      <-- Nouvelle option  
 *   5 : Température Actuelle  
 *   6 : Mise à jour firmware (XMODEM 1K)  
 *   7 : Lancer à l'application
 */
const char * const menu_items[MENU_OPTIONS] = {
    "Modifier CoefAnemo",
    "Modifier CoefPluvio",
    "Modifier Temp_A",
    "Modifier Temp_B",
    "Lecture Anémomètre",
    "Température Actuelle",
    "Mise à jour firmware (XMODEM 1K)",
    "Lancer à l'application"
};

/* ------------------------------------------------------------------------- */
/*                        Fonctions locales (statiques)                     */
/* ------------------------------------------------------------------------- */

/**
 * @brief Vérifie si un firmware valide est présent.
 *
 * La fonction compare la valeur "Presence" dans la structure appInfoRAM à 0x12345678.
 *
 * @return true  si un firmware valide est présent.
 * @return false sinon.
 */
static bool FirmwarePresent(void) {
    if (v_AppConfig.Presence == 0x12345678) {
        return true;
    }
    return false;
}

/**
 * @brief Affiche le menu de sélection à des positions absolues.
 *
 * La fonction affiche :
 * - L'ASCII art (les lignes VT100_ASCII_LINE_x sont intégrées dans ascii_art),
 * - Les informations d'en-tête (ID et Version),
 * - Une ligne vide,
 * - Les instructions du menu,
 * - Puis chaque option du menu à partir de MENU_START_LINE_NUMBER.
 *
 * Pour les options 0 à 3 et l'option 5 (Température Actuelle), des valeurs actuelles sont affichées.
 * Les autres options s'affichent simplement.
 *
 * @param[in] pConfig    Pointeur vers la structure de configuration.
 * @param[in] menu_index Indice de l'option actuellement sélectionnée.
 */
static void Bootloader_DisplayMenu(const AppConfig_t *pConfig, uint8_t menu_index) {
    char buffer[BUFFER_SIZE];
    uint8_t i;
    
    /* Actualise la variable firmware_ok selon la présence d'un firmware valide */
    firmware_ok = FirmwarePresent();

    /* Affichage de l'ASCII art */
    SendStringFTDI((char*)ascii_art[0]);
    SendStringFTDI((char*)ascii_art[1]);
    SendStringFTDI((char*)ascii_art[2]);
    SendStringFTDI((char*)ascii_art[3]);
    SendStringFTDI((char*)ascii_art[4]);

    /* Affichage de l'en-tête aux positions absolues */
    (void)snprintf(buffer, BUFFER_SIZE, VT100_HEADER_LINE_1 "UNIQUE ID ADAMO : %08X%08X%08X    Tension USB : %.2f",
                   pConfig->uniqueID0, pConfig->uniqueID1, pConfig->uniqueID2, v_ADC1_IN10);
    SendStringFTDI(buffer);

    if (firmware_ok == false) {
        (void)snprintf(buffer, BUFFER_SIZE, VT100_HEADER_LINE_2 "Version : %u.%02u%c (Firmware non trouvé)",
                       pConfig->MAJEUR_VERSION, pConfig->MINEUR_VERSION, (char)pConfig->RELEASE_VERSION);
    } else {
        (void)snprintf(buffer, BUFFER_SIZE, VT100_HEADER_LINE_2 "Version : %s, %s, %s",
                       v_AppConfig.Version_Compile, v_AppConfig.Date_Compile, v_AppConfig.Heure_Compile);
    }
    SendStringFTDI(buffer);

    SendStringFTDI(VT100_HEADER_LINE_3);
    SendStringFTDI(VT100_MENU_INFO_LINE "Utilisez les flèches Haut/Bas pour naviguer et ENTRÉE pour sélectionner");

    /* Affichage des options du menu, une par ligne à partir de MENU_START_LINE_NUMBER */
    for (i = 0U; i < MENU_OPTIONS; i++)
    {
        (void)snprintf(buffer, BUFFER_SIZE, "\033[%u;1H", (unsigned int)(MENU_START_LINE_NUMBER + i));
        SendStringFTDI(buffer);

        /* Pour "Lancer à l'application" en absence de firmware, affiche en gris */
        firmware_ok = FirmwarePresent();
        if ((i == MENU_OPTIONS - 1U) && (firmware_ok == false))
        {
            SendStringFTDI(VT100_GRAY);
            (void)snprintf(buffer, BUFFER_SIZE, "%s", menu_items[i]);
            SendStringFTDI(buffer);
            SendStringFTDI(VT100_RESET);
        }
        else
        {
            if (i == menu_index)
            {
                SendStringFTDI("\033[7m");
            }
            if ((i == 0U) || (i == 1U) || (i == 2U) || (i == 3U))
            {
                switch (i)
                {
                    case 0U:
                        (void)snprintf(buffer, BUFFER_SIZE, "%s (actuel : %.4f)", menu_items[i], pConfig->CoefAnemo);
                        break;
                    case 1U:
                        (void)snprintf(buffer, BUFFER_SIZE, "%s (actuel : %.4f)", menu_items[i], pConfig->CoefPluvio);
                        break;
                    case 2U:
                        (void)snprintf(buffer, BUFFER_SIZE, "%s (actuel : %.4f)", menu_items[i], pConfig->Temp_A);
                        break;
                    case 3U:
                        (void)snprintf(buffer, BUFFER_SIZE, "%s (actuel : %.4f)", menu_items[i], pConfig->Temp_B);
                        break;
                    default:
                        break;
                }
            }
            else if (i == 5U)
            {
                /* Pour "Température Actuelle", calcul et affichage de la température */
				v_temperature_mesuree = TMP1075_ReadTemperature();

                v_config_system.Temperature = (v_temperature_mesuree * v_config_system.Temp_A) + v_config_system.Temp_B;
                (void)snprintf(buffer, BUFFER_SIZE, "%s (Mesurée : %2.1f , Calculée : %2.1f)",
                               menu_items[i], v_temperature_mesuree, v_config_system.Temperature);
            }
            else
            {
                /* Pour "Lecture Anémomètre" et "Mise à jour firmware (XMODEM 1K)" */
                (void)snprintf(buffer, BUFFER_SIZE, "%s", menu_items[i]);
            }
            SendStringFTDI(buffer);
            if (i == menu_index)
            {
                SendStringFTDI(VT100_RESET);
            }
        }
    }
    SendStringFTDI(VT100_CURSOR_HIDE);
}

///**
// * @brief Lit un caractère depuis le FIFO de réception CDC.
// *
// * @return uint8_t Le caractère lu, ou 0 s'il n'y a aucun caractère disponible.
// */
//static uint8_t Bootloader_GetInputChar(void)
//{
//    uint8_t ch = 0U;
//    if (fifo_get(&usart2_fifo, &ch))
//    {
//        return ch;
//    }
//    return 0U;
//}

/**
 * @brief Affiche une invite et récupère une valeur float saisie par l'utilisateur.
 *
 * Affiche l'invite sur la ligne définie par INPUT_PROMPT_LINE_NUMBER,
 * affiche le curseur, efface la ligne de saisie (INPUT_LINE_NUMBER), lit les caractères
 * saisis jusqu'à ce que l'utilisateur appuie sur ENTRÉE, convertit la chaîne en float,
 * puis efface les lignes d'invite et de saisie.
 *
 * @param[in]  pPrompt Chaîne d'invite à afficher.
 * @param[out] pValue  Pointeur vers la variable float qui recevra la valeur saisie.
 */
static void Bootloader_GetInputFloat(const char *pPrompt, float *pValue)
{
    char input_buffer[BUFFER_SIZE];
    size_t i = 0U;
    uint8_t ch;

    (void)snprintf(input_buffer, BUFFER_SIZE, VT100_INPUT_PROMPT_LINE "%s", pPrompt);
    SendStringFTDI(input_buffer);
    SendStringFTDI(VT100_CURSOR_SHOW);
    SendStringFTDI(VT100_INPUT_CLEAR);

    memset(input_buffer, 0, sizeof(input_buffer));
    while (i < (BUFFER_SIZE - 1U))
    {
//        ch = Bootloader_GetInputChar();
		while(fifo_is_empy(&usart2_fifo)) {};
//		fifo_wait_for(&usart2_fifo, 1, 100);
		fifo_get(&usart2_fifo, &ch);
		
        if ((ch == '\r') || (ch == '\n'))
        {
            break;
        }
        else if (ch != 0U)
        {
            input_buffer[i] = (char)ch;
            i++;
            {
                char echo[2] = {(char)ch, '\0'};
                SendStringFTDI(echo);
            }
        }
        HAL_Delay(10U);
    }
    input_buffer[i] = '\0';
    SendStringFTDI("\r\n");

    *pValue = (float)atof(input_buffer);

    SendStringFTDI(VT100_PROMPT_CLEAR);
    SendStringFTDI(VT100_INPUT_CLEAR);
    SendStringFTDI(VT100_CURSOR_HIDE);
}

/**
 * @brief Affiche le menu interactif du bootloader et gère la navigation.
 *
 * Le menu affiche l'ASCII art, l'en-tête et les options.
 * La navigation se fait via les flèches Haut/Bas et la validation par ENTRÉE.
 *
 * Pour éviter le clignotement, aucune commande VT100_CLEAR_SCREEN n'est utilisée lors des rafraîchissements.
 * Seul le repositionnement du curseur (VT100_CURSOR_HOME) est effectué pour mettre à jour l'affichage.
 */
void Bootloader_Menu(void)
{
    uint8_t key;
    uint8_t menu_index = 0U;
    char buffer[BUFFER_SIZE];
    uint8_t seq1, seq2;
    uint8_t tmp_char;
    uint8_t selectable_options;

    (void)Config_Read(&v_config_system);
    firmware_ok = FirmwarePresent();
    selectable_options = firmware_ok ? MENU_OPTIONS : (MENU_OPTIONS - 1U);

    /* Rafraîchissement initial sans effacer l'écran complet */
    SendStringFTDI(VT100_CURSOR_HOME);
    Bootloader_DisplayMenu(&v_config_system, menu_index);

    while (1) {
//        fifo_wait_for(&usart2_fifo, 1, 100);
		while(fifo_is_empy(&usart2_fifo)) {};
		
        fifo_get(&usart2_fifo, &key);
        if (key == 0x1B) { /* Séquence d'échappement VT100 (flèches) */ 
            /* Mise à jour sans effacer tout l'écran */
            SendStringFTDI(VT100_CURSOR_HOME);
            Bootloader_DisplayMenu(&v_config_system, menu_index);
			while(fifo_is_empy(&usart2_fifo)) {};
			fifo_get(&usart2_fifo, &seq1);
			while(fifo_is_empy(&usart2_fifo)) {};
			fifo_get(&usart2_fifo, &seq2);
            if (seq1 == '[') {
                if (seq2 == 'A') {
                    menu_index = (menu_index == 0U) ? selectable_options - 1U : menu_index - 1U;
                } else if (seq2 == 'B') {
                    menu_index = (menu_index + 1U) % selectable_options;
                }
                SendStringFTDI(VT100_CURSOR_HOME);
                Bootloader_DisplayMenu(&v_config_system, menu_index);
            }
        } else if ((key == '\r') || (key == '\n')) {
            SendStringFTDI(VT100_CURSOR_HOME);
            Bootloader_DisplayMenu(&v_config_system, menu_index);
            switch (menu_index) {
                case 0U:
                    Bootloader_GetInputFloat("Entrez la nouvelle valeur pour CoefAnemo : ", &v_config_system.CoefAnemo);
                    if ((v_config_system.CoefAnemo <= 0.0F) || (v_config_system.CoefAnemo >= 5.0F))
                    {
                        (void)snprintf(buffer, BUFFER_SIZE,
                                         VT100_INPUT_LINE "Valeur invalide pour CoefAnemo. Doit être > 0.0 et < 5.0\r\n");
                        SendStringFTDI(buffer);
                    }
                    else
                    {
						Write_Structure_To_Flash(flash_address_config, &v_config_system, sizeof(AppConfig_t));
                        SendStringFTDI(VT100_INPUT_LINE "CoefAnemo mis à jour.\r\n");
                    }
                    break;
                case 1U:
                    Bootloader_GetInputFloat("Entrez la nouvelle valeur pour CoefPluvio : ", &v_config_system.CoefPluvio);
                    if ((v_config_system.CoefPluvio <= 0.0F) || (v_config_system.CoefPluvio >= 2.0F))
                    {
                        SendStringFTDI(VT100_INPUT_LINE "Valeur invalide pour CoefPluvio. Doit être > 0.0 et < 2.0\r\n");
                    }
                    else
                    {
						Write_Structure_To_Flash(flash_address_config, &v_config_system, sizeof(AppConfig_t));
                        SendStringFTDI(VT100_INPUT_LINE "CoefPluvio mis à jour.\r\n");
                    }
                    break;
                case 2U:
                    Bootloader_GetInputFloat("Entrez la nouvelle valeur pour Temp_A : ", &v_config_system.Temp_A);
					Write_Structure_To_Flash(flash_address_config, &v_config_system, sizeof(AppConfig_t));
			
                break;
                case 3U:
                    Bootloader_GetInputFloat("Entrez la nouvelle valeur pour Temp_B : ", &v_config_system.Temp_B);
					Write_Structure_To_Flash(flash_address_config, &v_config_system, sizeof(AppConfig_t));
                break;
                case 4U: {
                    /* Nouvelle option : Lecture Anémomètre
                     * Affiche toutes les secondes "Vitesse Vent : %.1f m/s" sur la ligne d'invite.
                     * La boucle se termine dès qu'un caractère est détecté.
                     */
                    SendStringFTDI(VT100_PROMPT_CLEAR);
                    while (1) {
						uint8_t ch;
                        char msg[BUFFER_SIZE];
                        (void)snprintf(msg, BUFFER_SIZE, "Vitesse Vent : %.1f m/s", v_vitesse_vent);
                        SendStringFTDI(VT100_INPUT_PROMPT_LINE);
                        SendStringFTDI(msg);
                        HAL_Delay(1000);
						while(fifo_is_empy(&usart2_fifo)) {};
						fifo_get(&usart2_fifo, &ch);
								
                        if (ch != 0)
                        {
                            break;
                        }
                        SendStringFTDI(VT100_PROMPT_CLEAR);
                    }
                    break;
                }
                case 5U:
//                    /* Option "Température Actuelle" */
//					v_temperature_mesuree = TMP1075_ReadTemperature();
//                    v_config_system.Temperature = (v_temperature_mesuree * v_config_system.Temp_A) + v_config_system.Temp_B;
//                    (void)snprintf(buffer, BUFFER_SIZE,
//                                   VT100_INPUT_LINE "Température Capteur : %.1f\r\nTempérature calculée : %.1f\r\n",
//                                   v_temperature_mesuree, v_config_system.Temperature);
//                    SendStringFTDI(buffer);
//                    SendStringFTDI(VT100_INPUT_LINE "Appuyez sur ENTRÉE pour continuer...");
//                    do {
//						while(fifo_is_empy(&usart2_fifo)) {};
//						fifo_get(&usart2_fifo, &tmp_char);
////                        HAL_Delay(10U);
//                    } while ((tmp_char != '\r') && (tmp_char != '\n'));
                break;
                case 6U:
                {
                    /* Option "Mise à jour firmware (XMODEM 1K)" */
                    xmodem_receive_1k_blockwise(&usart2_fifo, flash_write_callback);
                    do {
//                        tmp_char = Bootloader_GetInputChar();
						while(fifo_is_empy(&usart2_fifo)) {};
						fifo_get(&usart2_fifo, &tmp_char);
//	                    HAL_Delay(10U);
                    } while ((tmp_char != '\r') && (tmp_char != '\n'));
                    SendStringFTDI(VT100_INPUT_CLEAR);
                    SendStringFTDI(VT100_PROMPT_CLEAR);
                } break;
                case 7U:
                    /* Option "Lancer à l'application" */
                    if (firmware_ok)
                    {
                        SendStringFTDI(VT100_INPUT_LINE "Passage à l'application...\r\n");
                        SendStringFTDI(VT100_MENU_EXIT_LINE "Sortie du menu du bootloader...\r\n");
                        Bootloader_JumpToApplication();
                    }
                    else
                    {
                        SendStringFTDI(VT100_INPUT_LINE "Firmware non présent. Option indisponible.\r\n");
                        do {
//                            tmp_char = Bootloader_GetInputChar();
							while(fifo_is_empy(&usart2_fifo)) {};
							fifo_get(&usart2_fifo, &tmp_char);
//							HAL_Delay(10U);
                        } while ((tmp_char != '\r') && (tmp_char != '\n'));
                    }
                    break;
                default:
                    break;
            }
        }
        HAL_Delay(10U);
    }
}


#define APP_ADDR	0x08010000		// my MCU app code base address
#define	MCU_IRQS	70u				// no. of NVIC IRQ inputs

struct app_vectable_ {
    uint32_t Initial_SP;
    void (*Reset_Handler)(void);
};

#define APPVTAB	((struct app_vectable_ *)APP_ADDR)

/**
 * @brief Effectue le saut vers l'application utilisateur.
 *
 * La fonction vérifie que l'adresse de la pile (premier mot) est valide,
 * désactive les interruptions, initialise le pointeur de pile, récupère l'adresse
 * de réinitialisation (deuxième mot), puis effectue le saut vers l'application.
 */
#define	MCU_IRQS	70u	// no. of NVIC IRQ inputs
void Bootloader_JumpToApplication(void) {
//    uint32_t JumpAddress;
//    pFunction Jump_To_Application;
//	void (*app_reset_handler)(void) = (void*)(*((volatile uint32_t*) (APPLICATION_ADDRESS + 4U)));	
//    uint32_t Jump_To_Application = *(__IO uint32_t*)(0x8010000 + 4);
    memcpy(v_AppConfig.Boot_Bootloader, "TOOB",4);
	Write_Structure_To_Flash(flash_address, &v_AppConfig, sizeof(AppConfig_t));	
    if(((*(__IO uint32_t *)APPLICATION_ADDRESS) & 0x2FFE0000) == 0x20000000) {
        __disable_irq();
		RCC->CIER = 0x00000000; // Disable all interrupts related to clock
		HAL_RCC_DeInit();
		HAL_DeInit();
		// Reset the SysTick Timer
		SysTick->CTRL = 0;
		SysTick->LOAD = 0;
		SysTick->VAL =0;
		/* Clear Interrupt Enable Register & Interrupt Pending Register */
		for (uint8_t i = 0; i < (MCU_IRQS + 31u) / 32; i++)
		{
			NVIC->ICER[i]=0xFFFFFFFF;
			NVIC->ICPR[i]=0xFFFFFFFF;
		}
	/* Re-enable all interrupts */
	
	__enable_irq();
	// Set the MSP
	__set_MSP(APPVTAB->Initial_SP);
	// Jump to app firmware
	APPVTAB->Reset_Handler();		

    }
}


#include "stm32g4xx_hal.h"

#include "stm32g4xx_hal.h"

void HAL_MspDeInit(void) {
    /* Désactiver les interruptions globales */
    __disable_irq();

    /* Désactiver le SysTick */
    HAL_SuspendTick();

    /* Désactiver tous les GPIOs et remettre en état High-Z (entrée flottante) */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    /* Désactiver toutes les horloges des périphériques */
    __HAL_RCC_GPIOA_CLK_DISABLE();
    __HAL_RCC_GPIOB_CLK_DISABLE();
    __HAL_RCC_GPIOC_CLK_DISABLE();
    __HAL_RCC_GPIOD_CLK_DISABLE();
    __HAL_RCC_GPIOE_CLK_DISABLE();
    __HAL_RCC_GPIOF_CLK_DISABLE();
    __HAL_RCC_GPIOG_CLK_DISABLE();

    __HAL_RCC_USART1_CLK_DISABLE();
    __HAL_RCC_USART2_CLK_DISABLE();
    __HAL_RCC_USART3_CLK_DISABLE();
    __HAL_RCC_UART4_CLK_DISABLE();

    __HAL_RCC_SPI1_CLK_DISABLE();
    __HAL_RCC_SPI2_CLK_DISABLE();
    __HAL_RCC_SPI3_CLK_DISABLE();
    __HAL_RCC_I2C1_CLK_DISABLE();
    __HAL_RCC_I2C2_CLK_DISABLE();
    __HAL_RCC_I2C3_CLK_DISABLE();
    __HAL_RCC_TIM1_CLK_DISABLE();
    __HAL_RCC_TIM2_CLK_DISABLE();
    __HAL_RCC_TIM3_CLK_DISABLE();
    __HAL_RCC_TIM4_CLK_DISABLE();

    __HAL_RCC_TIM6_CLK_DISABLE();
    __HAL_RCC_TIM7_CLK_DISABLE();
    __HAL_RCC_TIM8_CLK_DISABLE();
    __HAL_RCC_TIM15_CLK_DISABLE();
    __HAL_RCC_TIM16_CLK_DISABLE();
    __HAL_RCC_TIM17_CLK_DISABLE();

    __HAL_RCC_DAC1_CLK_DISABLE();

    __HAL_RCC_FDCAN_CLK_DISABLE();
    __HAL_RCC_USB_CLK_DISABLE();
    __HAL_RCC_RNG_CLK_DISABLE();
    __HAL_RCC_CRC_CLK_DISABLE();
    __HAL_RCC_DMA1_CLK_DISABLE();
    __HAL_RCC_DMA2_CLK_DISABLE();

    /* Remettre les timers à l'état initial */
    __HAL_RCC_TIM1_FORCE_RESET();
    __HAL_RCC_TIM2_FORCE_RESET();
    __HAL_RCC_TIM3_FORCE_RESET();
    __HAL_RCC_TIM4_FORCE_RESET();

    __HAL_RCC_TIM6_FORCE_RESET();
    __HAL_RCC_TIM7_FORCE_RESET();
    __HAL_RCC_TIM8_FORCE_RESET();
    __HAL_RCC_TIM15_FORCE_RESET();
    __HAL_RCC_TIM16_FORCE_RESET();
    __HAL_RCC_TIM17_FORCE_RESET();

    __HAL_RCC_TIM1_RELEASE_RESET();
    __HAL_RCC_TIM2_RELEASE_RESET();
    __HAL_RCC_TIM3_RELEASE_RESET();
    __HAL_RCC_TIM4_RELEASE_RESET();
    
    __HAL_RCC_TIM6_RELEASE_RESET();
    __HAL_RCC_TIM7_RELEASE_RESET();
    __HAL_RCC_TIM8_RELEASE_RESET();
    __HAL_RCC_TIM15_RELEASE_RESET();
    __HAL_RCC_TIM16_RELEASE_RESET();
    __HAL_RCC_TIM17_RELEASE_RESET();

    /* Réinitialiser le bus APB et AHB (simule un reset du bus) */
    __HAL_RCC_AHB1_FORCE_RESET();
    __HAL_RCC_AHB1_RELEASE_RESET();
    __HAL_RCC_APB1_FORCE_RESET();
    __HAL_RCC_APB1_RELEASE_RESET();
    __HAL_RCC_APB2_FORCE_RESET();
    __HAL_RCC_APB2_RELEASE_RESET();

    /* Désactiver toutes les interruptions des périphériques */
    for (uint8_t i = 0; i < 85; i++) {
        HAL_NVIC_DisableIRQ(i);
    }

    /* Réinitialiser le système d'horloge à HSI (oscillateur interne) */
    RCC->CR |= RCC_CR_HSION;  // Activer HSI
    while (!(RCC->CR & RCC_CR_HSIRDY));  // Attendre que HSI soit stable

    RCC->CFGR = 0x00000000;   // Remettre la config d'horloge par défaut

    /* Réactiver les interruptions globales */
    __enable_irq();
}
