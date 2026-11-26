/**
  **************************************************************************
  * File   : rtc.h
  * Version: V1.3.0
  * Date   : 2021-03-18
  * Brief  : Header file for AT-START board
  *          1. 
  */

#ifndef __RTC_H
#define __RTC_H	 
#include <at32f4xx.h>


void exRtcConfig(void);
void initRtc(void);
void eRtcTimeShow(void);
void eRtcTimeSetBCD(uint8_t ucHour,uint8_t ucMin,uint8_t ucSec,uint8_t ucWeek,uint8_t ucYear,uint8_t ucMonth,uint8_t ucDay);
void eRtcTimeGetBin(ERTC_TimeType *pERTC_TimeStructure,ERTC_DateType *pERTC_DataStructure);
#endif

