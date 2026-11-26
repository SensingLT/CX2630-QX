#include "systemInit.h"
#include "sampleTask.h"
#include "crc.h"
#include "debugCli.h"
#include <string.h>
#include "flash.h"
#include "uart.h"
#include "IoPort.h"
#include "time.h"
#include "exAdc.h"
#include "debugCli.h"
#include "spi.h"
#include "time.h"

//----------系统信息-----------------------------------------------//
uint8_t const c_ucaHardware[2]={0x03,0X00};            //硬件版本
uint8_t const c_ucaFirmware[2]={0x01,0x01};            //软件版本
uint8_t const c_ucaDeviceMsg[4]  ={ROW,COLUMN,0x00,0x00}; //设备ID
uint8_t const c_ucaPassword[8]={0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55};
uint8_t const c_ucaWireProtocolVersion[4]    ={0x00,0x00,0x00,0x01};
uint8_t const c_ucaWirelessProtocolVersion[4]={0x00,0x00,0x00,0x01};

cfgSystemList_T  g_tCfgSystemList;
cfgSystemList_T *g_ptCfgSystemList;
systemConfiguration_T* g_ptSystemConfiguration; 
uint8_t g_ucaCfgBuff[CFG_BUFF_LEN];
extern uint8_t  g_ucaFileBuff[CFG_BUFF_LEN];
uint8_t g_ucaSerialNumber[SERNO_LEN];
extern uint32_t g_ulDurationTime;
uint8_t g_ucDurationTimeIndex;
uint8_t g_ucWriteDurationTimeFlg;
uint8_t g_ucDurationTimeBuff[CFG_FILE_DURATIONA_LEN];
uint32_t  g_ulDurationTime;//耐久时间
uint16_t  g_usDurationIndex;
 
void systemReadDurationTime(void);

systemTick_T g_tSystemTick;

const systemConfiguration_T  c_tSystemConfiguration =
{
  .ulStart=START_WORD,
  .ulLen  = sizeof(matCom_T) + sizeof(matSystemCfg_T) 
          + sizeof(matCalibrateMode1_T)+ sizeof(matDurationTime_T)+ sizeof(matBackup_T),
  .m_tMatCom=
  {
    .m_usId   = 1,
    .m_ulBaud  = 115200,
		.m_ucaBack ={0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF},
  },
  .m_tMatSystemCfg=
	{
		.m_ucDebugLevel = 4,
		.m_ucQsmEnable  = 0,
		.m_ucFlatThreshold =5,
    .m_ucSideThreshold=5,
    .m_ucaDataFrame  = 1,
    .m_usSamplePoint = 20,
    .m_ucRow         = 1,
    .m_ucColumn      = 20,
    .m_ucCaliMode    = 0,
    .m_usLocalId     =1,
    .m_ulTick        = 100,
		.m_fTotalSensorArea = 3.6,
    .m_fSensorSizeX = 162.5,
    .m_fSensorSizeY = 65.0,
    .m_usSensorPoint= 20,
    .m_fPressureRangeLow = 0,
    .m_fPressureRangeHigh = 100.0,
    .m_usTheshold = {200,500,1000,2000,3500,5000,6500,8000,9500,9500},
    .m_fBackUp=3.6,
		.m_ucZero = 5,
		.m_fScale = 1.0,
		.m_ucRtcSpeed = 6,
		.m_usAbsValue = 6,
		.m_usOverPressure = 60,
		.m_usAbs = 6,
		.m_ucaBack =
    {
			0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,
			0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,
		  0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,
			0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,
		  0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,
			0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0xFF,0XFF,0XFF,0XFF,0XFF,0XFF,
			0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0XFF,0XFF,
			0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF
		},
	},
  .m_tMatCalibrateMode1=
  {
    .m_ucSeg1 = 112,
    .m_fCailA1 = 0.2285,
    .m_fCailB1 = 0,
    .m_ucSeg2 = 133,
    .m_fCailA2 = 0.2285,
    .m_fCailB2 = 0,
    .m_ucSeg3 = 143,
    .m_fCailA3 = 0.2285,
    .m_fCailB3 = 0,
    .m_ucSeg4 = 150,
    .m_fCailA4 = 0.2285,
    .m_fCailB4 = 0,
  },
	.m_tMatDuration=
	{
		.m_ulDuration={1000,3000,5000,6000,8000,10000,16000,30000,40000,50000},
		.m_usErrpoint=800,
		.m_usAbsValue = 5,
		.m_ulThershold= 5,
		.m_ulInterval = 1,
	},	
  .m_tMatBackup=
  { 
      0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,
      0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,   
      0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,
      0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,   
      0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,
      0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,   
      0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,
      0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF, 
      0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,0xFF,0XFF,0XFF,0XFF,       
  },
};

