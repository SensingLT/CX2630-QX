
#include "IoPort.h"
#include "uart.h"
#include "rtc.h"
#include "systemInit.h"
#include "debugCli.h"
#define ERTC_CLOCK_SOURCE_LSI


uint32_t uwAsynchPrediv = 0;
uint32_t uwSynchPrediv = 0;
	
	
/**
  * @brief  配置RTC时钟源.
  * @param  None
  * @retval None
  */
void exRtcConfig(void)
{
		ERTC_DateType  ERTC_DateStructure;
    ERTC_TimeType  ERTC_TimeStructure;
	  ERTC_InitType  ERTC_InitStructure;
	
		/* Enable the PWR clock */
		RCC_APB1PeriphClockCmd(RCC_APB1PERIPH_PWR, ENABLE);

		/* Allow access to ERTC */
		PWR_BackupAccessCtrl(ENABLE);
			
		/* Reset ERTC Domain */
		RCC_BackupResetCmd(ENABLE);
		RCC_BackupResetCmd(DISABLE);
		
#if defined (ERTC_CLOCK_SOURCE_LSI)  /* LSI used as ERTC source clock,片内低速振荡器*/
		/* The ERTC Clock may varies due to LSI frequency dispersion */   
		/* Enable the LSI OSC */ 
		RCC_LSICmd(ENABLE);

		/* Wait till LSI is ready */  
		while(RCC_GetFlagStatus(RCC_FLAG_LSISTBL) == RESET){}

		/* Select the ERTC Clock Source */
		RCC_ERTCCLKConfig(RCC_ERTCCLKSelection_LSI);
		
		/* ck_spre(1Hz) = ERTCCLK(LSI) /(uwAsynchPrediv + 1)*(uwSynchPrediv + 1)*/
		switch(g_ptSystemConfiguration->m_tMatSystemCfg.m_ucRtcSpeed)
		{
			case 0: //256倍速
			uwSynchPrediv = 0;	
			break;
			case 1://128倍速
			uwSynchPrediv = 1;	
			break;
			case 2://64倍速
			uwSynchPrediv = 3;	
			break;
			case 3://32倍速
			uwSynchPrediv = 7;	
			break;
			case 4://16倍速
			uwSynchPrediv = 15;	
			break;
			case 5://4倍速
			uwSynchPrediv = 63;	
			break;
			case 6://1倍速
			uwSynchPrediv = 255;	
			break;
      default:
			uwSynchPrediv = 255;
      break;				
		}
		
		uwAsynchPrediv = 0x7F;

#elif defined (ERTC_CLOCK_SOURCE_LSE) /* LSE used as ERTC source clock，片外32768Hz振荡器 */
		/* Enable the LSE OSC */
		RCC_LSEConfig(RCC_LSE_ENABLE);

		/* Wait till LSE is ready */  
		while(RCC_GetFlagStatus(RCC_FLAG_LSESTBL) == RESET)
		{
		}

		/* Select the ERTC Clock Source */
		RCC_ERTCCLKConfig(RCC_ERTCCLKSelection_LSE);
		/* ck_spre(1Hz) = ERTCCLK(LSE) /(uwAsynchPrediv + 1)*(uwSynchPrediv + 1)*/
		switch(g_ptSystemConfiguration->m_tMatSystemCfg.m_ucRtcSpeed)
		{
			case 0: //256倍速
			uwSynchPrediv = 0;	
			break;
			case 1://128倍速
			uwSynchPrediv = 1;	
			break;
			case 2://64倍速
			uwSynchPrediv = 3;	
			break;
			case 3://32倍速
			uwSynchPrediv = 7;	
			break;
			case 4://16倍速
			uwSynchPrediv = 15;	
			break;
			case 5://4倍速
			uwSynchPrediv = 63;	
			break;
			case 6://1倍速
			uwSynchPrediv = 255;	
			break;
      default:
			uwSynchPrediv = 255;
      break;				
		}
		uwAsynchPrediv = 0x7F;
#endif /* ERTC_CLOCK_SOURCE_LSI */
		
		/* Enable the ERTC Clock */
		RCC_ERTCCLKCmd(ENABLE);

		/* Deinitializes the ERTC registers */  
		ERTC_Reset();  
		
		/* Wait for ERTC APB registers synchronisation */
		ERTC_WaitForSynchro();
		
		ERTC_InitStructure.ERTC_AsynchPrediv = uwAsynchPrediv;
		ERTC_InitStructure.ERTC_SynchPrediv = uwSynchPrediv;
		ERTC_InitStructure.ERTC_HourFormat = ERTC_HourFormat_24;
		ERTC_Init(&ERTC_InitStructure);
		
		/* Set the date: Friday January 11th 2013 */
		ERTC_DateStructure.ERTC_Year = 0x21;
		ERTC_DateStructure.ERTC_Month = ERTC_Month_JAN;
		ERTC_DateStructure.ERTC_Date = 0x11;
		ERTC_DateStructure.ERTC_WeekDay = ERTC_Week_SAT;
		ERTC_SetDateValue(ERTC_Format_BCD, &ERTC_DateStructure);
		
		/* Set the time to 05h 20mn 00s AM */
		ERTC_TimeStructure.ERTC_AMPM    = ERTC_H12_AM;
		ERTC_TimeStructure.ERTC_Hours   = 0x05;
		ERTC_TimeStructure.ERTC_Minutes = 0x20;
		ERTC_TimeStructure.ERTC_Seconds = 0x00; 
		
		ERTC_SetTimeValue(ERTC_Format_BCD, &ERTC_TimeStructure);   
		
		/* Indicator for the ERTC configuration */
		ERTC_WriteBackupRegister(ERTC_BKP_DT0, 0x32F1);
}




