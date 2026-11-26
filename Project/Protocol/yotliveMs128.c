#include <stdbool.h>
#include "systemInit.h"
#include "yotliveMs128.h"
#include "Rsms128.h"
#include "uart.h"
#include "stdio.h"
#include "time.h"
#include "crc.h"
#include "debugCli.h"
#include "sampleTask.h"
#include "string.h"
#include "rtc.h"
#include "myString.h"
#include "IoPort.h"
#include "myString.h"


uint8_t  g_ucDebugSndFlg;
extern uint8_t  g_ucaFileBuff[CFG_BUFF_LEN];
extern uint16_t g_ucaPressureRaw12Bit[ROW][COLUMN];
extern uint8_t  g_ucaPressureModbus[ROW*COLUMN*2];
uint16_t g_usSavCnt,g_usSavPoint;

//快速回复0字节或1字节的信息
void easy68Response(uint8_t reqCode, uint8_t data, bool withData)
{
    ytFrame_T* pf = (ytFrame_T*)&g_tUart2Phy.m_ucaTx;
    pf->m_ucHead = YT_HEAD;
    pf->m_ulAddr = g_ptSystemConfiguration->m_tMatCom.m_usId;
    pf->m_ucCmd  = reqCode | 0x80;

    uint8_t dataIndex = 0;
    pf->m_usLen = 0;
    if (withData) {
        pf->m_usLen = 1;
        pf->m_ucpDat[dataIndex++] = data;
    }

    uint16_t len = WINS_2017D_FIX_FRAME + pf->m_usLen;
    uint16_t t_usCrc = calcCrc16Modbus(0xFFFF, (uint8_t*)&pf->m_ulAddr, len - 1);
    pf->m_ucpDat[dataIndex++] = t_usCrc % 256;
    pf->m_ucpDat[dataIndex++] = t_usCrc / 256;
    uart2SendMsg(g_tUart2Phy.m_ucaTx, len + 2);
}

/**
 * 广播
 * @param:  NONE
 * @return: NONE
*/
uint8_t debugCmdBro(uint8_t *pBuf,uint32_t ulIndex)
{
    ytFrame_T *pf;
    uint16_t t_usCrc;
    uint8_t  *p1;
    uint16_t len;
    
    pf=(ytFrame_T*)&g_tUart2Phy.m_ucaTx;
    pf->m_ucHead = YT_HEAD;
    pf->m_ulAddr = g_ptSystemConfiguration->m_tMatCom.m_usId;
    pf->m_ucCmd  = WIN_2017D_BRO + 0x80;
    pf->m_usLen  = 12;
    len          = WINS_2017D_FIX_FRAME+12;
    memcpy(&pf->m_ucpDat,c_ucaDeviceMsg,4);
    pf->m_ucpDat[4] = g_ptSystemConfiguration->m_tMatCom.m_usId & 0xff;
    pf->m_ucpDat[5] = (g_ptSystemConfiguration->m_tMatCom.m_usId & 0xff00)>>8;
    pf->m_ucpDat[6] = 0;
    pf->m_ucpDat[7] = 0;
    memcpy(&pf->m_ucpDat[8], c_ucaHardware,2);
    memcpy(&pf->m_ucpDat[10],c_ucaFirmware,2);
    p1= (uint8_t*)&pf->m_ucHead;
    p1++;
    t_usCrc = calcCrc16Modbus(0XFFFF,p1,len-1);
    pf->m_ucpDat[12] = t_usCrc % 256;
    pf->m_ucpDat[13] = t_usCrc / 256;
		//debugSendMsg(g_tUart2Phy.m_ucaTx,len+2);
		uart2SendMsg(g_tUart2Phy.m_ucaTx,len+2);
		return 0;
}

/**
 * 写配置文件
 * @param:  NONE
 * @return: NONE
*/
uint8_t debugCmdWrCfgFile(uint8_t *pBuf,uint32_t ulIndex)
{
    uint16_t t_usCrc;
    uint8_t  *p1;
    uint16_t len;

	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t i;
	  uint8_t t_ucFlg;
	  uint8_t t_ucaTx[USART2_TXBUF_LEN];
    
	  t_ucFlg = 0;
    p=(ytFrame_T*)(&pBuf[ulIndex]);  
    if(p->m_usLen!=CFG_FILE_PACKAGE)return 1;
    uint32_t t_ulOffset = (uint32_t)p->m_tCd.Count * CFG_FILE_PACKAGE;
    if(t_ulOffset + p->m_usLen > CFG_BUFF_LEN)
    {
        return 1;
    }
    if((p->m_tCd.FIN==0) && (p->m_tCd.FIR==1))//首帧
    {
        for(i=0;i<p->m_usLen;i++)
        {
          g_ucaFileBuff[i] = p->m_ucpDat[i];
        }
    }
    else if((p->m_tCd.FIN==0) && (p->m_tCd.FIR==0))
    {
        for(i=0;i<p->m_usLen;i++)
        {
          g_ucaFileBuff[i + t_ulOffset] = p->m_ucpDat[i];
        }
    }
    else if((p->m_tCd.FIN==1) && (p->m_tCd.FIR==0))
    {
        for(i=0;i<p->m_usLen;i++)
        {
          g_ucaFileBuff[i + t_ulOffset] = p->m_ucpDat[i];
        }  
    }
    else if((p->m_tCd.FIN==1) && (p->m_tCd.FIR==1))
    {
        for(i=0;i<p->m_usLen;i++)
        {
            g_ucaFileBuff[i + t_ulOffset] = p->m_ucpDat[i];
        } 
				t_ucFlg = 1;
    }
    else
    {
        return 1;
    }
    
    pf=(ytFrame_T*)&g_tUart2Phy.m_ucaTx;
    pf->m_ucHead = YT_HEAD;
    pf->m_ulAddr = g_ptSystemConfiguration->m_tMatCom.m_usId;
    pf->m_ucCmd  = WIN_2017D_CFG + 0x80;
    pf->m_usLen  = 1;
    len  = WINS_2017D_FIX_FRAME+1;
    pf->m_ucpDat[0] = 0x01;
    p1= (uint8_t*)&pf->m_ucHead;
    p1++;
    t_usCrc = calcCrc16Modbus(0XFFFF,p1,len-1);
    pf->m_ucpDat[1] = t_usCrc % 256;
    pf->m_ucpDat[2] = t_usCrc / 256;
		uart2SendMsg(g_tUart2Phy.m_ucaTx,len+2);
		
		if(t_ucFlg==1)
		{
				t_ucFlg = 0;
				systemSetStructToCfgPage(CFG_FILE_EMBEDFLASH_STARTADDR,g_ucaFileBuff,CFG_BUFF_LEN); 
		}

		return 0;
}


/**
 * 读配置文件
 * @param:  NONE
 * @return: NONE
*/
uint8_t debugCmdRdCfgFile(uint8_t *pBuf,uint32_t ulIndex)
{
    uint16_t t_usCrc;
    uint8_t  *p1;
    uint16_t len;
	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t i;
    uint16_t t_usIndex,j;
    
    t_usIndex = 0;   
    
    for(j=0;j<CFG_FILE_EMBEDFLASH_TOTALPACKAGES;j++)
    {     
        pf=(ytFrame_T*)&g_tUart2Phy.m_ucaTx;
        if(j==0)
        {
            pf->m_tCd.FIN=0;
            pf->m_tCd.FIR=1;
            pf->m_tCd.Count=0;
        }
        else if(j==CFG_FILE_EMBEDFLASH_TOTALPACKAGES-1)
        {
            pf->m_tCd.FIN=1;
            pf->m_tCd.FIR=1; 
            pf->m_tCd.Count++;
        }
        else
        {
            pf->m_tCd.FIN=1;
            pf->m_tCd.FIR=0; 
            pf->m_tCd.Count++;
        }
				pf=(ytFrame_T*)&g_tUart2Phy.m_ucaTx;
				pf->m_ucHead = YT_HEAD;
        pf->m_ucCmd  = WIN_2017D_READCFG + 0x80;
        len = WINS_2017D_FIX_FRAME+CFG_FILE_PACKAGE;
        pf->m_usLen  = CFG_FILE_PACKAGE;
        p1= (uint8_t*)&pf->m_ucHead;
        p1++;
        for(i=0;i<CFG_FILE_PACKAGE;i++)
        {
            pf->m_ucpDat[i]=g_ucaCfgBuff[i+t_usIndex];
        }
        t_usCrc = calcCrc16Modbus(0XFFFF,p1,len-1);
        pf->m_ucpDat[CFG_FILE_PACKAGE] = t_usCrc % 256;
        pf->m_ucpDat[CFG_FILE_PACKAGE+1] = t_usCrc / 256;
				uart2SendMsg(g_tUart2Phy.m_ucaTx,len+2);
        t_usIndex +=CFG_FILE_PACKAGE;
    }
		return 0;
}