/**
  * @brief  从片外Flash中读取数据
  * @param  None
  * @retval None
  */
uint8_t systemGetRecordFromEmbedFlash(uint32_t ulAddr,uint8_t *ucaBuff,uint16_t usLen)
{
    uint32_t i; 
    
    for(i=0;i<usLen;i++)
    {
        ucaBuff[i]  = *(uint8_t *) (ulAddr+i);
    }
    return 0;
}

/**
  * @brief  将数据写到配置文件存储区
  * @param  None
  * @retval None
  */
uint8_t systemSetBuffToCfgPage(uint32_t ulAddr,uint8_t *ucaBuff,uint16_t usLen)
{      
	  
	  if(usLen % 2 != 0)
		{
				usLen= usLen+1;
		}
		FLASH_Write(ulAddr,(uint16_t *)ucaBuff,usLen/2);
    return 0;
}

uint8_t systemSetBuffToFlash(uint32_t ulAddr,uint16_t *ucsBuff,uint16_t usLen)
{      
	  
	  if(usLen % 2 != 0)
		{
				usLen= usLen+1;
		}
		FLASH_Write(ulAddr,(uint16_t *)ucsBuff,usLen);
    return 0;
}


/**
  * @brief  将数据写到配置文件存储区
  * @param  None
  * @retval None
  */
uint8_t systemSetStructToCfgPage(uint32_t ulAddr,uint8_t *ucaBuff,uint16_t usLen)
{
    uint32_t t_ulCrc;  
    uint32_t t_ulLen;	
    
	
	  t_ulLen = sizeof(matCom_T) + 
                     sizeof(matSystemCfg_T) + 
                     sizeof(matCalibrateMode1_T)+ 
										 sizeof(matDurationTime_T) + 
                     sizeof(matBackup_T);
														
    t_ulCrc = calcCrc16Modbus(0xFFFF,ucaBuff+8,
                            sizeof(matCom_T) + 
                            sizeof(matSystemCfg_T) + 
                            sizeof(matCalibrateMode1_T)+ 
										        sizeof(matDurationTime_T) + 
                            sizeof(matBackup_T));
														
    ucaBuff[8 + t_ulLen]    = t_ulCrc % 256;
    ucaBuff[8 + t_ulLen+1]  = t_ulCrc / 256;
    systemSetBuffToCfgPage(ulAddr,ucaBuff,sizeof(systemConfiguration_T));      
    return 0;
}

/**
  * @brief  加载配置文件
  * @param  None
  * @retval None
  */
