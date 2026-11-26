/**
  ******************************************************************************
  * File   : Templates/main.c 
  * Version: V1.3.0
  * Date   : 2021-03-18
  * Brief  : Main program body
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "uart.h"
#include "IoPort.h"
#include "debugCli.h"
#include "systemInit.h"
#include "sampleTask.h"
#include "Rsms128.h"
#include "yotliveMs128.h"
#include "time.h"
#include <stdio.h>


/** @addtogroup AT32F421_StdPeriph_Templates
  * @{
  */

/** @addtogroup Template
  * @{
  */

/* Private define ------------------------------------------------------------*/



/* Extern variables ---------------------------------------------------------*/
extern uint8_t g_ucUpdata10msFlg;
extern uint8_t g_ucYtSndFlg;
extern uint8_t g_ucrsYtSndFlg;
extern uint8_t g_ucSecFlg;

/* Private variables ---------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/	
/* Private functions ---------------------------------------------------------*/



/**
  * @brief  Configure Button EXTI   
  * @param  Button: Specifies the Button to be configured.
  * @retval None
  */


/**
  * @brief  Button EXTI0 Interrupt Handler
  * @param  None
  * @retval None
  */


/**
  * @brief  Button EXTI13 Interrupt Handler
  * @param  None
  * @retval None
  */


/**
  * @brief  Main Function.
  * @param  None
  * @retval None
  */
int main(void)
{
	uint32_t t_ulTime;
	systemParmInit();
	for(;;)
	{
		if(g_ucUpdata10msFlg==1)
		{
			g_ucUpdata10msFlg = 0;
			g_tSystemTick.m_ulTickSp++;
			if(g_tSystemTick.m_ulTickSp > g_ptSystemConfiguration->m_tMatSystemCfg.m_ulTick)
			{
				g_tSystemTick.m_ulTickSp  = 0;
				sampleTask();
				if(g_ucDebugSndFlg == 1)
				{
					debugMtCmdSndDat();
				}
				sndMsg();
			}
		}
		systemWriteDurationTime();
		getDataFromUart1();
		getDataFromUart2();
  }
}


#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif

/**
  * @}
  */

/**
  * @}
  */