/**
 * 写配置初值
 * @param:  NONE
 * @return: NONE
*/
uint8_t debugCmdWrCfgInitFile(uint8_t *pBuf,uint32_t ulIndex)
{
    uint16_t t_usCrc;
    uint8_t  *p1;
    uint16_t len;

	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t i;
	  uint8_t t_ucFlg;
    
	  t_ucFlg = 0;
    p=(ytFrame_T*)(&pBuf[ulIndex]);  
    if(p->m_usLen!=CFG_FILE_PACKAGE)return 1;
    uint32_t t_ulOffset = (uint32_t)p->m_tCd.Count * CFG_FILE_PACKAGE;
    if(t_ulOffset + p->m_usLen > CFG_BUFF_LEN)
    {
        return 1;
    }
    if((p->m_tCd.FIN==0) && (p->m_tCd.FIR==1))//首帧
    {
        for(i=0;i<p->m_usLen;i++)
        {
          g_ucaFileBuff[i] = p->m_ucpDat[i];
        }
    }
    else if((p->m_tCd.FIN==0) && (p->m_tCd.FIR==0))
    {
        for(i=0;i<p->m_usLen;i++)
        {
          g_ucaFileBuff[i + t_ulOffset] = p->m_ucpDat[i];
        }
    }
    else if((p->m_tCd.FIN==1) && (p->m_tCd.FIR==0))
    {
        for(i=0;i<p->m_usLen;i++)
        {
          g_ucaFileBuff[i + t_ulOffset] = p->m_ucpDat[i];
        }  
    }
    else if((p->m_tCd.FIN==1) && (p->m_tCd.FIR==1))
    {
        for(i=0;i<p->m_usLen;i++)
        {
            g_ucaFileBuff[i + t_ulOffset] = p->m_ucpDat[i];
        } 
				t_ucFlg = 1;
    }
    else
    {
        return 1;
    }
    
    pf=(ytFrame_T*)&g_tUart2Phy.m_ucaTx;
    pf->m_ucHead = YT_HEAD;
    pf->m_ulAddr = g_ptSystemConfiguration->m_tMatCom.m_usId;
    pf->m_ucCmd  = WIN_2017D_CFG_INIT + 0x80;
    pf->m_usLen  = 1;
    len  = WINS_2017D_FIX_FRAME+1;
    pf->m_ucpDat[0] = 0x01;
    p1= (uint8_t*)&pf->m_ucHead;
    p1++;
    t_usCrc = calcCrc16Modbus(0XFFFF,p1,len-1);
    pf->m_ucpDat[1] = t_usCrc % 256;
    pf->m_ucpDat[2] = t_usCrc / 256;
		uart2SendMsg(g_tUart2Phy.m_ucaTx,len+2);
		
	  if(t_ucFlg==1)
		{
				t_ucFlg = 0;
				systemSetStructToCfgPage(BACKUP_CFG_FILE_EMBEDFLASH_STARTADDR,g_ucaFileBuff,CFG_BUFF_LEN);
		}
		return 0;
}


/**
 * 读配置文件初值
 * @param:  NONE
 * @return: NONE
*/
uint8_t debugCmdRdCfgInitFile(uint8_t *pBuf,uint32_t ulIndex)
{
    uint16_t t_usCrc;
    uint8_t  *p1;
    uint16_t len;
	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t i;
    uint16_t t_usIndex,j;
    
    t_usIndex = 0;   
    
    for(j=0;j<CFG_FILE_EMBEDFLASH_TOTALPACKAGES;j++)
    {     
        pf=(ytFrame_T*)&g_tUart2Phy.m_ucaTx;
        if(j==0)
        {
            pf->m_tCd.FIN=0;
            pf->m_tCd.FIR=1;
            pf->m_tCd.Count=0;
        }
        else if(j==CFG_FILE_EMBEDFLASH_TOTALPACKAGES-1)
        {
            pf->m_tCd.FIN=1;
            pf->m_tCd.FIR=1; 
            pf->m_tCd.Count++;
        }
        else
        {
            pf->m_tCd.FIN=1;
            pf->m_tCd.FIR=0; 
            pf->m_tCd.Count++;
        }
				pf=(ytFrame_T*)&g_tUart2Phy.m_ucaTx;
				pf->m_ucHead = YT_HEAD;
        pf->m_ucCmd  = WIN_2017D_READCFG_INIT + 0x80;
        len = WINS_2017D_FIX_FRAME+CFG_FILE_PACKAGE;
        pf->m_usLen  = CFG_FILE_PACKAGE;
        p1= (uint8_t*)&pf->m_ucHead;
        p1++;
        for(i=0;i<CFG_FILE_PACKAGE;i++)
        {
            pf->m_ucpDat[i]=*(char*)(BACKUP_CFG_FILE_EMBEDFLASH_STARTADDR+i+t_usIndex);
        }
        t_usCrc = calcCrc16Modbus(0XFFFF,p1,len-1);
        pf->m_ucpDat[CFG_FILE_PACKAGE] = t_usCrc % 256;
        pf->m_ucpDat[CFG_FILE_PACKAGE+1] = t_usCrc / 256;
				uart2SendMsg(g_tUart2Phy.m_ucaTx,len+2);
        t_usIndex +=CFG_FILE_PACKAGE;
    }
		return 0;
}


/**
 * 写序号
 * @param:  NONE
 * @return: NONE
*/
uint8_t debugCmdWrSerialNo(uint8_t *pBuf,uint32_t ulIndex)
{
	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t t_usCrc;
    uint8_t *p1;
    uint16_t i;
    uint16_t len;

    p=(ytFrame_T*)(&pBuf[ulIndex]);
    
    for(i=0;i<16;i++)
    {
        g_ucaFileBuff[i] =p->m_ucpDat[i];
    }
		systemSetBuffToCfgPage(SERNO_START_ADDR,g_ucaFileBuff,CFG_BUFF_LEN);
    for(i=0;i<16;i++) 
    {
        g_ucaSerialNumber[i] = g_ucaFileBuff[i];
    }
    pf=(ytFrame_T*)&g_tUart2Phy.m_ucaTx;
    pf->m_ucHead = YT_HEAD;
    pf->m_ulAddr = g_ptSystemConfiguration->m_tMatCom.m_usId;
    pf->m_ucCmd  = WIN_2017D_WRSERNO + 0x80;
    pf->m_usLen  = 1;
    len  = WINS_2017D_FIX_FRAME+1;
    pf->m_ucpDat[0] = 0x01;
    p1= (uint8_t*)&pf->m_ucHead;
    p1++;
    t_usCrc = calcCrc16Modbus(0XFFFF,p1,len-1);
    pf->m_ucpDat[1] = t_usCrc % 256;
    pf->m_ucpDat[2] = t_usCrc / 256;
		uart2SendMsg(g_tUart2Phy.m_ucaTx,len+2);
		return 0;
}