uint8_t systemLoadCfgFile(void)
{
    uint16_t t_usCrc;
	
    g_tCfgSystemList.m_usBufLen      = CFG_BUFF_LEN;
    g_tCfgSystemList.ucpDataBuffer   = g_ucaCfgBuff;
    g_tCfgSystemList.m_ulCfgAddr     = CFG_FILE_EMBEDFLASH_STARTADDR;
    systemGetRecordFromEmbedFlash(CFG_FILE_EMBEDFLASH_STARTADDR,g_tCfgSystemList.ucpDataBuffer,g_tCfgSystemList.m_usBufLen);
    g_ptSystemConfiguration = (systemConfiguration_T *)g_ucaCfgBuff;
  
    if(g_ptSystemConfiguration->ulStart == START_WORD)
    {
        t_usCrc = calcCrc16Modbus(0xffff,g_ucaCfgBuff+8,sizeof(matCom_T) + 
                                              sizeof(matSystemCfg_T) + 
                                              sizeof(matCalibrateMode1_T)+ 
										                          sizeof(matDurationTime_T) + 																								
                                              sizeof(matBackup_T));
        if((t_usCrc / 256 == g_ptSystemConfiguration->m_usCrc /256)
        && (t_usCrc % 256 == g_ptSystemConfiguration->m_usCrc %256))
        {
            g_tCfgSystemList.m_ulInitSucessed = SET;
            debugCli(LOG_OTHER,"Config All right!");
        }
        else
        {
            g_tCfgSystemList.m_ulInitSucessed = RESET;
            debugCli(LOG_OTHER,"Config File CRC error!");
        }
    }
    
    if(g_tCfgSystemList.m_ulInitSucessed == RESET)/*校验错误*/
    {
        g_tCfgSystemList.m_usBufLen      = CFG_BUFF_LEN;
        g_tCfgSystemList.ucpDataBuffer   = g_ucaCfgBuff;
        g_tCfgSystemList.m_ulCfgAddr     = CFG_FILE_EMBEDFLASH_STARTADDR;
        systemGetRecordFromEmbedFlash(BACKUP_CFG_FILE_EMBEDFLASH_STARTADDR,g_tCfgSystemList.ucpDataBuffer,g_tCfgSystemList.m_usBufLen);//从备份区获取数据
        g_ptSystemConfiguration = (systemConfiguration_T *)g_ucaCfgBuff;
        if(g_ptSystemConfiguration->ulStart == START_WORD)
        {
            t_usCrc = calcCrc16Modbus(0xffff,g_ucaCfgBuff+8,
                                              sizeof(matCom_T) + 
                                              sizeof(matSystemCfg_T) + 
                                              sizeof(matCalibrateMode1_T)+ 
																						  sizeof(matDurationTime_T) + 
                                              sizeof(matBackup_T));
            if((t_usCrc / 256 == g_ptSystemConfiguration->m_usCrc /256)
            && (t_usCrc % 256 == g_ptSystemConfiguration->m_usCrc %256))
            {
                systemSetBuffToCfgPage(CFG_FILE_EMBEDFLASH_STARTADDR,g_tCfgSystemList.ucpDataBuffer,g_tCfgSystemList.m_usBufLen); /*将备份区的数据写到操作区*/
                debugCli(LOG_OTHER,"Write file form embed flash to fram!");
            }
            else
            {
                g_tCfgSystemList.pctCfg = &c_tSystemConfiguration; 
                memcpy(g_ucaCfgBuff,g_tCfgSystemList.pctCfg,sizeof(systemConfiguration_T));
                systemSetBuffToCfgPage(CFG_FILE_EMBEDFLASH_STARTADDR,g_tCfgSystemList.ucpDataBuffer,g_tCfgSystemList.m_usBufLen);  /*将片内默认Flash写到操作区*/
                debugCli(LOG_OTHER,"recover factory config!");
            }
        }
        else
        {
            g_tCfgSystemList.m_usBufLen      = CFG_BUFF_LEN;
            g_tCfgSystemList.ucpDataBuffer   = g_ucaCfgBuff;
            g_tCfgSystemList.m_ulCfgAddr     = CFG_FILE_EMBEDFLASH_STARTADDR;
            g_tCfgSystemList.pctCfg = &c_tSystemConfiguration; 
            memcpy(g_ucaCfgBuff,g_tCfgSystemList.pctCfg,sizeof(systemConfiguration_T));
            systemSetStructToCfgPage(CFG_FILE_EMBEDFLASH_STARTADDR,g_tCfgSystemList.ucpDataBuffer,g_tCfgSystemList.m_usBufLen);  
            systemGetRecordFromEmbedFlash(CFG_FILE_EMBEDFLASH_STARTADDR,g_tCfgSystemList.ucpDataBuffer,g_tCfgSystemList.m_usBufLen); 
            g_ptSystemConfiguration = (systemConfiguration_T *)g_ucaCfgBuff;
            debugCli(LOG_OTHER,"recover factory config!");   
        }
    }
    return 0;
}

/**
  * @brief  系统延时
  * @param  None
  * @retval None
  */
void systemDelay(uint32_t ulMs)
{
		uint32_t i,j;
	
	  for(i=0;i<ulMs;i++)
		{
			for(j=0;j<10000;j++);
		}
}


/**
  * @brief  系统延时
  * @param  None
  * @retval None
  */
void systemIwdg(void)
{
	  if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
		{
			RCC_ClearFlag();
		}
    IWDG_SetReload(LSIFREQ-1);
    IWDG_KeyRegWrite(IWDG_KeyRegWrite_Enable);
    IWDG_SetPrescaler(IWDG_Psc_16);
    IWDG_KeyRegWrite(IWDG_KeyRegWrite_Disable);
    IWDG_ReloadCounter();
    IWDG_Enable();
}

/**
  * @brief  设备初始化
  * @param  None
  * @retval None
  */
