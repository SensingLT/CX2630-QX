
#include "uart.h"
#include "stdio.h"

#ifdef __cplusplus
namespace std
{
  extern "C" {
#endif

/* Suport printf function, useMicroLib is unnecessary */
#ifdef __CC_ARM
  #pragma import(__use_no_semihosting)
  struct __FILE
  {
    int handle;
  };
  
  FILE __stdout;
  
  void _sys_exit(int x)
  {
    x = x;
  }
#endif

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{  
		USART_SendData(DEBUG_UART, ch);
		while ( USART_GetFlagStatus(DEBUG_UART, USART_FLAG_TRAC) == RESET ){};
		return ch;
}


#ifdef __cplusplus
  } /* extern "C" */
} /* namespace */
#endif
	
#define Hex2Ascii(data)  (data < 10)? ('0' + data) : ('A' + data - 10)

uart1Phy_T g_tUart1Phy;
uart2Phy_T g_tUart2Phy;


char debugSendMsg(uint8_t *pData,uint32_t ulTxLEN)
{
	  uint16_t i;
	
		for(i=0;i<ulTxLEN;i++)
		{
				USART_SendData(DEBUG_UART, *(pData+i));
				while ( USART_GetFlagStatus(DEBUG_UART, USART_FLAG_TRAC) == RESET ){};
		}			
		return 0;
}



/**
  * @brief  调试串口初始化
  * @param  None
  * @retval None
  */