/**
 * 读系统信息
 * @param:  NONE
 * @return: NONE
*/
uint8_t debugCmdReadVersion(uint8_t *pBuf,uint32_t ulIndex)
{
	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t t_usCrc;
    uint8_t *p1;
    uint16_t i;
    uint16_t len;

    pf=(ytFrame_T*)&g_tUart2Phy.m_ucaTx;
    pf->m_ucHead = YT_HEAD;
    pf->m_ulAddr = g_ptSystemConfiguration->m_tMatCom.m_usId;
    pf->m_ucCmd  = WIN_2017D_VERSION + 0x80;
    pf->m_usLen  = 28;
    len = WINS_2017D_FIX_FRAME+28;
    memcpy(&pf->m_ucpDat[0],c_ucaHardware,2);
    memcpy(&pf->m_ucpDat[2],c_ucaFirmware,2);
    memcpy(&pf->m_ucpDat[4],c_ucaWireProtocolVersion,4);
    memcpy(&pf->m_ucpDat[8],c_ucaWirelessProtocolVersion,4);
	  for(i=0;i<16;i++)
    {
				pf->m_ucpDat[12+i]=*(char*)(SERNO_START_ADDR+i);
			  g_ucaSerialNumber[i]=*(char*)(SERNO_START_ADDR+i);
    }
    //memcpy(&pf->m_ucpDat[12],g_ucaSerialNumber,16);
    p1= (uint8_t*)&pf->m_ucHead;
    p1++;
    t_usCrc = calcCrc16Modbus(0XFFFF,p1,len-1);
    pf->m_ucpDat[28] = t_usCrc % 256;
    pf->m_ucpDat[29] = t_usCrc / 256;
		uart2SendMsg(g_tUart2Phy.m_ucaTx,len+2);
		return 0;
}

/**
 * 复位系统
 * @param:  NONE
 * @return: NONE
*/
uint8_t debugCmdRstMe(uint8_t *pBuf,uint32_t ulIndex)
{
	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t t_usCrc;
    uint8_t *p1;
    uint16_t i;
    uint16_t len;

	  p=(ytFrame_T*)(&pBuf[ulIndex]);
    if(p->m_ucpDat[0]==0x01)//系统复位
    {

    }
    pf=(ytFrame_T*)&g_tUart2Phy.m_ucaTx;
    pf->m_ucHead = YT_HEAD;
    pf->m_ulAddr = g_ptSystemConfiguration->m_tMatCom.m_usId;
    pf->m_ucCmd  = WIN_2017D_RST + 0x80;
    len = WINS_2017D_FIX_FRAME+1;
    pf->m_usLen  = 1;
    pf->m_ucpDat[0] = 0x01;
    p1= (uint8_t*)&pf->m_ucHead;
    p1++;
    t_usCrc = calcCrc16Modbus(0XFFFF,p1,len-1);
    pf->m_ucpDat[1] = t_usCrc % 256;
    pf->m_ucpDat[2] = t_usCrc / 256;
    uart2SendMsg(g_tUart2Phy.m_ucaTx,len+2);
    
     __set_FAULTMASK(1);//关闭所有中断
     NVIC_SystemReset();
    while(1);
		return 0;
}

/**
 * 设置零点
 * @param:  NONE
 * @return: NONE
*/
uint8_t debugCmdSetZero(uint8_t *pBuf,uint32_t ulIndex)
{
	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t t_usCrc;
    uint8_t *p1;
    uint16_t i;
    uint16_t len;

    systemSetBuffToFlash(ZERO_FILE_STARTADDR,&g_ucsPressureZero12Bit[0][0],TOTAL_SAMPLE_POINT);
	
    pf=(ytFrame_T*)&g_tUart2Phy.m_ucaTx;
    pf->m_ucHead = YT_HEAD;
    pf->m_ulAddr = g_ptSystemConfiguration->m_tMatCom.m_usId;
    pf->m_ucCmd  = WIN_2017D_RSTRATEING + 0x80;
    pf->m_usLen  = 1;
    len  = WINS_2017D_FIX_FRAME+1;
    pf->m_ucpDat[0] = 0x01;
    p1= (uint8_t*)&pf->m_ucHead;
    p1++;
    t_usCrc = calcCrc16Modbus(0XFFFF,p1,len-1);
    pf->m_ucpDat[1] = t_usCrc % 256;
    pf->m_ucpDat[2] = t_usCrc / 256;
		uart2SendMsg(g_tUart2Phy.m_ucaTx,len+2);
    return 0;
}

/**
 *  恢复零点
 * @param:  NONE
 * @return: NONE
*/ 
uint8_t debugCmdResetZero(uint8_t *pBuf,uint32_t ulIndex)
{
	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t t_usCrc;
    uint8_t *p1;
    uint16_t i;
    uint16_t len;

		memset(g_ucsPressureZero12Bit[0],0,sizeof(g_ucsPressureZero12Bit));
		systemSetBuffToFlash(ZERO_FILE_STARTADDR,&g_ucsPressureZero12Bit[0][0],TOTAL_SAMPLE_POINT);
    
    pf=(ytFrame_T*)&g_tUart2Phy.m_ucaTx;
    pf->m_ucHead = YT_HEAD;
    pf->m_ulAddr = g_ptSystemConfiguration->m_tMatCom.m_usId;
    pf->m_ucCmd  = WIN_2017D_RSTRATEING + 0x80;
    pf->m_usLen  = 1;
    len  = WINS_2017D_FIX_FRAME+1;
    pf->m_ucpDat[0] = 0x01;
    p1= (uint8_t*)&pf->m_ucHead;
    p1++;
    t_usCrc = calcCrc16Modbus(0XFFFF,p1,len-1);
    pf->m_ucpDat[1] = t_usCrc % 256;
    pf->m_ucpDat[2] = t_usCrc / 256;
		uart2SendMsg(g_tUart2Phy.m_ucaTx,len+2);

    return 0;
}

extern  uint16_t g_usSavCnt,g_usSavPoint;
/**
 * 写模式7
 * @param:  NONE
 * @return: NONE
*/ 
uint8_t debugCmdProtocolWin2017DWriteCaliMode7Parm(uint8_t *pBuf,uint32_t ulIndex)
{
    uint16_t t_usCrc;
    uint8_t  *p1;
    uint16_t len;

	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t i;
	  uint8_t t_ucFlg;
	  uint16_t t_usCnt;
	  uint32_t t_ulAddr,t_ulDiv;

		t_ucFlg = 0;
    p=(ytFrame_T*)(&pBuf[ulIndex]);  
    if(p->m_usLen!=CFG_FILE_PACKAGE)return 1;
    if((p->m_tCd.FIN==0) && (p->m_tCd.FIR==1))//首帧
    {
			  g_usSavCnt = 0;
			  g_usSavPoint = 0;
        for(i=0;i<p->m_usLen;i++)
        {
          g_ucaFileBuff[i] = p->m_ucpDat[i];
        }
				g_usSavCnt++;
    }
    else if((p->m_tCd.FIN==1) && (p->m_tCd.FIR==0))
    {
			  t_usCnt = p->m_tCd.Count % 8;
        for(i=0;i<p->m_usLen;i++)
        {
          g_ucaFileBuff[i+t_usCnt*CFG_FILE_PACKAGE] = p->m_ucpDat[i];
        } 
        g_usSavCnt++;				
    }
    else if((p->m_tCd.FIN==1) && (p->m_tCd.FIR==1))
    {
			  t_usCnt = p->m_tCd.Count % 8;
        for(i=0;i<p->m_usLen;i++)
        {
            g_ucaFileBuff[i+t_usCnt*CFG_FILE_PACKAGE] = p->m_ucpDat[i];
        } 
				g_usSavCnt++;		
				t_ucFlg = 1;
    }
		
		if(g_usSavCnt == 8 || t_ucFlg == 1)
		{
			  g_usSavCnt = 0;
			  t_ucFlg = 1;
			  if(g_usSavPoint < CALMODE7_FILE_PAGES)/*only write mode7 area*/
			  {
			      t_ulAddr = g_usSavPoint;
			      t_ulAddr = t_ulAddr * CFG_BUFF_LEN;
			      t_ulAddr = CALMODE7_ADDR_START + t_ulAddr;
			      systemSetBuffToCfgPage(t_ulAddr,g_ucaFileBuff,CFG_BUFF_LEN);
			  }
			  g_usSavPoint++;
		}
		pf=(ytFrame_T*)&g_tUart2Phy.m_ucaTx;
    pf->m_ucHead = YT_HEAD;
    pf->m_ulAddr = g_ptSystemConfiguration->m_tMatCom.m_usId;
    pf->m_ucCmd  = WIN_2017D_WRCALMODE7 + 0x80;
    pf->m_usLen  = 1;
    len  = WINS_2017D_FIX_FRAME+1;
    pf->m_ucpDat[0] = 0x01;
    p1= (uint8_t*)&pf->m_ucHead;
    p1++;
    t_usCrc = calcCrc16Modbus(0XFFFF,p1,len-1);
    pf->m_ucpDat[1] = t_usCrc % 256;
    pf->m_ucpDat[2] = t_usCrc / 256;
		uart2SendMsg(g_tUart2Phy.m_ucaTx,len+2);
    return 0;
}