uint8_t systemParmInit(void)
{
	  uint8_t i;
	 // systemIwdg();
	  WDTCLR();
	  GpioInit(); 
	  Delay_init();         /*延时函数初始化*/
	  WDTCLR();
	  Delay_ms(500);
	  WDTCLR();
	  Delay_ms(500);
	  WDTCLR();
		systemLoadCfgFile();  /*加载配置文件*/
	  systemReadDurationTime();
	  WDTCLR();
		time3Init();
    NVIC_Time3Configuration();
	  debugPortInit(115200);/*调试串口初始化*/

	  debugLevel();
		rsPortInit(115200);   /*RS485串口函数初始化*/
		sampleInitAdcPort();  /*端口初始化*/
		exAdcInit();          
	  initRtc();
	  for(i=0;i<16;i++)
	  {
			g_ucaSerialNumber[i]=*(char*)(SERNO_START_ADDR+i);
		}
		WDTCLR();
	  return 0;
}


/**
  ******************************************************************************
  * 函数名：  systemSetDurayionTimetToExMem
  * 功能：    写
  * 输入：    ucaBuff  usLen
  * 输出：    返回0
              返回1 失败 
  */
uint8_t systemSetDurayionTimetToExMem(uint32_t ulAddr,uint8_t *ucaBuff,uint16_t usLen)
{
	  if(usLen % 2 != 0)
		{
				usLen= usLen+1;
		}
		FLASH_Write_BYTES(ulAddr,(uint16_t *)ucaBuff,usLen/2);
    return 0;
}

/**
  ******************************************************************************
  * 函数名：  systemGetDurayionTimetToExMem
  * 功能：    写
  * 输入：    ucaBuff  usLen
  * 输出：    返回0
              返回1 失败 
  */
uint8_t systemGetDurayionTimetToExMem(uint32_t ulAddr ,uint8_t *ucaBuff,uint16_t usLen)
{
	  uint8_t t_ucStatus;
	  uint16_t i;
    
    for(i=0;i<usLen;i++)
    {
        ucaBuff[i]  = *(uint8_t *) (ulAddr+i);
    }
    return 0;
}


/**
  ******************************************************************************
  * 函数名：  systemMaxIndex
  * 功能：    查找最大的时间
  * 输入：    
  * 输出：   
  */
uint8_t systemMaxIndex(uint32_t *ulBuff,uint8_t ucNum)
{
	  uint8_t t_ucIndex,i;
	  uint32_t t_ulMax; 
	
	  t_ulMax = ulBuff[0];
	  t_ucIndex = 0;
	  for(i=1;i<CFG_FILE_DURATIONAREADEEPTH-1;i++)
		{
			 if(t_ulMax<ulBuff[i])
			 {
					t_ucIndex = i;
				  t_ulMax = ulBuff[i];
			 }
		}
		return t_ucIndex;
}

/**
  ******************************************************************************
  * 函数名：  systemReadDurationTime
  * 功能：    读衰老曲线时间
  * 输入：    
  * 输出：   
  */
void systemReadDurationTime(void)
{
	  uint8_t i,t_ucFlg,t_ucIndex;
	  uint16_t j;
	  uint8_t t_ucaTempBuff[CFG_FILE_DURATION_PAGE];//32个缓存区的BUFF 4字节的数据+2字节crc校验
	  uint32_t t_ulDurationTimeTemp;
	  uint16_t t_usCrc;

		t_ucFlg = 0;
		for(i=0;i<CFG_FILE_DURATIONAREADEEPTH;i++)
		{
				systemGetDurayionTimetToExMem(CFG_FILE_DURATIONAREA1+i*CFG_FILE_DURATION_PAGE,t_ucaTempBuff,CFG_FILE_DURATION_PAGE);
				t_usCrc = calcCrc16Modbus(0xffff,t_ucaTempBuff,4);
				if((t_ucaTempBuff[4] == t_usCrc % 256) && (t_ucaTempBuff[5] == t_usCrc / 256))
				{
						memcpy(&t_ulDurationTimeTemp, &t_ucaTempBuff, 4);
					  g_ulDurationTime = t_ulDurationTimeTemp;
					  g_ucDurationTimeIndex = i+1;
					  t_ucFlg = 1;
				} 
				else//检查是否不是FF的CRC监测错误
				{
						break;
				}
		}
		if(t_ucFlg==0)
		{
				FLASH_Unlock();
		    FLASH_ErasePage(CFG_FILE_DURATIONAREA1);
        FLASH_Lock();
			  g_ucDurationTimeIndex = 0;
				memset(t_ucaTempBuff,0,4);
				t_ulDurationTimeTemp=0;
				t_usCrc = calcCrc16Modbus(0xffff,t_ucaTempBuff,4);
				t_ucaTempBuff[4] = t_usCrc % 256;
				t_ucaTempBuff[5] = t_usCrc / 256;
				systemSetDurayionTimetToExMem(CFG_FILE_DURATIONAREA1,t_ucaTempBuff,CFG_FILE_DURATION_PAGE);	
			  g_ulDurationTime = t_ulDurationTimeTemp;		
		}
}

