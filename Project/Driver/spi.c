#include "spi.h"
SPI_InitType   SPI_InitStructure;

/* Configure and enable SPI_SLAVE interrupt --------------------------------*/
void NVIC_Spi1Configuration(void)
{
    NVIC_InitType NVIC_InitStructure;

	  NVIC_InitStructure.NVIC_IRQChannel = SPI1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
}		

/**
  * @brief  Configures the different SPIy and SPIz GPIO ports.
  * @param  SPIy_Mode: Specifies the SPIy operating mode. 
  *            This parameter can be:
  *              -  SPI1_Mode_Slave                 
  * @retval None
  */
void SPI1SlaverCfg(void)
{
		GPIO_InitType GPIO_InitStructure;
	  NVIC_InitType NVIC_InitStructure;
		
		RCC_APB2PeriphClockCmd(SPI_SLAVER_CLK, ENABLE);
		RCC_AHBPeriphClockCmd(SPI_SLAVER_GPIO_CLK, ENABLE);
			
	
		GPIO_StructInit(&GPIO_InitStructure);
		/* Configure SPI pins: SCK, MISO and MOSI ---------------------------------*/
		GPIO_InitStructure.GPIO_Pins = SPI_SLAVER_PIN_SCK | SPI_SLAVER_PIN_MOSI;
		GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_10MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Pull = GPIO_Pull_PD;
		GPIO_Init(SPI_SLAVER_GPIO, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pins = SPI_SLAVER_PIN_MISO;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Pull = GPIO_Pull_PD;
		GPIO_Init(SPI_SLAVER_GPIO, &GPIO_InitStructure);
		GPIO_PinAFConfig(SPI_SLAVER_GPIO, GPIO_PinsSource5, GPIO_AF_0);
		GPIO_PinAFConfig(SPI_SLAVER_GPIO, GPIO_PinsSource6, GPIO_AF_0);
		GPIO_PinAFConfig(SPI_SLAVER_GPIO, GPIO_PinsSource7, GPIO_AF_0);
	
		GPIO_StructInit(&GPIO_InitStructure);                    /*复位到初始状态*/
		GPIO_InitStructure.GPIO_Pins = SPI_SLAVER_PIN_CS;				 

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_OutType = GPIO_OutType_PP;
		GPIO_InitStructure.GPIO_Pull = GPIO_Pull_NOPULL;

		GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;	
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		//SPI配置
		SPI_DefaultInitParaConfig(&SPI_InitStructure);
		SPI_InitStructure.SPI_TransMode = SPI_TRANSMODE_FULLDUPLEX;
		SPI_InitStructure.SPI_Mode = SPI_MODE_SLAVE;
		SPI_InitStructure.SPI_FrameSize = SPI_FRAMESIZE_8BIT;
		SPI_InitStructure.SPI_CPOL = SPI_CPOL_LOW;
		SPI_InitStructure.SPI_CPHA = SPI_CPHA_2EDGE;
		SPI_InitStructure.SPI_NSSSEL = SPI_NSSSEL_SOFT;
		SPI_InitStructure.SPI_MCLKP = SPI_MCLKP_8;
		SPI_InitStructure.SPI_FirstBit = SPI_FIRSTBIT_LSB;
		SPI_InitStructure.SPI_CPOLY = 7;
		SPI_Init(SPI_SLAVER, &SPI_InitStructure);

    SPI_I2S_INTConfig(SPI_SLAVER, SPI_I2S_INT_RNE, ENABLE);
		/* Enable SPI_SLAVER */
		SPI_Enable(SPI_SLAVER, ENABLE);
		NVIC_Spi1Configuration();

}