/**
 * 读模式7
 * @param:  NONE
 * @return: NONE
*/ 

uint8_t debugCmdProtocolWin2017DReadCaliMode7Parm(uint8_t *pBuf,uint32_t ulIndex)
{
    uint16_t t_usCrc;
    uint8_t  *p1;
    uint16_t len;
	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t i;
    uint16_t t_usIndex,j;
    
    t_usIndex = 0;   
    
    for(j=0;j<CALI7_FILE_EMBEDFLASH_TOTALPACKAGES;j++)
    {     
        pf=(ytFrame_T*)&g_tUart2Phy.m_ucaTx;
        if(j==0)
        {
            pf->m_tCd.FIN=0;
            pf->m_tCd.FIR=1;
            pf->m_tCd.Count=0;
        }
        else if(j==CALI7_FILE_EMBEDFLASH_TOTALPACKAGES-1)
        {
            pf->m_tCd.FIN=1;
            pf->m_tCd.FIR=1; 
            pf->m_tCd.Count++;
        }
        else
        {
            pf->m_tCd.FIN=1;
            pf->m_tCd.FIR=0; 
            pf->m_tCd.Count++;
        }
				pf=(ytFrame_T*)&g_tUart2Phy.m_ucaTx;
				pf->m_ucHead = YT_HEAD;
        pf->m_ucCmd  = WIN_2017D_RDCALMODE7 + 0x80;
        len = WINS_2017D_FIX_FRAME+CFG_FILE_PACKAGE;
        pf->m_usLen  = CFG_FILE_PACKAGE;
        p1= (uint8_t*)&pf->m_ucHead;
        p1++;
        for(i=0;i<CFG_FILE_PACKAGE;i++)
        {
            pf->m_ucpDat[i]=*(uint8_t *)(CALMODE7_ADDR_START+i+j*128) ;//g_ucaCfgBuff[i+t_usIndex];
        }
        t_usCrc = calcCrc16Modbus(0XFFFF,p1,len-1);
        pf->m_ucpDat[CFG_FILE_PACKAGE] = t_usCrc % 256;
        pf->m_ucpDat[CFG_FILE_PACKAGE+1] = t_usCrc / 256;
				uart2SendMsg(g_tUart2Phy.m_ucaTx,len+2);
        t_usIndex +=CFG_FILE_PACKAGE;
				Delay_ms(5);
    } 
    return 0;
}

/*模式4标定文件下载状态: 当前正在拼装的页号(0xFFFFFFFF=无)与页内已收字节数*/
static uint32_t s_ulCali4Page = 0xFFFFFFFFUL;
static uint16_t s_usCali4Fill = 0;

/**
 * 写模式4标定文件
 * 上位机(ms128.exe, RWConfig.xml: 写模式4 CmdType=11/0x0B, BlockSize=256)把整个标定XML
 * 展开成字节流后按BlockSize分包下发, 控制码约定为:
 *     首帧   FIN=0, FIR=1, Count=0
 *     中间帧 FIN=1, FIR=0, Count=1..n-2   <-- 中间帧的FIN同样置1
 *     末帧   FIN=1, FIR=1, Count=n-1
 * 所以不能用FIN判断“文件结束”, 必须用Count×帧长算出本帧在文件中的绝对偏移;
 * 1KB页收满就立即写Flash, 只有末帧(FIN&&FIR)才把不足一页的尾部补0xFF写入。
 * 命令码: 0x0B(WIN_2017D_WRFILE) 与 0x20(WIN_2017D_WRCALMODE4) 都走这里。
 * 帧长允许128/256(按上位机BlockSize), 也允许上位机只发前若干包。
 */
uint8_t debugCmdProtocolWin2017DWriteCaliMode4Parm(uint8_t *pBuf,uint32_t ulIndex)
{
    uint16_t t_usCrc;
    uint8_t  *p1;
    uint16_t len;
    uint16_t t_usLen;
    uint16_t t_usOffset;
    ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t i;
    uint32_t t_ulOffset;
    uint32_t t_ulByte;
    uint32_t t_ulPage;
    uint8_t  t_ucLast;

    p=(ytFrame_T*)(&pBuf[ulIndex]);
    if((p->m_usLen == 0) || (p->m_usLen > CFG_BUFF_LEN))return 1;

    /*首帧(单帧文件也置FIR): 重新开始本次下载*/
    if(p->m_tCd.FIR==1)
    {
        s_ulCali4Page = 0xFFFFFFFFUL;
        s_usCali4Fill = 0;
        g_usSavCnt    = 0;
        g_usSavPoint  = 0;
    }
    /*真正的末帧: FIN与FIR同时置位*/
    t_ucLast = ((p->m_tCd.FIN==1) && (p->m_tCd.FIR==1)) ? 1 : 0;

    /*本帧数据在标定文件中的绝对偏移 = 帧序号 × 帧数据长度*/
    t_ulOffset = (uint32_t)p->m_tCd.Count * p->m_usLen;

    for(i=0;i<p->m_usLen;i++)
    {
        t_ulByte = t_ulOffset + i;
        if(t_ulByte >= CALMODE4_FILE_LEN)
        {
            break;                      /*超出模式4区(5120字节), 剩余数据丢弃*/
        }
        t_ulPage   = t_ulByte / CFG_BUFF_LEN;
        t_usOffset = (uint16_t)(t_ulByte % CFG_BUFF_LEN);

        if(t_ulPage != s_ulCali4Page)/*换页*/
        {
            if((s_ulCali4Page != 0xFFFFFFFFUL) && (s_usCali4Fill != 0))
            {
                /*上一页没收满(丢帧), 补0xFF写下去, 避免该页残留旧数据*/
                for(t_usLen=s_usCali4Fill;t_usLen<CFG_BUFF_LEN;t_usLen++)
                {
                    g_ucaFileBuff[t_usLen] = 0xFF;
                }
                systemSetBuffToCfgPage(CALMODE4_ADDR_START + s_ulCali4Page*CFG_BUFF_LEN,
                                       g_ucaFileBuff,CFG_BUFF_LEN);
            }
            s_ulCali4Page = t_ulPage;
            s_usCali4Fill = 0;
        }

        g_ucaFileBuff[t_usOffset] = p->m_ucpDat[i];
        if((uint16_t)(t_usOffset+1) > s_usCali4Fill)
        {
            s_usCali4Fill = (uint16_t)(t_usOffset+1);
        }

        if(s_usCali4Fill >= CFG_BUFF_LEN)/*整页收满: 立即写Flash*/
        {
            systemSetBuffToCfgPage(CALMODE4_ADDR_START + s_ulCali4Page*CFG_BUFF_LEN,
                                   g_ucaFileBuff,CFG_BUFF_LEN);
            s_ulCali4Page = 0xFFFFFFFFUL;
            s_usCali4Fill = 0;
            g_usSavPoint++;
        }
    }

    if(t_ucLast == 1)/*末帧: 写入不足一页的尾部*/
    {
        if((s_ulCali4Page != 0xFFFFFFFFUL) && (s_usCali4Fill != 0))
        {
            for(t_usLen=s_usCali4Fill;t_usLen<CFG_BUFF_LEN;t_usLen++)
            {
                g_ucaFileBuff[t_usLen] = 0xFF;
            }
            systemSetBuffToCfgPage(CALMODE4_ADDR_START + s_ulCali4Page*CFG_BUFF_LEN,
                                   g_ucaFileBuff,CFG_BUFF_LEN);
        }
        s_ulCali4Page = 0xFFFFFFFFUL;
        s_usCali4Fill = 0;
        g_usSavCnt    = 0;
    }

    pf=(ytFrame_T*)&g_tUart2Phy.m_ucaTx;
    pf->m_ucHead = YT_HEAD;
    pf->m_ulAddr = g_ptSystemConfiguration->m_tMatCom.m_usId;
    pf->m_ucCmd  = p->m_ucCmd + 0x80;/*0x0B->0x8B, 0x20->0xA0*/
    pf->m_usLen  = 1;
    len  = WINS_2017D_FIX_FRAME+1;
    pf->m_ucpDat[0] = 0x01;
    p1= (uint8_t*)&pf->m_ucHead;
    p1++;
    t_usCrc = calcCrc16Modbus(0XFFFF,p1,len-1);
    pf->m_ucpDat[1] = t_usCrc % 256;
    pf->m_ucpDat[2] = t_usCrc / 256;
    uart2SendMsg(g_tUart2Phy.m_ucaTx,len+2);
    return 0;
}

