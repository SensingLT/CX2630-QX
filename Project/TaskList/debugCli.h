/*
 * File:    debug_p.h
 * Purpose: 
 *
 * Notes:
 */

#ifndef _DEBUGCLI_H_
#define _DEBUGCLI_H_

#include <at32f4xx.h>
#define SYSTEM_DEBUG_LEV 5


#define LOG_CRITICAL	(1)
#define LOG_IMPORTANT	(2)
#define LOG_ERROR	(3)
#define LOG_WARNNING	(4)
#define LOG_OTHER       (5)

void initRtc(void);
void ERTC_TimeStampConfig(void);
void debugCli(unsigned char level, char *fmt, ...);
void espPrintf(uint8_t ucLevel,uint8_t const *head,uint8_t ucHeadLen,uint8_t *str,uint16_t usStrLen);
uint8_t debugUSART1Init(void);
void debugLevel(void);
extern unsigned char g_ucSystemDebugLevel;
#endif 

