#ifndef __BL24CXX_H
#define __BL24CXX_H

#include "at32f4xx.h"
 
typedef enum 
{
  I2C_OK             = 0,
  
  I2C_ERROR_STEP_1   = 1,
  I2C_ERROR_STEP_2   = 2,
  I2C_ERROR_STEP_3   = 3,
  I2C_ERROR_STEP_4   = 4,
  I2C_ERROR_STEP_5   = 5,
  I2C_ERROR_STEP_6   = 6,
  I2C_ERROR_STEP_7   = 7,
  I2C_ERROR_STEP_8   = 8,  
  I2C_ERROR_STEP_9   = 9,
  I2C_ERROR_STEP_10  = 10,
  I2C_ERROR_STEP_11  = 11,
  I2C_ERROR_STEP_12  = 12,
  
  I2C_BUSY           = 100,
  I2C_TIMEOUT        = 101,
  I2C_ERROR          = 102,    
} I2C_StatusType;


   
/* Defintions for the state of the DMA transfer */   
#define sEE_PAGESIZE              64
#define sEE_MAX_TRIALS_NUMBER     4096

/** 
  * @brief  I2C Parameter definition  
  */  
#define I2C_SPEED                    100000
#define I2C_EEAddress                0xA0
/**
  * @brief  I2C Interface pins
  */
#define I2C_PORT                     I2C2
#define I2C_CLK                      RCC_APB1PERIPH_I2C2
#define I2C_SCL_PIN                  GPIO_Pins_11                  /* PA11 */
#define I2C_SCL_GPIO_PORT            GPIOA                        /* GPIOA */
#define I2C_SCL_GPIO_CLK             RCC_AHBPERIPH_GPIOA
#define I2C_SCL_GPIO_PinsSource      GPIO_PinsSource11            
#define I2C_SCL_GPIO_GPIO_AF         GPIO_AF_5     

#define I2C_SCL_SET()            GPIO_WriteBit(I2C_SCL_GPIO_PORT, I2C_SCL_PIN, Bit_SET)      
#define I2C_SCL_CLR()            GPIO_WriteBit(I2C_SCL_GPIO_PORT, I2C_SCL_PIN, Bit_RESET)

#define I2C_SDA_PIN                  GPIO_Pins_12                  /* PA12 */
#define I2C_SDA_GPIO_PORT            GPIOA                       /* GPIOA */
#define I2C_SDA_GPIO_CLK             RCC_AHBPERIPH_GPIOA
#define I2C_SDA_GPIO_PinsSource      GPIO_PinsSource12             
#define I2C_SDA_GPIO_GPIO_AF         GPIO_AF_5 



#define EEPROM_WP_PIN              		GPIO_Pins_6
#define EEPROM_WP_GPIO             		GPIOF
#define EEPROM_WP_GPIO_RCC_CLK     		RCC_AHBPERIPH_GPIOF
#define EEPROM_WP_LOCK()            	GPIO_WriteBit(EEPROM_WP_GPIO, EEPROM_WP_PIN, Bit_SET)      
#define EEPROM_WP_UNLOCK()            GPIO_WriteBit(EEPROM_WP_GPIO, EEPROM_WP_PIN, Bit_RESET)

/** 
  * @}
  */

/* Exported macro ------------------------------------------------------------*/

/** @defgroup I2C_Exported_Macros I2C Exported Macros
  * @{
  */
/** 
  * @}
  */
  
/* Exported functions ------------------------------------------------------- */

/** @addtogroup I2C_Exported_Functions
  * @{  
  */
  
 
void initBl24Cxx(void); 
void I2Cx_Init(I2C_Type* I2Cx);

I2C_StatusType I2C_EE_ReadBuffer (I2C_Type* I2Cx, uint16_t ReadAddr,  uint8_t *pBuffer, uint16_t NumByteToRead,  uint32_t Timeout);
I2C_StatusType I2C_EE_WriteBuffer(I2C_Type* I2Cx, uint16_t WriteAddr, uint8_t *pBuffer, uint16_t NumByteToWrite, uint32_t Timeout);
I2C_StatusType I2C_EE_WritePage  (I2C_Type* I2Cx, uint16_t WriteAddr, uint8_t *pBuffer, uint16_t NumByteToWrite, uint32_t Timeout);

#endif