/**
 * Read calibration mode 4: 整个模式4区(5120字节)按 256 字节/包回读, 共20包
 */
uint8_t debugCmdProtocolWin2017DReadCaliMode4Parm(uint8_t *pBuf,uint32_t ulIndex)
{
    uint16_t t_usCrc;
    uint8_t  *p1;
    uint16_t len;
    ytFrame_T *pf;
    uint16_t i;
    uint16_t t_usIndex,j;
    uint32_t t_ulAddr;

    t_usIndex = 0;

    for(j=0;j<CALI4_FILE_EMBEDFLASH_TOTALPACKAGES;j++)
    {
        pf=(ytFrame_T*)&g_tUart2Phy.m_ucaTx;
        if(j==0)
        {
            pf->m_tCd.FIN=0;
            pf->m_tCd.FIR=1;
            pf->m_tCd.Count=0;
        }
        else if(j==CALI4_FILE_EMBEDFLASH_TOTALPACKAGES-1)
        {
            pf->m_tCd.FIN=1;
            pf->m_tCd.FIR=1;
            pf->m_tCd.Count++;
        }
        else
        {
            pf->m_tCd.FIN=1;
            pf->m_tCd.FIR=0;
            pf->m_tCd.Count++;
        }
        pf->m_ucHead = YT_HEAD;
        pf->m_ulAddr = g_ptSystemConfiguration->m_tMatCom.m_usId;
        pf->m_ucCmd  = WIN_2017D_RDCALMODE4 + 0x80;
        len = WINS_2017D_FIX_FRAME+CFG_FILE_PACKAGE;
        pf->m_usLen  = CFG_FILE_PACKAGE;
        p1= (uint8_t*)&pf->m_ucHead;
        p1++;
        t_ulAddr = CALMODE4_ADDR_START + (uint32_t)j*CFG_FILE_PACKAGE;/*按帧长步进, 原来固定128会把地址算错*/
        for(i=0;i<CFG_FILE_PACKAGE;i++)
        {
            pf->m_ucpDat[i]=*(uint8_t *)(t_ulAddr+i);
        }
        t_usCrc = calcCrc16Modbus(0XFFFF,p1,len-1);
        pf->m_ucpDat[CFG_FILE_PACKAGE] = t_usCrc % 256;
        pf->m_ucpDat[CFG_FILE_PACKAGE+1] = t_usCrc / 256;
        uart2SendMsg(g_tUart2Phy.m_ucaTx,len+2);
        t_usIndex += CFG_FILE_PACKAGE;
        Delay_ms(5);
    }
    return 0;
}
/**
 * 清除累加
 * @param:  NONE
 * @return: NONE
*/ 
uint8_t cmdProtocolWin2017DClrDuration(uint8_t *pBuf,uint32_t ulIndex)
{
    ytFrame_T *pf;
    uint8_t *p1;
    uint16_t len;
    uint16_t t_usCrc;

    systemClrDurationTime();
    pf=(ytFrame_T*)&g_tUart2Phy.m_ucaTx;
    pf->m_ucHead = YT_HEAD;
    pf->m_ulAddr = g_ptSystemConfiguration->m_tMatCom.m_usId;
    pf->m_ucCmd  = WIN_2017D_CLRDURATIONTIME + 0x80;
    pf->m_usLen  = 1;
    len = WINS_2017D_FIX_FRAME+1;
	  pf->m_tCd.FIN=1;
    pf->m_tCd.FIR=1;
    pf->m_ucpDat[0] = 0x01;
    p1= (uint8_t*)&pf->m_ucHead;;
    p1++;
    t_usCrc = calcCrc16Modbus(0XFFFF,p1,len-1);
    pf->m_ucpDat[1] = t_usCrc % 256;
    pf->m_ucpDat[2] = t_usCrc / 256;
	  uart2SendMsg(g_tUart2Phy.m_ucaTx,len+2);    

		return 0;
}

/**
 * 写计数器
 * @param:  NONE
 * @return: NONE
*/ 
extern uint32_t  g_ulDurationTime;//耐久时间
extern uint16_t  g_usDurationIndex;
uint8_t cmdProtocolWin2017DWrDuration(uint8_t *pBuf,uint32_t ulIndex)
{
    ytFrame_T *pf;
    ytFrame_T *p;
    uint8_t *p1;
    uint16_t len;
    uint16_t t_usCrc;

	  p=(ytFrame_T*)(&pBuf[ulIndex]);
    if(p->m_usLen!=4)return 1;
	  systemClrDurationTime();
    memcpy(&g_ulDurationTime,&p->m_ucpDat[0],4);
    g_ucWriteDurationTimeFlg = 1;

    pf=(ytFrame_T*)&g_tUart2Phy.m_ucaTx;
    pf->m_ucHead = YT_HEAD;
    pf->m_ulAddr = g_ptSystemConfiguration->m_tMatCom.m_usId;
    pf->m_ucCmd  = WIN_2017D_WRDURATIONTIME + 0x80;
    pf->m_usLen  = 1;
    len = WINS_2017D_FIX_FRAME+1;
	  pf->m_ucpDat[0] = 0x01;
    p1= (uint8_t*)&pf->m_ucHead;
    p1++;
		pf->m_tCd.FIN=1;
    pf->m_tCd.FIR=1;
    t_usCrc = calcCrc16Modbus(0XFFFF,p1,len-1);
    pf->m_ucpDat[1] = t_usCrc % 256;
    pf->m_ucpDat[2] = t_usCrc / 256;
    uart2SendMsg(g_tUart2Phy.m_ucaTx,len+2);    

		return 0;
}

