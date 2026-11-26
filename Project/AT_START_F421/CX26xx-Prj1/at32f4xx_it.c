/**
  ******************************************************************************
  * File   : Templates/at32f4xx_it.c 
  * Version: V1.3.0
  * Date   : 2021-03-18
  * Brief  : Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  */ 


/* Includes ------------------------------------------------------------------*/
#include "at32f4xx_it.h"
#include "systemInit.h"
#include "uart.h"
#include "exAdc.h"
#include "IoPort.h"

/** @addtogroup AT32F421_StdPeriph_Templates
  * @{
  */

/** @addtogroup GPIO_LED_Toggle
  * @{
  */

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSV_Handler exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
}

/**
  * @brief  This function handles USART1 global interrupt request.
  * @param  None
  * @retval None
  */
void USART1_IRQHandler(void)
{
  if(USART_GetITStatus(USART1, USART_INT_IDLEF))
  {
		  LEDRED_CLR();
		  uart1Idle();
  }
}

/**
  * @brief  This function handles USART1 global interrupt request.
  * @param  None
  * @retval None
  */

void USART2_IRQHandler(void)
{
  if(USART_GetITStatus(USART2, USART_INT_IDLEF))
  {
			uart2Idle();
  }
}

/**
  * @brief  This function handles adc global interrupt request.
  * @param  None
  * @retval None
  */
void ADC_COMP_IRQHandler(void)
{
		g_tAdc1.m_ucSampleCompleted = 1;
}

/**
  * @brief  This function handles TMR3 global interrupt request.
  * @param  None
  * @retval None
  */
uint8_t g_ucSecFlg;
uint8_t g_ucUpdata10msFlg;
uint8_t g_ucLed;
void TMR3_GLOBAL_IRQHandler(void)
{
	  if (TMR_GetINTStatus(TMR3, TMR_INT_Overflow) != RESET)
		{
			  TMR_ClearITPendingBit(TMR3, TMR_INT_Overflow);
			  systemSetRecord();
			  g_ucUpdata10msFlg=1;
			  g_tSystemTick.m_ulTick++;
			  g_tSystemTick.m_ulTick1S++;
			  if(g_tSystemTick.m_ulTick1S >99)
				{
						g_tSystemTick.m_ulTick1S = 0;
					  g_ucSecFlg = 1;
					  g_ucLed++;
						if(g_ucLed %2 == 0)
						{
								LEDGREEN_SET();
						}
						else
						{
							LEDGREEN_CLR();
						}
				}
		}
}



/******************************************************************************/
/*                 AT32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_at32f413_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/


/**
  * @}
  */

/**
  * @}
  */

