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

uint8_t  g_ucaFileBuff[CFG_BUFF_LEN];
uint8_t g_ucYtSndFlg;

extern uint8_t  g_ucaFileBuff[CFG_BUFF_LEN];
extern uint16_t g_ucaPressureRaw12Bit[ROW][COLUMN];
uint8_t g_ucrsYtSndFlg;

extern void easy68Response(uint8_t reqCode, uint8_t data, bool withData);
//
/**
 * 广播
 * @param:  NONE
 * @return: NONE
*/
uint8_t cmdYtBro(uint8_t *pBuf,uint32_t ulIndex)
{
    ytFrame_T *pf;
    uint16_t t_usCrc;
    uint8_t  *p1;
    uint16_t len;
	  uint8_t t_ucaTx[USART2_TXBUF_LEN];
    
    pf=(ytFrame_T*)&t_ucaTx;
    pf->m_ucHead = YT_HEAD;
    pf->m_ulAddr = g_ptSystemConfiguration->m_tMatCom.m_usId;
    pf->m_ucCmd  = WIN_2017D_BRO + 0x80;
    pf->m_usLen  = 12;
    len      = WINS_2017D_FIX_FRAME+12;
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
		uart1SendMsg(t_ucaTx,len+2);
		return 0;
}

/**
 * 写配置文件
 * @param:  NONE
 * @return: NONE
*/
uint8_t cmdYtWrCfgFile(uint8_t *pBuf,uint32_t ulIndex)
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
    if((p->m_tCd.FIN==0) && (p->m_tCd.FIR==1))//首帧
    {
        for(i=0;i<p->m_usLen;i++)
        {
          g_ucaFileBuff[i] = p->m_ucpDat[i];
        }
    }
    else if((p->m_tCd.FIN==1) && (p->m_tCd.FIR==0))
    {
        for(i=0;i<p->m_usLen;i++)
        {
          g_ucaFileBuff[i+p->m_tCd.Count*CFG_FILE_PACKAGE] = p->m_ucpDat[i];
        }  
    }
    else if((p->m_tCd.FIN==1) && (p->m_tCd.FIR==1))
    {
        for(i=0;i<p->m_usLen;i++)
        {
            g_ucaFileBuff[i+p->m_tCd.Count*CFG_FILE_PACKAGE] = p->m_ucpDat[i];
        } 
				t_ucFlg = 1;
    }
    
    pf=(ytFrame_T*)&t_ucaTx;
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
		uart1SendMsg(t_ucaTx,len+2);
		
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
uint8_t cmdYtRdCfgFile(uint8_t *pBuf,uint32_t ulIndex)
{
    uint16_t t_usCrc;
    uint8_t  *p1;
    uint16_t len;
	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t i;
    uint16_t t_usIndex,j;
	  uint8_t t_ucaTx[USART2_TXBUF_LEN];
    
    t_usIndex = 0;   
    
    for(j=0;j<CFG_FILE_EMBEDFLASH_TOTALPACKAGES;j++)
    {     
        pf=(ytFrame_T*)&t_ucaTx;
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
		pf=(ytFrame_T*)&t_ucaTx;
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
				uart1SendMsg(t_ucaTx,len+2);
        t_usIndex +=CFG_FILE_PACKAGE;
    }
		return 0;
}

/**
 * 写配置
 * @param:  NONE
 * @return: NONE
*/
uint8_t cmdYtWrCfgInitFile(uint8_t *pBuf,uint32_t ulIndex)
{
    uint16_t t_usCrc;
    uint8_t  *p1;
    uint16_t len;

	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t i;
	  uint8_t t_ucaTx[USART2_TXBUF_LEN];
	  uint8_t t_ucFlg;
    
	  t_ucFlg = 0;
    p=(ytFrame_T*)(&pBuf[ulIndex]);  
    if(p->m_usLen!=CFG_FILE_PACKAGE)return 1;
    if((p->m_tCd.FIN==0) && (p->m_tCd.FIR==1))//首帧
    {
        for(i=0;i<p->m_usLen;i++)
        {
          g_ucaFileBuff[i] = p->m_ucpDat[i];
        }
    }
    else if((p->m_tCd.FIN==1) && (p->m_tCd.FIR==0))
    {
        for(i=0;i<p->m_usLen;i++)
        {
          g_ucaFileBuff[i+p->m_tCd.Count*CFG_FILE_PACKAGE] = p->m_ucpDat[i];
        }  
    }
    else if((p->m_tCd.FIN==1) && (p->m_tCd.FIR==1))
    {
        for(i=0;i<p->m_usLen;i++)
        {
            g_ucaFileBuff[i+p->m_tCd.Count*CFG_FILE_PACKAGE] = p->m_ucpDat[i];
        } 
				t_ucFlg = 1;
    }
    
    pf=(ytFrame_T*)&t_ucaTx;
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
		uart1SendMsg(t_ucaTx,len+2);
		
	  if(t_ucFlg==1)
		{
				t_ucFlg = 0;
				systemSetStructToCfgPage(BACKUP_CFG_FILE_EMBEDFLASH_STARTADDR,g_ucaFileBuff,CFG_BUFF_LEN);
		}
		return 0;
}