uint8_t cmdProtocolWin2017DRdDuration(uint8_t *pBuf,uint32_t ulIndex)
{
    ytFrame_T *pf;
    uint8_t *p1;
    uint16_t t_usCrc;
    uint16_t len;
          
    pf=(ytFrame_T*)&g_tUart2Phy.m_ucaTx;
	
	  pf->m_tCd.FIN=1;
    pf->m_tCd.FIR=1;
    pf->m_tCd.Count=0;
    pf->m_ucHead = YT_HEAD;
    pf->m_ucCmd  = WIN_2017D_RDDURATIONTIME + 0x80;
    len = WINS_2017D_FIX_FRAME+4+CFG_FILE_DURATIONA_LEN;
    pf->m_usLen  = 4+CFG_FILE_DURATIONA_LEN;
    p1= (uint8_t*)&pf->m_ucHead;
    p1++;
    pf->m_ucpDat[0] =  (uint8_t)(g_ulDurationTime & 0x000000FF);
		pf->m_ucpDat[1] =  (uint8_t)((g_ulDurationTime & 0x0000FF00)>>8);
		pf->m_ucpDat[2] =  (uint8_t)((g_ulDurationTime & 0x00FF0000)>>16);
		pf->m_ucpDat[3] =  (uint8_t)((g_ulDurationTime & 0xFF000000)>>24);
		
		systemReadDurationTimeArray();
		memcpy(&pf->m_ucpDat[4],&g_ucDurationTimeBuff,CFG_FILE_DURATIONA_LEN);
    t_usCrc = calcCrc16Modbus(0XFFFF,p1,len-1);
    pf->m_ucpDat[4+CFG_FILE_DURATIONA_LEN] = t_usCrc % 256;
    pf->m_ucpDat[5+CFG_FILE_DURATIONA_LEN] = t_usCrc / 256;
    uart2SendMsg(g_tUart2Phy.m_ucaTx,len+2);
		return 0;
}

/**
 * 采样
 * @param:  NONE
 * @return: NONE
*/
uint8_t debugMtCmdStartSample(uint8_t *pBuf,uint32_t ulIndex)
{
    YotlivePt_T *t_ptylc;  
    uint16_t t_ust_usCrc;

    t_ptylc=(YotlivePt_T*)g_tUart2Phy.m_ucaTx;
									 
    t_ptylc->m_ucHeadH = 0x68;
    t_ptylc->m_ucHeadL = 0x68;
    t_ptylc->m_usAddr =  g_ptSystemConfiguration->m_tMatCom.m_usId;
    t_ptylc->m_ucCmd  =  0x83;
    t_ptylc->m_usLen  =  1;
    t_ptylc->m_ucpDat[0] = g_ucDebugSndFlg;
    t_ust_usCrc =   calcCrc16Modbus(0XFFFF,(uint8_t*)t_ptylc->m_ucpDat,t_ptylc->m_usLen);
    t_ptylc->m_ucpDat[t_ptylc->m_usLen]   = t_ust_usCrc % 256;
    t_ptylc->m_ucpDat[t_ptylc->m_usLen+1] = t_ust_usCrc / 256;  
    
    uart2SendMsg(g_tUart2Phy.m_ucaTx,9);
    return 0;
}

/**
 * 读系统
 * @param:  NONE
 * @return: NONE
*/
uint8_t debugMtCmdGetSystem(uint8_t *pBuf,uint32_t ulIndex)
{
    YotlivePt_T *t_ptylc;  
    uint16_t t_ust_usCrc;
    
    t_ptylc=(YotlivePt_T*)g_tUart2Phy.m_ucaTx;
									 
    t_ptylc->m_ucHeadH = 0x68;
    t_ptylc->m_ucHeadL = 0x68;
    t_ptylc->m_usAddr =  g_ptSystemConfiguration->m_tMatCom.m_usId;
    t_ptylc->m_ucCmd  =  0xfe;
    t_ptylc->m_usLen  =  58+4;
    memcpy(&t_ptylc->m_ucpDat[0],c_ucaHardware,2);
    memcpy(&t_ptylc->m_ucpDat[2],c_ucaFirmware,2);
    memcpy(&t_ptylc->m_ucpDat[4],g_ucaSerialNumber,16);
	  memset(&t_ptylc->m_ucpDat[20],0x00,6);
    
    t_ptylc->m_ucpDat[26] = g_ptSystemConfiguration->m_tMatSystemCfg.m_ulTick;
    t_ptylc->m_ucpDat[27] = g_ptSystemConfiguration->m_tMatSystemCfg.m_ucaDataFrame;
    t_ptylc->m_ucpDat[28] = TOTAL_SAMPLE_POINT % 256;
    t_ptylc->m_ucpDat[29] = TOTAL_SAMPLE_POINT / 256;
    t_ptylc->m_ucpDat[30] = ROW;
    t_ptylc->m_ucpDat[31] = COLUMN;
    t_ptylc->m_ucpDat[32] = 0xFF;
    t_ptylc->m_ucpDat[33] = 0x01; /*表示当前模式是采用以太网进行通信*/
     
    TransFloat2Bytes(g_ptSystemConfiguration->m_tMatSystemCfg.m_fTotalSensorArea, &t_ptylc->m_ucpDat[34]);
    TransFloat2Bytes(g_ptSystemConfiguration->m_tMatSystemCfg.m_fSensorSizeX, &t_ptylc->m_ucpDat[38]);
    TransFloat2Bytes(g_ptSystemConfiguration->m_tMatSystemCfg.m_fSensorSizeY, &t_ptylc->m_ucpDat[42]);
    TransFloat2Bytes(g_ptSystemConfiguration->m_tMatSystemCfg.m_fPressureRangeLow, &t_ptylc->m_ucpDat[46]);
    TransFloat2Bytes(g_ptSystemConfiguration->m_tMatSystemCfg.m_fPressureRangeHigh,&t_ptylc->m_ucpDat[50]);
    TransFloat2Bytes(g_ptSystemConfiguration->m_tMatSystemCfg.m_fBackUp, &t_ptylc->m_ucpDat[54]); 
		t_ptylc->m_ucpDat[58] = 0;
    t_ust_usCrc =   calcCrc16Modbus(0XFFFF,(uint8_t*)t_ptylc->m_ucpDat,t_ptylc->m_usLen);
    t_ptylc->m_ucpDat[t_ptylc->m_usLen]   = t_ust_usCrc % 256;
    t_ptylc->m_ucpDat[t_ptylc->m_usLen+1] = t_ust_usCrc / 256;  
    uart2SendMsg(g_tUart2Phy.m_ucaTx,t_ptylc->m_usLen+9);
    return 0;
}

/**
 * 广播
 * @param:  NONE
 * @return: NONE
*/
uint8_t debugMtCmdBrocast(uint8_t *pBuf,uint32_t ulIndex)
{
    YotlivePt_T *t_ptylc;  
    uint16_t t_ust_usCrc;
    
    t_ptylc=(YotlivePt_T*)g_tUart2Phy.m_ucaTx;
									 
    t_ptylc->m_ucHeadH = 0x68;
    t_ptylc->m_ucHeadL = 0x68;
    t_ptylc->m_usAddr =  g_ptSystemConfiguration->m_tMatCom.m_usId;
    t_ptylc->m_ucCmd  =  0xfF;
    t_ptylc->m_usLen  =  58+4;
    memcpy(&t_ptylc->m_ucpDat[0],c_ucaHardware,2);
    memcpy(&t_ptylc->m_ucpDat[2],c_ucaFirmware,2);
    memcpy(&t_ptylc->m_ucpDat[4],g_ucaSerialNumber,16);
	  memset(&t_ptylc->m_ucpDat[20],0x00,6);
    
    t_ptylc->m_ucpDat[26] = g_ptSystemConfiguration->m_tMatSystemCfg.m_ulTick;
    t_ptylc->m_ucpDat[27] = g_ptSystemConfiguration->m_tMatSystemCfg.m_ucaDataFrame;
    t_ptylc->m_ucpDat[28] = TOTAL_SAMPLE_POINT % 256;
    t_ptylc->m_ucpDat[29] = TOTAL_SAMPLE_POINT / 256;
    t_ptylc->m_ucpDat[30] = ROW;
    t_ptylc->m_ucpDat[31] = COLUMN;
    t_ptylc->m_ucpDat[32] = 0xFF;
    t_ptylc->m_ucpDat[33] = 0x01; /*表示当前模式是采用以太网进行通信*/
     
    TransFloat2Bytes(g_ptSystemConfiguration->m_tMatSystemCfg.m_fTotalSensorArea, &t_ptylc->m_ucpDat[34]);
    TransFloat2Bytes(g_ptSystemConfiguration->m_tMatSystemCfg.m_fSensorSizeX, &t_ptylc->m_ucpDat[38]);
    TransFloat2Bytes(g_ptSystemConfiguration->m_tMatSystemCfg.m_fSensorSizeY, &t_ptylc->m_ucpDat[42]);
    TransFloat2Bytes(g_ptSystemConfiguration->m_tMatSystemCfg.m_fPressureRangeLow, &t_ptylc->m_ucpDat[46]);
    TransFloat2Bytes(g_ptSystemConfiguration->m_tMatSystemCfg.m_fPressureRangeHigh,&t_ptylc->m_ucpDat[50]);
    TransFloat2Bytes(g_ptSystemConfiguration->m_tMatSystemCfg.m_fBackUp, &t_ptylc->m_ucpDat[54]); 
		t_ptylc->m_ucpDat[58] = 0;
    t_ust_usCrc =   calcCrc16Modbus(0XFFFF,(uint8_t*)t_ptylc->m_ucpDat,t_ptylc->m_usLen);
    t_ptylc->m_ucpDat[t_ptylc->m_usLen]   = t_ust_usCrc % 256;
    t_ptylc->m_ucpDat[t_ptylc->m_usLen+1] = t_ust_usCrc / 256;  
    uart2SendMsg(g_tUart2Phy.m_ucaTx,t_ptylc->m_usLen+9);
    return 0;
}