/**
  * @brief  初始化RTC时钟源.
  * @param  None
  * @retval None
  */
void initRtc(void)
{
	  //uint32_t t_ulTemp;
	
	  //t_ulTemp = ERTC_ReadBackupRegister(ERTC_BKP_DT0);
	  //debugCli(LOG_OTHER, "备份寄存器数值0x%x",t_ulTemp);	
		if(ERTC_ReadBackupRegister(ERTC_BKP_DT0) != 0x32F1)
		{  
			/* ERTC configuration  */
			exRtcConfig();
			/* Display the ERTC Time */
			eRtcTimeShow();
			debugCli(LOG_OTHER, "RTC初始化");
		}
		else
		{				
				/* Enable the PWR clock */
				RCC_APB1PeriphClockCmd(RCC_APB1PERIPH_PWR, ENABLE);
				/* Allow access to ERTC */
				PWR_BackupAccessCtrl(ENABLE);
				/* Wait for ERTC APB registers synchronisation */
				ERTC_WaitForSynchro();
				/* Display the ERTC Time and Alarm */
				eRtcTimeShow();
        		
		}
}



/**
  * @brief  显示当前时间.
  * @param  None
  * @retval None
  */
void eRtcTimeShow(void)
{
	  ERTC_TimeType  ERTC_TimeStructure;
		/* Get the current Time */
		ERTC_GetTimeValue(ERTC_Format_BIN, &ERTC_TimeStructure);
		/* Display time Format : hh:mm:ss */
		//printf("%0.2d:%0.2d:%0.2d\r\n",ERTC_TimeStructure.ERTC_Hours, ERTC_TimeStructure.ERTC_Minutes, ERTC_TimeStructure.ERTC_Seconds);
}

/**
  * @brief  读取当前时间参数
  * @param  None
  * @retval None
  */
void eRtcTimeGetBin(ERTC_TimeType *pERTC_TimeStructure,ERTC_DateType *pERTC_DataStructure)
{	
		ERTC_GetTimeValue(ERTC_Format_BIN, pERTC_TimeStructure);
	  ERTC_GetDateValue(ERTC_Format_BIN, pERTC_DataStructure);
	
		//ERTC_GetTimeStamp(ERTC_Format_BIN, pERTC_TimeStructure, pERTC_DataStructure);
}


/**
  * @brief  设置当前时间参数
  * @param  None
  * @retval None
  */
void eRtcTimeSetBCD(uint8_t ucHour,uint8_t ucMin,uint8_t ucSec,uint8_t ucWeek,uint8_t ucYear,uint8_t ucMonth,uint8_t ucDay)
{
	  ERTC_DateType ERTC_DateStructure;
	  ERTC_TimeType  ERTC_TimeStructure;
	  ERTC_TimeType  t_tErtcTimeStructure;
    ERTC_DateType  t_tErtcDataStructure;
	  uint16_t i;
	
		/* Set the date */
		ERTC_DateStructure.ERTC_Year = ucYear;
		ERTC_DateStructure.ERTC_Month = ucMonth;
		ERTC_DateStructure.ERTC_Date = ucDay;
		ERTC_DateStructure.ERTC_WeekDay = ucWeek;
		ERTC_SetDateValue(ERTC_Format_BCD, &ERTC_DateStructure);
		
		/* Set the time */
	  if(ucHour > 11)
		{
				ERTC_TimeStructure.ERTC_AMPM    = ERTC_H12_PM;
		}
		else
		{
				ERTC_TimeStructure.ERTC_AMPM    = ERTC_H12_AM;
		}
		ERTC_TimeStructure.ERTC_Hours   = ucHour;
		ERTC_TimeStructure.ERTC_Minutes = ucMin;
		ERTC_TimeStructure.ERTC_Seconds = ucSec; 
		
		ERTC_SetTimeValue(ERTC_Format_BCD, &ERTC_TimeStructure); 
		for(i=0;i<10000;i++);

		eRtcTimeGetBin(&t_tErtcTimeStructure,&t_tErtcDataStructure);  
	  ERTC_WriteBackupRegister(ERTC_BKP_DT1, 0x5555);//时间同步成功，可以进行操作	 
}