/**
 * 读配置问及那
 * @param:  NONE
 * @return: NONE
*/
uint8_t cmdYtRdCfgInitFile(uint8_t *pBuf,uint32_t ulIndex)
{
    uint16_t t_usCrc;
    uint8_t  *p1;
    uint16_t len;
	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t i;
    uint16_t t_usIndex,j;
	  uint8_t t_ucaTx[USART2_TXBUF_LEN];
    
    t_usIndex = 0;   
    
    for(j=0;j<CFG_FILE_EMBEDFLASH_TOTALPACKAGES;j++)
    {     
        pf=(ytFrame_T*)&t_ucaTx;
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
				pf=(ytFrame_T*)&t_ucaTx;
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
				uart1SendMsg(t_ucaTx,len+2);
        t_usIndex +=CFG_FILE_PACKAGE;
    }
		return 0;
}


/**
 * 写序号
 * @param:  NONE
 * @return: NONE
*/
uint8_t cmdYtWrSerialNo(uint8_t *pBuf,uint32_t ulIndex)
{
	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t t_usCrc;
    uint8_t *p1;
    uint16_t i;
    uint16_t len;
	  uint8_t t_ucaTx[USART2_TXBUF_LEN];

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
    pf=(ytFrame_T*)&t_ucaTx;
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
		uart1SendMsg(t_ucaTx,len+2);
		return 0;
}

/**
 * 读系统信息
 * @param:  NONE
 * @return: NONE
*/
uint8_t cmdYtReadVersion(uint8_t *pBuf,uint32_t ulIndex)
{
	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t t_usCrc;
    uint8_t *p1;
    uint16_t i;
    uint16_t len;
	  uint8_t t_ucaTx[USART2_TXBUF_LEN];

    pf=(ytFrame_T*)&t_ucaTx;
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
		uart1SendMsg(t_ucaTx,len+2);
		return 0;
}

/**
 * 复位系统
 * @param:  NONE
 * @return: NONE
*/
uint8_t cmdYtRstMe(uint8_t *pBuf,uint32_t ulIndex)
{
	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t t_usCrc;
    uint8_t *p1;
    uint16_t i;
    uint16_t len;
	  uint8_t t_ucaTx[USART2_TXBUF_LEN];

	  p=(ytFrame_T*)(&pBuf[ulIndex]);
    if(p->m_ucpDat[0]==0x01)//系统复位
    {

    }
    pf=(ytFrame_T*)&t_ucaTx;
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
    uart1SendMsg(t_ucaTx,len+2);
    
     __set_FAULTMASK(1);//关闭所有中断
     NVIC_SystemReset();
    while(1);
		return 0;
}



//-----------------matlab 通信-------------------//
/**
 * 采样
 * @param:  NONE
 * @return: NONE
*/
uint8_t usbCmdStartSample(uint8_t *pBuf,uint32_t ulIndex)
{
    YotlivePt_T *t_ptylc;  
    uint16_t t_ust_usCrc;
	  uint8_t t_ucaTx[USART2_TXBUF_LEN];

    t_ptylc=(YotlivePt_T*)t_ucaTx;
									 
    t_ptylc->m_ucHeadH = 0x68;
    t_ptylc->m_ucHeadL = 0x68;
    t_ptylc->m_usAddr =  g_ptSystemConfiguration->m_tMatCom.m_usId;
    t_ptylc->m_ucCmd  =  0x83;
    t_ptylc->m_usLen  =  1;
    t_ptylc->m_ucpDat[0] = g_ucYtSndFlg;
    t_ust_usCrc =   calcCrc16Modbus(0XFFFF,(uint8_t*)t_ptylc->m_ucpDat,t_ptylc->m_usLen);
    t_ptylc->m_ucpDat[t_ptylc->m_usLen]   = t_ust_usCrc % 256;
    t_ptylc->m_ucpDat[t_ptylc->m_usLen+1] = t_ust_usCrc / 256;  
    
    uart1SendMsg(t_ucaTx,9);
    return 0;
}