/**
 * 发送数据
 * @param:  NONE
 * @return: NONE
*/
extern uint16_t g_usUsbFrameCount;
extern uint8_t t_ucaSampleBuff[ROW * COLUMN * 2];
uint8_t debugMtCmdSndDat(void)
{
	
    YotlivePt_T *t_ptylc;  
    uint16_t t_ust_usCrc;
    
	  uint16_t t_usLen,t_usTempLen,t_usDiv,t_usMod,t_usRawLen,t_usTemp,t_usTempH,t_usTempL;
    uint16_t *ucp,i,j,t_usCrc;
	  

    switch(g_ptSystemConfiguration->m_tMatSystemCfg.m_ucaDataFrame)
    {
    case 1://8bit
	  t_ptylc=(YotlivePt_T*)g_tUart2Phy.m_ucaTx;
    t_ptylc->m_ucHeadH = 0x68;
    t_ptylc->m_ucHeadL = 0x68;
    t_ptylc->m_usAddr =  g_ptSystemConfiguration->m_tMatCom.m_usId;
    t_ptylc->m_ucCmd = 0X84;
    t_ptylc->m_usLen = sizeof(g_ucaPressureRaw)+3;
    t_ptylc->m_ucpDat[0]=g_usUsbFrameCount%256;
    t_ptylc->m_ucpDat[1]=g_usUsbFrameCount/256;
    t_ptylc->m_ucpDat[2]=1;
    
    memcpy(&t_ptylc->m_ucpDat[3],g_ucaPressureRaw,sizeof(g_ucaPressureRaw));
    t_ust_usCrc = calcCrc16Modbus(0XFFFF,(uint8_t*)t_ptylc->m_ucpDat,t_ptylc->m_usLen);
    t_ptylc->m_ucpDat[t_ptylc->m_usLen] = t_ust_usCrc % 256;
    t_ptylc->m_ucpDat[t_ptylc->m_usLen+1] = t_ust_usCrc / 256;
    g_usUsbFrameCount++;
		uart2SendMsg(g_tUart2Phy.m_ucaTx,t_ptylc->m_usLen+9);
      break;
    case 2://12bit        
			t_ptylc=(YotlivePt_T*)g_tUart2Phy.m_ucaTx;
			t_ptylc->m_ucHeadH = 0x68;
			t_ptylc->m_ucHeadL = 0x68;
			t_ptylc->m_usAddr = g_ptSystemConfiguration->m_tMatCom.m_usId;
			t_ptylc->m_ucCmd = 0X84;
			t_usLen = TOTAL_SAMPLE_POINT*2;
			t_usTempLen = t_usLen * 3;
			t_usDiv = t_usTempLen / 4;
			t_usMod = t_usTempLen % 4;
			if(t_usMod !=0)
			{
				t_usRawLen = t_usDiv + 1;
			}
			else
			{
				t_usRawLen = t_usDiv;
			}
			t_usTemp = ((t_usLen - t_usMod) * 3)/4;
			t_ptylc->m_usLen = t_usRawLen + 3;
			t_ptylc->m_ucpDat[0]=g_usUsbFrameCount%256;
			t_ptylc->m_ucpDat[1]=g_usUsbFrameCount/256;
							 
			ucp = (uint16_t *)&g_ucaPressureRaw12Bit[0][0];
			for(i=0,j=0;i<t_usTemp;i+=3,j=j+2)
			{
					t_usTempH = (*(ucp+j)) << 4;
					t_usTempH = t_usTempH & 0xff00;
					t_ucaSampleBuff[i] =  (uint8_t)(t_usTempH >> 8);
					t_usTempH = *(ucp+j)  & 0x000F;
					t_usTempH = t_usTempH<<4;
					t_usTempL = (*(ucp+j+1))  & 0x0F00;
					t_usTempL = t_usTempL >> 8;
					t_ucaSampleBuff[i+1] =  (uint8_t)(t_usTempH + t_usTempL);
					t_usTempH = (*(ucp+j+1)) & 0x00FF;
					t_ucaSampleBuff[i+2] = (uint8_t)t_usTempH;
			}
			if(t_usMod !=0)
			{
					t_usTempH = *(ucp+j)  << 4;
					t_usTempH = t_usTempH & 0xff00;
					t_ucaSampleBuff[t_usTemp] =  (uint8_t)(t_usTempH >> 8);
					t_usTempH =( *(ucp+j)) & 0x000F;
					t_usTempH = t_usTempH<<4;
					t_ucaSampleBuff[t_usTemp+1] = (uint8_t)t_usTempH;
			}
			t_ptylc->m_ucpDat[2]=2;
			memcpy(&t_ptylc->m_ucpDat[3],t_ucaSampleBuff,t_usRawLen);
			t_usCrc = calcCrc16Modbus(0XFFFF,(uint8_t*)t_ptylc->m_ucpDat,t_ptylc->m_usLen);
			t_ptylc->m_ucpDat[t_ptylc->m_usLen] = t_usCrc % 256;
			t_ptylc->m_ucpDat[t_ptylc->m_usLen+1] = t_usCrc / 256;
			g_usUsbFrameCount++;
		 uart2SendMsg(g_tUart2Phy.m_ucaTx,t_ptylc->m_usLen+9);
      break;      
    }
    return 0;
}


