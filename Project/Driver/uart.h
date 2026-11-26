/**
  **************************************************************************
  * File   : uart.h
  * Version: V1.3.0
  * Date   : 2021-03-18
  * Brief  : Header file for AT-START board
  *          1. 
  */

#ifndef __UART_H
#define __UART_H	 
#include <at32f4xx.h>

/*RS485初始化*/
#define RS_UART		                  USART1

/*Tx*/
#define RS_UARTTX_PIN               GPIO_Pins_9 
#define RS_UARTTX_GPIO              GPIOA
#define RS_UARTTX_GPIO_RCC          RCC_AHBPERIPH_GPIOA

/*Rx*/
#define RS_UARTRX_PIN          		  GPIO_Pins_10
#define RS_UARTRX_GPIO              GPIOA
#define RS_UARTRX_GPIO_RCC          RCC_AHBPERIPH_GPIOA



/*调试串口初始化*/
#define DEBUG_UART                     USART2
#define USARTx_IRQn                    USART2_IRQn
#define USARTx_IRQ_Handler             USART2_IRQHandler

/*Tx*/
#define DEBUG_UARTTX_PIN               GPIO_Pins_2
#define DEBUG_UARTTX_GPIO           	 GPIOA
#define DEBUG_UARTTX_GPIO_RCC     	   RCC_AHBPERIPH_GPIOA

/*Rx*/
#define DEBUG_UARTRX_PIN                GPIO_Pins_3
#define DEBUG_UARTRX_GPIO               GPIOA
#define DEBUG_UARTRX_GPIO_RCC           RCC_AHBPERIPH_GPIOA




/*RS485变量初始化*/
//---
#define USART1_TXBUF_LEN   1024
#define USART1_RXBUF_LEN   400
#define USART1_RXBUF_DEPTH 2
struct tagUart1Phy_T
{
	  uint16_t m_usRxLen;
	  uint8_t m_ucaTx[USART1_RXBUF_LEN];
    uint8_t m_ucaRx[USART1_RXBUF_LEN];
    uint16_t m_usTxLen;
    uint16_t m_usRxDepth;
    uint8_t m_ucReceived;
    uint8_t m_ucSnd;
};
typedef struct tagUart1Phy_T uart1Phy_T; 
extern uart1Phy_T g_tUart1Phy;


#define USART2_TXBUF_LEN   460
#define USART2_RXBUF_LEN   1440
struct tagUart2Phy_T
{
	  uint8_t m_ucaTx[USART2_TXBUF_LEN];
    uint8_t m_ucaRx[USART2_RXBUF_LEN];
    uint32_t m_ulTxLen;
    uint32_t m_ulRxLen;
    uint32_t m_ulRxDepth;
    uint8_t m_ucReceived;
    uint8_t m_ucSnd;
};
typedef struct tagUart2Phy_T uart2Phy_T; 
extern uart2Phy_T g_tUart2Phy;


void    NVIC_Uart1Configuration(void);
void    NVIC_Uart2Configuration(void);
uint8_t debugPortInit(uint32_t ulBaud);
uint8_t qmsPortInit(uint32_t ulBaud);
void    uart1SendMsg(uint8_t *pData,uint32_t ulTxLEN);
void    uart1Idle(void);
void    uart2SendMsg(uint8_t *pData,uint32_t ulTxLEN);
void    uart2Idle(void);
void    uart2RxDma(void);
uint8_t getDataFromUart2(void);
char debugSendMsg(uint8_t *pData,uint32_t ulTxLEN);
uint8_t rsPortInit(uint32_t ulBaud);
#endif

