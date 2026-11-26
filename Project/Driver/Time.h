/**
  **************************************************************************
  * File   : TIME.h
  * Version: V1.3.0
  * Date   : 2021-03-18
  * Brief  : Header file for AT-START board
  *          1. 
  */

#ifndef __TIME_H
#define __TIME_H	 
#include <at32f4xx.h>
#include "stdio.h"
void Delay_init(void);

#define STEP_DELAY_MS	50

uint8_t time3Init(void);
void delayInit(void);
void delayMs(u16 nms);
void delayUs(u32 nus);
void delaySec(u16 sec);
void Delay_ms(u16 nms);
void NVIC_Time3Configuration(void);

#endif