uint8_t getDataFromUart2(void)
{
	  
		uint8_t t_ucaDeal[USART2_RXBUF_LEN];
	  ytFrame_T *t_ptYt;
	  uint32_t i,t_ulIndex,t_ulSts,t_ulDt;
	  uint8_t t_ucFlg,t_ucFinishFlg;
	  uint8_t *ptData;
	  uint16_t t_usCrc;
	  uint16_t t_usAcc;
	  YotlivePt_T *t_ptYlc;

		if(USART_GetITStatus(USART2, USART_INT_ORERR))
		{
				t_ulSts = USART2->STS;
				t_ulDt  = USART2->DT;
		}

	  if(g_tUart2Phy.m_ucReceived)
    {
		    WDTCLR();
			  g_tUart2Phy.m_ucReceived = 0;
			  if(g_tUart2Phy.m_ulRxLen> 544) return 0;/*标定文件写入帧为256字节数据(共268字节),原144会整帧丢弃*/
			  memcpy(t_ucaDeal,g_tUart2Phy.m_ucaRx,g_tUart2Phy.m_ulRxLen);
				t_ucFlg = 0;
				for(i=0;i<4;i++)
				{
						if(t_ucaDeal[i]==YT_HEAD && t_ucaDeal[i+1]==YT_HEAD)
						{
								t_ucFlg = 1;
								t_ulIndex = i;
								break;
						}
				}
				
				if(t_ucFlg==1)//找到报文头
				{
					  t_ucFinishFlg = 0;
					  t_ptYlc = (YotlivePt_T*)(&t_ucaDeal[i]);
					  if(t_ptYlc->m_usLen > 512) return 0;
					  if(t_ptYlc->m_ucHeadH==YT_HEAD && t_ptYlc->m_ucHeadL==YT_HEAD)
						{
								if((t_ptYlc->m_usAddr==g_ptSystemConfiguration->m_tMatCom.m_usId) && (t_ptYlc->m_usAddr!=0xffff))
								{
											switch(t_ptYlc->m_ucCmd)
											{
											case 0x03://启动停止采样
													if(t_ptYlc->m_ucpDat[0]==0x01)
													{
														 g_ucDebugSndFlg = 1;
														 debugMtCmdStartSample(t_ucaDeal,t_ulIndex); 
													}
													else
													{
														 g_ucDebugSndFlg = 0;
														 debugMtCmdStartSample(t_ucaDeal,t_ulIndex); 
													}
													t_ucFinishFlg = 1;
											break;
											case 0x7e://系统信息
													g_ucDebugSndFlg = 0;
													debugMtCmdGetSystem(t_ucaDeal,t_ulIndex); 
											    t_ucFinishFlg = 1;
											break;
											case 0x7F:
													g_ucDebugSndFlg = 0;
													debugMtCmdBrocast(t_ucaDeal,t_ulIndex);
													t_ucFinishFlg = 1;
											break;  
										}
								}
								if(t_ptYlc->m_usAddr==0xffff)
								{
											switch(t_ptYlc->m_ucCmd)
											{
													case 0x7F:
													g_ucDebugSndFlg = 0;	
													debugMtCmdBrocast(t_ucaDeal,t_ulIndex); 
													t_ucFinishFlg = 1;
													break;
											}
								 } 
						}
						if(t_ucFinishFlg == 1)
						{
								t_ucFinishFlg = 0;
							  LEDRED_SET();
							   return 0;
						}
					}						
					
					t_ucFlg = 0;
					for(i=0;i<8;i++)
					{
							if(t_ucaDeal[i]==YT_HEAD)
							{
										t_ucFlg = 1;
										t_ulIndex = i;
										break;
							}
					}
					if(t_ucFlg==1)
					{
							t_ptYt = (ytFrame_T*)(&t_ucaDeal[t_ulIndex]);
							if(t_ptYt->m_usLen > 512) return 0;
							t_usCrc = calcCrc16Modbus(0XFFFF,&t_ucaDeal[t_ulIndex+1],t_ptYt->m_usLen+WINS_2017D_FIX_FRAME-1);

							if(((t_usCrc / 256) == t_ptYt->m_ucpDat[t_ptYt->m_usLen+1]) && ((t_usCrc % 256) == t_ptYt->m_ucpDat[t_ptYt->m_usLen]))
							{
										g_ucSystemDebugLevel = 0;
										if((t_ptYt->m_ulAddr==g_ptSystemConfiguration->m_tMatCom.m_usId) || (t_ptYt->m_ulAddr==0xffff))	
										{
												switch(t_ptYt->m_ucCmd)
												{
												case WIN_2017D_BRO:             //广播帧，问地址
													debugCmdBro(t_ucaDeal,t_ulIndex);
												  t_ucFinishFlg = 1;
												break;
												case WIN_2017D_CFG:             //更新配置文件
													debugCmdWrCfgFile(t_ucaDeal,t_ulIndex);
												  t_ucFinishFlg = 1;
												break;  
												case WIN_2017D_READCFG:         //读取配置文件
													debugCmdRdCfgFile(t_ucaDeal,t_ulIndex);
												  t_ucFinishFlg = 1;
												break; 
												case WIN_2017D_CFG_INIT:        //更新配置文件
													debugCmdWrCfgInitFile(t_ucaDeal, t_ulIndex);
												  t_ucFinishFlg = 1;
												break;  
												case WIN_2017D_READCFG_INIT:    //读取配置文件
													debugCmdRdCfgInitFile(t_ucaDeal, t_ulIndex);
												  t_ucFinishFlg = 1;
												break;
												case WIN_2017D_WRSERNO:        //写序列号
													debugCmdWrSerialNo(t_ucaDeal, t_ulIndex);
												  t_ucFinishFlg = 1;
												break;
												case WIN_2017D_VERSION:        //读取版本号
													debugCmdReadVersion(t_ucaDeal, t_ulIndex);
												  t_ucFinishFlg = 1;
												break;  
												case WIN_2017D_RST:            //复位系统
													debugCmdRstMe(t_ucaDeal, t_ulIndex);
												  t_ucFinishFlg = 1;
												break; 
												case WIN_2017D_RATEING:            //系统调零
													debugCmdSetZero(t_ucaDeal, t_ulIndex);
												  t_ucFinishFlg = 1;
												break; 
												case WIN_2017D_RSTRATEING:            //恢复调零
													debugCmdResetZero(t_ucaDeal, t_ulIndex);
												  t_ucFinishFlg = 1;
												break;
												case WIN_2017D_WRCALMODE7:
													debugCmdProtocolWin2017DWriteCaliMode7Parm(t_ucaDeal, t_ulIndex);
												   t_ucFinishFlg = 1;
												break;
												case WIN_2017D_RDCALMODE7:
													debugCmdProtocolWin2017DReadCaliMode7Parm(t_ucaDeal, t_ulIndex);
												   t_ucFinishFlg = 1;
												break;
												case WIN_2017D_WRFILE:
													debugCmdProtocolWin2017DWriteCaliMode4Parm(t_ucaDeal, t_ulIndex);
												   t_ucFinishFlg = 1;
												break;
												case WIN_2017D_WRCALMODE4:
													debugCmdProtocolWin2017DWriteCaliMode4Parm(t_ucaDeal, t_ulIndex);
												   t_ucFinishFlg = 1;
												break;
												case WIN_2017D_RDCALMODE4:
													debugCmdProtocolWin2017DReadCaliMode4Parm(t_ucaDeal, t_ulIndex);
												   t_ucFinishFlg = 1;
												break;
												case WIN_2017D_CLRDURATIONTIME:
													cmdProtocolWin2017DClrDuration(t_ucaDeal, t_ulIndex);
												  t_ucFinishFlg = 1;
													break;
												case WIN_2017D_RDDURATIONTIME:
													cmdProtocolWin2017DRdDuration(t_ucaDeal, t_ulIndex);
												  t_ucFinishFlg = 1;
													break;						
												case WIN_2017D_WRDURATIONTIME:
													cmdProtocolWin2017DWrDuration(t_ucaDeal, t_ulIndex);
												  t_ucFinishFlg = 1;
													break;									
											case CMD_SET_UNCALIBRATED: //原始值
													g_ptSystemConfiguration->m_tMatSystemCfg.m_ucCaliMode = 0;
													easy68Response(CMD_SET_UNCALIBRATED, 1, true);
													break;
												case CMD_SET_CALIBRATED: //标定值：本工程只有模式4(原为模式7，已无实现)
													g_ptSystemConfiguration->m_tMatSystemCfg.m_ucCaliMode = 4;
													easy68Response(CMD_SET_CALIBRATED, 1, true);
													break;
												case CMD_SET_CALIBRATED_MODE4: //mode4
													g_ptSystemConfiguration->m_tMatSystemCfg.m_ucCaliMode = 4;
													easy68Response(CMD_SET_CALIBRATED_MODE4, 1, true);
												break;
												}
										}
										if(t_ucFinishFlg == 1)
										{
												t_ucFinishFlg = 0;
											  LEDRED_SET();
											  return 0;
										}
							}
						}
					LEDRED_SET();
			}
}