/**
  ******************************************************************************
  * 函数名：  systemClrDurationTime
  * 功能：    清除衰老曲线时间
  * 输入：    NONE
  * 输出：    NONE
  */
void systemClrDurationTime(void)
{
		g_ulDurationTime=0;
	  g_ucDurationTimeIndex = 0;//索引
	  FLASH_Unlock();
		FLASH_ErasePage(CFG_FILE_DURATIONAREA1);
	  FLASH_Lock();
}

/**
  ******************************************************************************
  * 函数名：  systemWriteDurationTime
  * 功能：    写衰老曲线时间
  * 输入：    NONE
  * 输出：    NONE
  */
void systemWriteDurationTime(void)
{
	  uint8_t t_ucaTempBuff[CFG_FILE_DURATION_PAGE];//32个缓存区的BUFF 4字节的数据+2字节crc校验
	  uint16_t t_usCrc;
	
	  if(g_ucWriteDurationTimeFlg==1)
		{
			  g_ucWriteDurationTimeFlg = 0;
			  if(g_ucDurationTimeIndex >= CFG_FILE_DURATIONAREADEEPTH)//异常
				{
						g_ucDurationTimeIndex = 0;
					  FLASH_Unlock();
		        FLASH_ErasePage(CFG_FILE_DURATIONAREA1);
            FLASH_Lock();
				}
			  memcpy(&t_ucaTempBuff,&g_ulDurationTime,4);
				t_usCrc = calcCrc16Modbus(0xffff,t_ucaTempBuff,4);
				t_ucaTempBuff[4] = t_usCrc % 256;
				t_ucaTempBuff[5] = t_usCrc / 256;
				systemSetDurayionTimetToExMem(CFG_FILE_DURATIONAREA1+g_ucDurationTimeIndex*CFG_FILE_DURATION_PAGE,t_ucaTempBuff,CFG_FILE_DURATION_PAGE);
	      
				g_ucDurationTimeIndex++;
			  if(g_ucDurationTimeIndex >= CFG_FILE_DURATIONAREADEEPTH)
				{
						g_ucDurationTimeIndex = 0;
					  FLASH_Unlock();
		        FLASH_ErasePage(CFG_FILE_DURATIONAREA1);
            FLASH_Lock();
						memcpy(&t_ucaTempBuff,&g_ulDurationTime,4);
						t_usCrc = calcCrc16Modbus(0xffff,t_ucaTempBuff,4);
						t_ucaTempBuff[4] = t_usCrc % 256;
						t_ucaTempBuff[5] = t_usCrc / 256;
						systemSetDurayionTimetToExMem(CFG_FILE_DURATIONAREA1+g_ucDurationTimeIndex*CFG_FILE_DURATION_PAGE,t_ucaTempBuff,CFG_FILE_DURATION_PAGE);
				    g_ucDurationTimeIndex++;
				}
		}
}


/**
  ******************************************************************************
  * 函数名：  systemReadDurationTime
  * 功能：    读衰老曲线时间
  * 输入：    
  * 输出：   
  */
void systemReadDurationTimeArray(void)
{
	  uint8_t i;
	  uint8_t t_ucaTempBuff[CFG_FILE_DURATION_PAGE];//32个缓存区的BUFF 4字节的数据+2字节crc校验

	  for(i=0;i<CFG_FILE_DURATIONAREADEEPTH;i++)
		{
				systemGetDurayionTimetToExMem(CFG_FILE_DURATIONAREA1+i*CFG_FILE_DURATION_PAGE,t_ucaTempBuff,CFG_FILE_DURATION_PAGE);
        memcpy(&g_ucDurationTimeBuff[i*6],t_ucaTempBuff,6); 
	  }
}

uint32_t g_ulSavRecord;
uint32_t g_ulSystemTick;
void systemSetRecord(void)
{
	  g_ulSystemTick++;
	  if(g_ulSystemTick < 6000) return;
	  //if(g_ulSystemTick < 600) return;
	  g_ulSystemTick=0;
	  if(g_ulDurationTime >=MAX_DURATIONTIME )
		{
				g_ulDurationTime = MAX_DURATIONTIME;
		}
		else
		{
			  g_ulDurationTime++;
			  g_ulSavRecord++;
			  if(g_ulSavRecord > g_ptSystemConfiguration->m_tMatDuration.m_ulInterval)
				{
						g_ulSavRecord = 0;
						g_ucWriteDurationTimeFlg = 1;
				}
		}

}