uint8_t debugPortInit(uint32_t ulBaud)
{
		GPIO_InitType GPIO_InitStructure;
		USART_InitType USART_InitStructure;

		/*Enable the UART Clock*/
		RCC_AHBPeriphClockCmd(DEBUG_UARTTX_GPIO_RCC | DEBUG_UARTRX_GPIO_RCC, ENABLE);	
		RCC_APB1PeriphClockCmd(RCC_APB1PERIPH_USART2, ENABLE);

			/* Configure the UART2 TX pin */
		GPIO_StructInit(&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pins = DEBUG_UARTTX_PIN; 
		GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OutType = GPIO_OutType_PP;
		GPIO_InitStructure.GPIO_Pull = GPIO_Pull_NOPULL;

		GPIO_Init(DEBUG_UARTTX_GPIO, &GPIO_InitStructure);

			/* Configure the UART2 RX pin */
		GPIO_InitStructure.GPIO_Pins = DEBUG_UARTRX_PIN;//PA3
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Pull = GPIO_Pull_PU;

		GPIO_Init(DEBUG_UARTRX_GPIO, &GPIO_InitStructure);

		GPIO_PinAFConfig(GPIOA, GPIO_PinsSource2, GPIO_AF_1);
		GPIO_PinAFConfig(GPIOA, GPIO_PinsSource3, GPIO_AF_1);

			/*Configure UART param*/
		USART_StructInit(&USART_InitStructure);
		USART_InitStructure.USART_BaudRate = ulBaud;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	

		USART_Init(DEBUG_UART, &USART_InitStructure); 
		uart2RxDma();
		USART_DMACmd(DEBUG_UART, USART_DMAReq_Rx, ENABLE);
		/* Enable USART1 TX DMA1 Channel */
		DMA_ChannelEnable(DMA1_Channel4, ENABLE);
		/* Enable USART1 RX DMA1 Channel */
		DMA_ChannelEnable(DMA1_Channel5, ENABLE);
		USART_Cmd(DEBUG_UART, ENABLE);
		USART_INTConfig(DEBUG_UART, USART_INT_IDLEF, ENABLE);
    NVIC_Uart2Configuration();		
		return 0;
}	



/**
  * @brief  Configures the DMA.
  * UART2DMA
  * @param  None
  * @retval None
  */
void uart2RxDma(void)
{
		DMA_InitType DMA_InitStructure;

		DMA_Reset(DMA1_Channel5);  
		DMA_DefaultInitParaConfig(&DMA_InitStructure);
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DT;
		DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_tUart2Phy.m_ucaRx;
		DMA_InitStructure.DMA_Direction = DMA_DIR_PERIPHERALSRC;
		DMA_InitStructure.DMA_BufferSize = USART2_RXBUF_LEN;
	  DMA_InitStructure.DMA_PeripheralInc = DMA_PERIPHERALINC_DISABLE;
	  DMA_InitStructure.DMA_MemoryInc = DMA_MEMORYINC_ENABLE;
		DMA_Init(DMA1_Channel5, &DMA_InitStructure); 
		USART_DMACmd(DEBUG_UART, USART_DMAReq_Rx, ENABLE);
		DMA_ChannelEnable(DMA1_Channel5, ENABLE);
}

/**
  * @brief  发送DMA请求
  * @param  None
  * @retval None
  */
void uart2TxDma(uint8_t *pData,uint32_t ulTxLen)
{
		DMA_InitType DMA_InitStructure;
		/* USART1 TX DMA1 Channel (triggered by USART1 Tx event) Config */
		DMA_Reset(DMA1_Channel4);
		DMA_DefaultInitParaConfig(&DMA_InitStructure);
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DT;
		DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pData;
		DMA_InitStructure.DMA_Direction = DMA_DIR_PERIPHERALDST;
		DMA_InitStructure.DMA_BufferSize = ulTxLen;
		DMA_InitStructure.DMA_PeripheralInc = DMA_PERIPHERALINC_DISABLE;
		DMA_InitStructure.DMA_MemoryInc = DMA_MEMORYINC_ENABLE;
		DMA_InitStructure.DMA_PeripheralDataWidth = DMA_PERIPHERALDATAWIDTH_BYTE;
		DMA_InitStructure.DMA_MemoryDataWidth = DMA_MEMORYDATAWIDTH_BYTE;
		DMA_InitStructure.DMA_Mode = DMA_MODE_NORMAL;
		DMA_InitStructure.DMA_Priority = DMA_PRIORITY_VERYHIGH;
		DMA_InitStructure.DMA_MTOM = DMA_MEMTOMEM_DISABLE;
		DMA_Init(DMA1_Channel4, &DMA_InitStructure);
	  USART_DMACmd(DEBUG_UART, USART_DMAReq_Tx, ENABLE);
  	DMA_ChannelEnable(DMA1_Channel4, ENABLE);
}

/**
  * @brief  Configures the nested vectored interrupt controller.
  * @param  None
  * @retval None
  */
void NVIC_Uart2Configuration(void)
{
   NVIC_InitType NVIC_InitStructure;

  /* Enable the USART1 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  uart1空闲中断
  * @param  None
  * @retval None
  */
void uart2Idle(void)
{
	  uint32_t t_ulSts,t_ulDt;
		uint32_t t_ulRxLen;
	
		t_ulSts = USART2->STS;
		t_ulDt  = USART2->DT;
		t_ulRxLen = USART2_RXBUF_LEN - DMA_GetCurrDataCounter(DMA1_Channel5);

	  if(t_ulRxLen==0)
		{
				uart2RxDma();
		}
		else
		{
			  uart2RxDma();
				g_tUart2Phy.m_ulRxLen = t_ulRxLen;
				g_tUart2Phy.m_ucReceived = 1;
		}		
}

		
/**
  * @brief  发送数据
  * @param  None
  * @retval None
  */
void uart2SendMsg(uint8_t *pData,uint32_t ulTxLEN)
{			
		uart2TxDma(pData,ulTxLEN);
		/* Wait until USART2 TX DMA1 Channel  Transfer Complete */
		while (DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET){};				
}

//-------------------对外串口--------------------//

/**
  * @brief  Configures the DMA.
  * UART1串口初始化
  * @param  None
  * @retval None
  */
void uart1RxDma(void)
{
		DMA_InitType DMA_InitStructure;

		DMA_Reset(DMA1_Channel3);  
		DMA_DefaultInitParaConfig(&DMA_InitStructure);
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DT;
		DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_tUart1Phy.m_ucaRx;
		DMA_InitStructure.DMA_Direction = DMA_DIR_PERIPHERALSRC;
		DMA_InitStructure.DMA_BufferSize = USART1_RXBUF_LEN;
	  DMA_InitStructure.DMA_PeripheralInc = DMA_PERIPHERALINC_DISABLE;
	  DMA_InitStructure.DMA_MemoryInc = DMA_MEMORYINC_ENABLE;
		DMA_Init(DMA1_Channel3, &DMA_InitStructure); 
		USART_DMACmd(RS_UART, USART_DMAReq_Rx, ENABLE);
		DMA_ChannelEnable(DMA1_Channel3, ENABLE);
}

/**
  * @brief  发送DMA请求
  * @param  None
  * @retval None
  */
void uart1TxDma(uint8_t *pData,uint32_t ulTxLen)
{
		DMA_InitType DMA_InitStructure;
		/* USART1 TX DMA1 Channel (triggered by USART1 Tx event) Config */
		DMA_Reset(DMA1_Channel2);
		DMA_DefaultInitParaConfig(&DMA_InitStructure);
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DT;
		DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pData;
		DMA_InitStructure.DMA_Direction = DMA_DIR_PERIPHERALDST;
		DMA_InitStructure.DMA_BufferSize = ulTxLen;
		DMA_InitStructure.DMA_PeripheralInc = DMA_PERIPHERALINC_DISABLE;
		DMA_InitStructure.DMA_MemoryInc = DMA_MEMORYINC_ENABLE;
		DMA_InitStructure.DMA_PeripheralDataWidth = DMA_PERIPHERALDATAWIDTH_BYTE;
		DMA_InitStructure.DMA_MemoryDataWidth = DMA_MEMORYDATAWIDTH_BYTE;
		DMA_InitStructure.DMA_Mode = DMA_MODE_NORMAL;
		DMA_InitStructure.DMA_Priority = DMA_PRIORITY_VERYHIGH;
		DMA_InitStructure.DMA_MTOM = DMA_MEMTOMEM_DISABLE;
		DMA_Init(DMA1_Channel2, &DMA_InitStructure);
	  USART_DMACmd(RS_UART, USART_DMAReq_Tx, ENABLE);
  	DMA_ChannelEnable(DMA1_Channel2, ENABLE);
}

/**
  * @brief  UART1初始化
  * @param  None
  * @retval None
  */
uint8_t rsPortInit(uint32_t ulBaud)
{
		GPIO_InitType GPIO_InitStructure;
		USART_InitType USART_InitStructure;

		/*Enable the UART Clock*/
		RCC_AHBPeriphClockCmd(RS_UARTTX_GPIO_RCC, ENABLE);	
		RCC_APB2PeriphClockCmd(RCC_APB2PERIPH_USART1, ENABLE);
	  RCC_AHBPeriphClockCmd(RCC_AHBPERIPH_DMA1, ENABLE);
	
		/* Configure the UART1 TX pin */
		GPIO_StructInit(&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pins = RS_UARTTX_PIN; 
		GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OutType = GPIO_OutType_PP;
		GPIO_InitStructure.GPIO_Pull = GPIO_Pull_NOPULL;
		GPIO_Init(RS_UARTTX_GPIO, &GPIO_InitStructure);

		/* Configure the UART1 RX pin */
		GPIO_InitStructure.GPIO_Pins = RS_UARTRX_PIN;//PA10
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Pull = GPIO_Pull_NOPULL;

		GPIO_Init(RS_UARTRX_GPIO, &GPIO_InitStructure);

		GPIO_PinAFConfig(GPIOA, GPIO_PinsSource9, GPIO_AF_1);
		GPIO_PinAFConfig(GPIOA, GPIO_PinsSource10, GPIO_AF_1);

			/*Configure UART param*/
		USART_StructInit(&USART_InitStructure);
		USART_InitStructure.USART_BaudRate = ulBaud;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	

		USART_Init(RS_UART, &USART_InitStructure); 
		uart1RxDma();
		USART_DMACmd(RS_UART, USART_DMAReq_Rx, ENABLE);
		/* Enable USART1 TX DMA1 Channel */
		DMA_ChannelEnable(DMA1_Channel2, ENABLE);
		/* Enable USART1 RX DMA1 Channel */
		DMA_ChannelEnable(DMA1_Channel3, ENABLE);
	
		USART_Cmd(RS_UART, ENABLE); 
    USART_INTConfig(RS_UART, USART_INT_IDLEF, ENABLE);

	  NVIC_Uart1Configuration();

		return 0;
}

/**
  * @brief  发送数据
  * @param  None
  * @retval None
  */
void uart1SendMsg(uint8_t *pData,uint32_t ulTxLEN)
{
	  uart1TxDma(pData,ulTxLEN);
		while (DMA_GetFlagStatus(DMA1_FLAG_TC2) == RESET){};	  			
}

/**
  * @brief  Configures the nested vectored interrupt controller.
  * @param  None
  * @retval None
  */
void NVIC_Uart1Configuration(void)
{
   NVIC_InitType NVIC_InitStructure;

  /* Enable the USART1 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  uart1空闲中断
  * @param  None
  * @retval None
  */
void uart1Idle(void)
{
	  uint32_t t_ulSts,t_ulDt;
		uint32_t t_ulRxLen;
	
		t_ulSts = USART1->STS;
		t_ulDt  = USART1->DT;
		t_ulRxLen = USART1_RXBUF_LEN - DMA_GetCurrDataCounter(DMA1_Channel3);
	
	  if(t_ulRxLen==0)
		{
				uart1RxDma();
		}
		else
		{
			  uart1RxDma();
				g_tUart1Phy.m_usRxLen = t_ulRxLen;
				g_tUart1Phy.m_ucReceived = 1;
		}	
}