/**
 * 读系统
 * @param:  NONE
 * @return: NONE
*/
uint8_t usbCmdGetSystem(uint8_t *pBuf,uint32_t ulIndex)
{
    YotlivePt_T *t_ptylc;  
    uint16_t t_ust_usCrc;
    uint8_t t_ucaTx[USART2_TXBUF_LEN];
    
    t_ptylc=(YotlivePt_T*)t_ucaTx;
									 
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
    uart1SendMsg(t_ucaTx,t_ptylc->m_usLen+9);
    return 0;
}

/**
 * 广播
 * @param:  NONE
 * @return: NONE
*/
uint8_t usbCmdBrocast(uint8_t *pBuf,uint32_t ulIndex)
{
    YotlivePt_T *t_ptylc;  
    uint16_t t_ust_usCrc;
    uint8_t t_ucaTx[USART2_TXBUF_LEN];
    
    t_ptylc=(YotlivePt_T*)t_ucaTx;
									 
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
    uart1SendMsg(t_ucaTx,t_ptylc->m_usLen+9);
    return 0;
}

/**
 * 发送数据
 * @param:  NONE
 * @return: NONE
*/
uint16_t g_usUsbFrameCount;
uint8_t t_ucaSampleBuff[ROW * COLUMN * 2];
uint8_t usbCmdSndDat(void)
{
	
    YotlivePt_T *t_ptylc;  
    uint16_t t_ust_usCrc;
    
	  uint16_t t_usLen,t_usTempLen,t_usDiv,t_usMod,t_usRawLen,t_usTemp,t_usTempH,t_usTempL;
    uint16_t *ucp,i,j,t_usCrc;
	  

    switch(g_ptSystemConfiguration->m_tMatSystemCfg.m_ucaDataFrame)
    {
    case 1://8bit
	  t_ptylc=(YotlivePt_T*)g_tUart1Phy.m_ucaTx;
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
		uart1SendMsg(g_tUart1Phy.m_ucaTx,t_ptylc->m_usLen+9);
      break;
    case 2://12bit        
			t_ptylc=(YotlivePt_T*)g_tUart1Phy.m_ucaTx;
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
		  uart1SendMsg(g_tUart1Phy.m_ucaTx,t_ptylc->m_usLen+9);
      break;      
    }
    return 0;
}

//

/**
 * 广播
 * @param:  NONE
 * @return: NONE
*/
uint8_t rscmdYtBro(uint8_t *pBuf,uint32_t ulIndex)
{
    ytFrame_T *pf;
    uint16_t t_usCrc;
    uint8_t  *p1;
    uint16_t len;
	  uint8_t t_ucaTx[USART1_TXBUF_LEN];
    
    pf=(ytFrame_T*)&t_ucaTx;
    pf->m_ucHead = YT_HEAD;
    pf->m_ulAddr = g_ptSystemConfiguration->m_tMatCom.m_usId;
    pf->m_ucCmd  = WIN_2017D_BRO + 0x80;
    pf->m_usLen  = 12;
    len      = WINS_2017D_FIX_FRAME+12;
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
		uart1SendMsg(t_ucaTx,len+2);
		return 0;
}

