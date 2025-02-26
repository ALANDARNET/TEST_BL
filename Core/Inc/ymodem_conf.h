#define DEVICE_FAMILY_STM32G4

/**
 * @brief	Whether to check programming was successful
 * 
 */
//#define YMODEM_VALIDATE_PROGRAMMING	


/**
 * @brief	Size of flash to save file to  
 * 
 */
#define YMODEM_FLASH_SIZE				(64 * 1024) // 64kB

/**
 * @brief  Starting address of flash
 * 
 */
#define YMODEM_FLASH_START				0x08010000

/**
 * @brief  Flash sector number
 * 
 */
#define YMODEM_FLASH_FIRST_SECTOR_NUM			(8) 

/**
 * @brief  Num of flash sectors 
 * 
 */
#define YMODEM_FLASH_NUM_OF_SECTORS		(1)
