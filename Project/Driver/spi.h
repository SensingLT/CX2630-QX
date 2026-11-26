#ifndef __SPI_H
#define __SPI_H	 
#include <at32f4xx.h>

#define SPI1_BUF                     480
#define SPI_SLAVER                   SPI1
#define SPI_SLAVER_CLK               RCC_APB2PERIPH_SPI1
#define SPI_SLAVER_GPIO              GPIOA
#define SPI_SLAVER_GPIO_CLK          RCC_AHBPERIPH_GPIOA 

#define SPI_SLAVER_PIN_CS            GPIO_Pins_4
#define SPI_SLAVER_PIN_SCK           GPIO_Pins_5
#define SPI_SLAVER_PIN_MISO          GPIO_Pins_6
#define SPI_SLAVER_PIN_MOSI          GPIO_Pins_7

#define SPI_READ_CS()                  GPIO_ReadInputDataBit(GPIOA,SPI_SLAVER_PIN_CS) 

void SPI1SlaverCfg(void);
void NVIC_Spi1Configuration(void);
#endif