/**
 * 写配置文件
 * @param:  NONE
 * @return: NONE
*/
uint8_t rscmdYtWrCfgFile(uint8_t *pBuf,uint32_t ulIndex)
{
    uint16_t t_usCrc;
    uint8_t  *p1;
    uint16_t len;

	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t i;
	  uint8_t t_ucFlg;
	  uint8_t t_ucaTx[USART1_TXBUF_LEN];
    
	  t_ucFlg = 0;
    p=(ytFrame_T*)(&pBuf[ulIndex]);  
    if(p->m_usLen!=CFG_FILE_PACKAGE)return 1;
    if((p->m_tCd.FIN==0) && (p->m_tCd.FIR==1))//首帧
    {
        for(i=0;i<p->m_usLen;i++)
        {
          g_ucaFileBuff[i] = p->m_ucpDat[i];
        }
    }
    else if((p->m_tCd.FIN==1) && (p->m_tCd.FIR==0))
    {
        for(i=0;i<p->m_usLen;i++)
        {
          g_ucaFileBuff[i+p->m_tCd.Count*CFG_FILE_PACKAGE] = p->m_ucpDat[i];
        }  
    }
    else if((p->m_tCd.FIN==1) && (p->m_tCd.FIR==1))
    {
        for(i=0;i<p->m_usLen;i++)
        {
            g_ucaFileBuff[i+p->m_tCd.Count*CFG_FILE_PACKAGE] = p->m_ucpDat[i];
        } 
				t_ucFlg = 1;
    }
    
    pf=(ytFrame_T*)&t_ucaTx;
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
		uart1SendMsg(t_ucaTx,len+2);
		
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
uint8_t rscmdYtRdCfgFile(uint8_t *pBuf,uint32_t ulIndex)
{
    uint16_t t_usCrc;
    uint8_t  *p1;
    uint16_t len;
	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t i;
    uint16_t t_usIndex,j;
	  uint8_t t_ucaTx[USART1_TXBUF_LEN];
    
    t_usIndex = 0;   
    
    for(j=0;j<CFG_FILE_EMBEDFLASH_TOTALPACKAGES;j++)
    {     
        pf=(ytFrame_T*)&t_ucaTx;
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
				pf=(ytFrame_T*)&t_ucaTx;
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
				uart1SendMsg(t_ucaTx,len+2);
        t_usIndex +=CFG_FILE_PACKAGE;
    }
		return 0;
}

/**
 * 写配置
 * @param:  NONE
 * @return: NONE
*/
uint8_t rscmdYtWrCfgInitFile(uint8_t *pBuf,uint32_t ulIndex)
{
    uint16_t t_usCrc;
    uint8_t  *p1;
    uint16_t len;

	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t i;
	  uint8_t t_ucaTx[USART1_TXBUF_LEN];
	  uint8_t t_ucFlg;
    
	  t_ucFlg = 0;
    p=(ytFrame_T*)(&pBuf[ulIndex]);  
    if(p->m_usLen!=CFG_FILE_PACKAGE)return 1;
    if((p->m_tCd.FIN==0) && (p->m_tCd.FIR==1))//首帧
    {
        for(i=0;i<p->m_usLen;i++)
        {
          g_ucaFileBuff[i] = p->m_ucpDat[i];
        }
    }
    else if((p->m_tCd.FIN==1) && (p->m_tCd.FIR==0))
    {
        for(i=0;i<p->m_usLen;i++)
        {
          g_ucaFileBuff[i+p->m_tCd.Count*CFG_FILE_PACKAGE] = p->m_ucpDat[i];
        }  
    }
    else if((p->m_tCd.FIN==1) && (p->m_tCd.FIR==1))
    {
        for(i=0;i<p->m_usLen;i++)
        {
            g_ucaFileBuff[i+p->m_tCd.Count*CFG_FILE_PACKAGE] = p->m_ucpDat[i];
        } 
				t_ucFlg = 1;
    }
    
    pf=(ytFrame_T*)&t_ucaTx;
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
		uart1SendMsg(t_ucaTx,len+2);
		
	  if(t_ucFlg==1)
		{
				t_ucFlg = 0;
				systemSetStructToCfgPage(BACKUP_CFG_FILE_EMBEDFLASH_STARTADDR,g_ucaFileBuff,CFG_BUFF_LEN);
		}
		return 0;
}


/**
 * 读配置问及那
 * @param:  NONE
 * @return: NONE
*/
uint8_t rscmdYtRdCfgInitFile(uint8_t *pBuf,uint32_t ulIndex)
{
    uint16_t t_usCrc;
    uint8_t  *p1;
    uint16_t len;
	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t i;
    uint16_t t_usIndex,j;
	  uint8_t t_ucaTx[USART1_TXBUF_LEN];
    
    t_usIndex = 0;   
    
    for(j=0;j<CFG_FILE_EMBEDFLASH_TOTALPACKAGES;j++)
    {     
        pf=(ytFrame_T*)&t_ucaTx;
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
				pf=(ytFrame_T*)&t_ucaTx;
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
				uart1SendMsg(t_ucaTx,len+2);
        t_usIndex +=CFG_FILE_PACKAGE;
    }
		return 0;
}


/**
 * 写序号
 * @param:  NONE
 * @return: NONE
*/
uint8_t rscmdYtWrSerialNo(uint8_t *pBuf,uint32_t ulIndex)
{
	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t t_usCrc;
    uint8_t *p1;
    uint16_t i;
    uint16_t len;
	  uint8_t t_ucaTx[USART1_TXBUF_LEN];

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
    pf=(ytFrame_T*)&t_ucaTx;
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
		uart1SendMsg(t_ucaTx,len+2);
		return 0;
}

/**
 * 读系统信息
 * @param:  NONE
 * @return: NONE
*/
uint8_t rscmdYtReadVersion(uint8_t *pBuf,uint32_t ulIndex)
{
	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t t_usCrc;
    uint8_t *p1;
    uint16_t i;
    uint16_t len;
	  uint8_t t_ucaTx[USART1_TXBUF_LEN];

    pf=(ytFrame_T*)&t_ucaTx;
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
		uart1SendMsg(t_ucaTx,len+2);
		return 0;
}

/**
 * 复位系统
 * @param:  NONE
 * @return: NONE
*/
uint8_t rscmdYtRstMe(uint8_t *pBuf,uint32_t ulIndex)
{
	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t t_usCrc;
    uint8_t *p1;
    uint16_t i;
    uint16_t len;
	  uint8_t t_ucaTx[USART1_TXBUF_LEN];

	  p=(ytFrame_T*)(&pBuf[ulIndex]);
    if(p->m_ucpDat[0]==0x01)//系统复位
    {

    }
    pf=(ytFrame_T*)&t_ucaTx;
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
    uart1SendMsg(t_ucaTx,len+2);
    
     __set_FAULTMASK(1);//关闭所有中断
     NVIC_SystemReset();
    while(1);
		return 0;
}

/**
 * 零点
 * @param:  NONE
 * @return: NONE
*/
uint8_t cmdYtSetZero(uint8_t *pBuf,uint32_t ulIndex)
{
	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t t_usCrc;
    uint8_t *p1;
    uint16_t i;
    uint16_t len;
	  uint8_t t_ucaTx[USART1_TXBUF_LEN];

    systemSetBuffToFlash(ZERO_FILE_STARTADDR,&g_ucsPressureZero12Bit[0][0],TOTAL_SAMPLE_POINT);
	
    pf=(ytFrame_T*)&t_ucaTx;
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
		uart1SendMsg(t_ucaTx,len+2);
    return 0;
}

/**
 *  ????
 * @param:  NONE
 * @return: NONE
*/ 
uint8_t cmdYtResetZero(uint8_t *pBuf,uint32_t ulIndex)
{
	  ytFrame_T *pf;
    ytFrame_T *p;
    uint16_t t_usCrc;
    uint8_t *p1;
    uint16_t i;
    uint16_t len;
	  uint8_t t_ucaTx[USART1_TXBUF_LEN];

		memset(g_ucsPressureZero12Bit[0],0,sizeof(g_ucsPressureZero12Bit));
		systemSetBuffToFlash(ZERO_FILE_STARTADDR,&g_ucsPressureZero12Bit[0][0],TOTAL_SAMPLE_POINT);
    
    pf=(ytFrame_T*)&t_ucaTx;
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
		uart1SendMsg(t_ucaTx,len+2);

    return 0;
}


//-----------------matlab 通信-------------------//
/**
 * 采样
 * @param:  NONE
 * @return: NONE
*/
uint8_t rsCmdStartSample(uint8_t *pBuf,uint32_t ulIndex)
{
    YotlivePt_T *t_ptylc;  
    uint16_t t_ust_usCrc;
	  uint8_t t_ucaTx[USART1_TXBUF_LEN];

    t_ptylc=(YotlivePt_T*)t_ucaTx;
									 
    t_ptylc->m_ucHeadH = 0x68;
    t_ptylc->m_ucHeadL = 0x68;
    t_ptylc->m_usAddr =  g_ptSystemConfiguration->m_tMatCom.m_usId;
    t_ptylc->m_ucCmd  =  0x83;
    t_ptylc->m_usLen  =  1;
    t_ptylc->m_ucpDat[0] = g_ucrsYtSndFlg;
    t_ust_usCrc =   calcCrc16Modbus(0XFFFF,(uint8_t*)t_ptylc->m_ucpDat,t_ptylc->m_usLen);
    t_ptylc->m_ucpDat[t_ptylc->m_usLen]   = t_ust_usCrc % 256;
    t_ptylc->m_ucpDat[t_ptylc->m_usLen+1] = t_ust_usCrc / 256;  
    
    uart1SendMsg(t_ucaTx,9);
    return 0;
}


/**
 * 读系统
 * @param:  NONE
 * @return: NONE
*/
uint8_t rsCmdGetSystem(uint8_t *pBuf,uint32_t ulIndex)
{
    YotlivePt_T *t_ptylc;  
    uint16_t t_ust_usCrc;
    uint8_t t_ucaTx[USART1_TXBUF_LEN];
    
    t_ptylc=(YotlivePt_T*)t_ucaTx;
									 
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
    uart1SendMsg(t_ucaTx,t_ptylc->m_usLen+9);
    return 0;
}

/**
 * 广播
 * @param:  NONE
 * @return: NONE
*/
uint8_t rsCmdBrocast(uint8_t *pBuf,uint32_t ulIndex)
{
    YotlivePt_T *t_ptylc;  
    uint16_t t_ust_usCrc;
    uint8_t t_ucaTx[USART1_TXBUF_LEN];
    
    t_ptylc=(YotlivePt_T*)t_ucaTx;
									 
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
    uart1SendMsg(t_ucaTx,t_ptylc->m_usLen+9);
    return 0;
}



/**
 * 发送数据
 * @param:  NONE
 * @return: NONE
*/
uint16_t g_usrsFrameCount;


uint8_t t_ucaSampleBuff[ROW * COLUMN * 2];
uint8_t rsCmdSndDat(void)
{
	
    YotlivePt_T *t_ptylc;  
    uint16_t t_ust_usCrc;
    
	  uint16_t t_usLen,t_usTempLen,t_usDiv,t_usMod,t_usRawLen,t_usTemp,t_usTempH,t_usTempL;
    uint16_t *ucp,i,j,t_usCrc;
	  

    switch(g_ptSystemConfiguration->m_tMatSystemCfg.m_ucaDataFrame)
    {
    case 1://8bit
	  t_ptylc=(YotlivePt_T*)g_tUart1Phy.m_ucaTx;
    t_ptylc->m_ucHeadH = 0x68;
    t_ptylc->m_ucHeadL = 0x68;
    t_ptylc->m_usAddr =  g_ptSystemConfiguration->m_tMatCom.m_usId;
    t_ptylc->m_ucCmd = 0X84;
    t_ptylc->m_usLen = sizeof(g_ucaPressureRaw)+3;
    t_ptylc->m_ucpDat[0]=g_usrsFrameCount%256;
    t_ptylc->m_ucpDat[1]=g_usrsFrameCount/256;
    t_ptylc->m_ucpDat[2]=1;
    
    memcpy(&t_ptylc->m_ucpDat[3],g_ucaPressureRaw,sizeof(g_ucaPressureRaw));
    t_ust_usCrc = calcCrc16Modbus(0XFFFF,(uint8_t*)t_ptylc->m_ucpDat,t_ptylc->m_usLen);
    t_ptylc->m_ucpDat[t_ptylc->m_usLen] = t_ust_usCrc % 256;
    t_ptylc->m_ucpDat[t_ptylc->m_usLen+1] = t_ust_usCrc / 256;
    g_usrsFrameCount++;
		uart1SendMsg(g_tUart1Phy.m_ucaTx,t_ptylc->m_usLen+9);
      break;
    case 2://12bit        
    t_ptylc=(YotlivePt_T*)g_tUart1Phy.m_ucaTx;
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
     t_ptylc->m_ucpDat[0]=g_usrsFrameCount%256;
     t_ptylc->m_ucpDat[1]=g_usrsFrameCount/256;
             
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
     g_usrsFrameCount++;
		 uart1SendMsg(g_tUart1Phy.m_ucaTx,t_ptylc->m_usLen+9);
      break;      
    }
    return 0;
}



uint8_t getDataFromUart1(void)
{
	  
		uint8_t t_ucaDeal[USART1_RXBUF_LEN];
	  ytFrame_T *t_ptYt;
	  uint32_t i,t_ulIndex,t_ulSts,t_ulDt;
	  uint8_t t_ucFlg,t_ucFinishFlg;
	  uint8_t *ptData;
	  uint16_t t_usCrc;
	  uint16_t t_usAcc;
	  YotlivePt_T *t_ptYlc;

		if(USART_GetITStatus(USART1, USART_INT_ORERR))
		{
				t_ulSts = USART1->STS;
				t_ulDt  = USART1->DT;
		}

	  if(g_tUart1Phy.m_ucReceived)
    {
		    WDTCLR();
			  g_tUart1Phy.m_ucReceived = 0;
			  if(g_tUart1Phy.m_usRxLen> 144) return 0;
			  memcpy(t_ucaDeal,g_tUart1Phy.m_ucaRx,g_tUart1Phy.m_usRxLen);
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
														 g_ucYtSndFlg = 1;
														 usbCmdStartSample(t_ucaDeal,t_ulIndex); 
													}
													else
													{
														 g_ucYtSndFlg = 0;
														 usbCmdStartSample(t_ucaDeal,t_ulIndex); 
													}
													t_ucFinishFlg = 1;
											break;
											case 0x04://读取传感器数据
												  g_ucYtSndFlg = 0;
													usbCmdSndDat();
													t_ucFinishFlg = 1;
											break;
											case 0x7e://系统信息
													g_ucYtSndFlg = 0;
													usbCmdGetSystem(t_ucaDeal,t_ulIndex); 
											    t_ucFinishFlg = 1;
											break;
											case 0x7F:
													g_ucYtSndFlg = 0;
													usbCmdBrocast(t_ucaDeal,t_ulIndex);
													t_ucFinishFlg = 1;
											break;  
										}
								}
								if(t_ptYlc->m_usAddr==0xffff)
								{
											switch(t_ptYlc->m_ucCmd)
											{
													case 0x7F:
													g_ucYtSndFlg = 0;	
													usbCmdBrocast(t_ucaDeal,t_ulIndex); 
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
													cmdYtBro(t_ucaDeal,t_ulIndex);
												  t_ucFinishFlg = 1;
												break;
												case WIN_2017D_CFG:             //更新配置文件
													cmdYtWrCfgFile(t_ucaDeal,t_ulIndex);
												  t_ucFinishFlg = 1;
												break;  
												case WIN_2017D_READCFG:         //读取配置文件
													cmdYtRdCfgFile(t_ucaDeal,t_ulIndex);
												  t_ucFinishFlg = 1;
												break; 
												case WIN_2017D_CFG_INIT:        //更新配置文件
													cmdYtWrCfgInitFile(t_ucaDeal, t_ulIndex);
												  t_ucFinishFlg = 1;
												break;  
												case WIN_2017D_READCFG_INIT:    //读取配置文件
													cmdYtRdCfgInitFile(t_ucaDeal, t_ulIndex);
												  t_ucFinishFlg = 1;
												break;
												case WIN_2017D_WRSERNO:        //写序列号
													cmdYtWrSerialNo(t_ucaDeal, t_ulIndex);
												  t_ucFinishFlg = 1;
												break;
												case WIN_2017D_VERSION:        //读取版本号
													cmdYtReadVersion(t_ucaDeal, t_ulIndex);
												  t_ucFinishFlg = 1;
												break;  
												case WIN_2017D_RST:            //复位系统
													cmdYtRstMe(t_ucaDeal, t_ulIndex);
												  t_ucFinishFlg = 1;
												break; 
												case WIN_2017D_RATEING:            //系统调零
													cmdYtSetZero(t_ucaDeal, t_ulIndex);
												  t_ucFinishFlg = 1;
												break; 
												case WIN_2017D_RSTRATEING:            //恢复调零
													cmdYtResetZero(t_ucaDeal, t_ulIndex);
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
												case CMD_SET_CALIBRATED_MODE4: //模式4标定值
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



