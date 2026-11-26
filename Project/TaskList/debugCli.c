/*!
 *     COPYRIGHT NOTICE
 *     Copyright (c) 2021,尧乐
 *     All rights reserved.
 *
 *
 * @file       debugCli.c
 * @brief      
 * @author     周潇
 * @version    v1.0
 * @date       2021-03-13
 */
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include "debugCli.h"
#include "uart.h"
#include "systemInit.h"
   
/**
 * gbt31960.3-2015调试      
 * @param:  NONE    	
 * @return: NONE
*/
unsigned char g_ucSystemDebugLevel;

/**
 * 调试等级     
 * @param:  NONE    	
 * @return: NONE
*/
void debugLevel(void)
{
    g_ucSystemDebugLevel = g_ptSystemConfiguration->m_tMatSystemCfg.m_ucDebugLevel;
}


/**
 * 硬件调试     
 * @param:  NONE    	
 * @return: NONE
*/
void debugCli(unsigned char level, char *fmt, ...)
{
    char temp[200]={0,};
    if(level>g_ucSystemDebugLevel)
    {
        return;
    }
    va_list ap;
    va_start(ap, fmt);		
    vsprintf(temp, fmt, ap);
    va_end(ap);
    if(LOG_CRITICAL == level )
    {
			printf("[1]%s\n\r",temp);
    }
    else if(LOG_IMPORTANT == level )
    {
        printf("[2]%s\n\r",temp);
    }
    else if(LOG_ERROR == level )
    {
				printf("[3]%s\n\r",temp);
    }
    else if(LOG_WARNNING == level )
    {
				printf("[4]%s\n\r",temp);
    }
    else if(LOG_OTHER== level )
    {
        printf("[5]%s\n\r",temp);
    }
    else
	;
    return;
}

