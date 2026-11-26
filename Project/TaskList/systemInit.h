/*
 * File:		systemInit.h
 * Purpose:     Provide commond line interface routines
 * Notes:
 */
#ifndef __SYSTEMINIT_H__
#define __SYSTEMINIT_H__
#include <at32f4xx.h>


//片外存储区初始值

#undef DEBUG

#define CFG_FILE_DURATIONAREA1      0x0800CC00UL //0x0800CC00--0x0800CC00 32 个点
#define CFG_FILE_DURATIONAREA2      0x0800CC20UL //0x0800CC20--0x0800CC3F 32 个点
#define CFG_FILE_DURATIONAREA3      0x0800CC40UL //0x0800CC40--0x0800CC5F 32 个点
#define CFG_FILE_DURATIONAREA4      0x0800CC60UL //0x0800CC60--0x0800CC7F 32 个点
#define CFG_FILE_DURATIONAREA5      0x0800CC80UL //0x0800CC80--0x0800CC9F 32 个点
#define CFG_FILE_DURATIONAREA6      0x0800CCA0UL //0x0800CCA0--0x0800CCBF 32 个点
#define CFG_FILE_DURATIONAREA7      0x0800CCC0UL //0x0800CCC0--0x0800CCDF 32 个点
#define CFG_FILE_DURATIONAREA8      0x0800CCE0UL //0x0800CCE0--0x0800CCFF 32 个点
#define CFG_FILE_DURATIONA_LEN      192

//标定数据区------------8192字节(0x0800D000--0x0800EFFF)
//模式4与模式7共用同一块起始地址：同一时间只使用一种标定模式，切换模式后需重新下载对应标定文件
//模式4：每点40字节(4段门限+K/B)，128点=5120字节，占5页(1页=1024字节)
#define  CALMODE4_POINT_LEN                    40                /*每点标定参数长度*/
#define  CALMODE4_FILE_LEN                     5120              /*模式4标定数据区长度*/
#define  CALMODE4_FILE_PAGES                   5                 /*模式4占用FLASH页数*/
#define  CALMODE4_ADDR_START                   0x0800D000        /*模式4*/
#define  CALMODE4_ADDR_END                     0x0800E3FF        /**/

//模式7：19包*128字节=2432字节，占3页(3072字节)，与模式4共用起始地址
#define  CALMODE7_FILE_LEN                     64
#define  CALMODE7_FILE_PAGES                   3                 /*模式7占用FLASH页数*/
#define  CALMODE7_ADDR_START                   0x0800D000        /*模式7*/
#define  CALMODE7_ADDR_END                     0x0800DFFF        /**/

//序列号------------1024字节
#define  SERNO_START_ADDR                      0x0800F000        /*配置文件存储起始地址*/
#define  SERNO_END_ADDR                        0x0800F3FF        /*配置文件存储结束地址*/
#define  SERNO_LEN   16

//存放零点数据------1024字节
#define  ZERO_FILE_LEN                         240
#define  ZERO_FILE_STARTADDR                   0x0800F400        /*零点起始地址*/
#define  ZERO_FILE_ENDADDR                     0x0800F7FF        /*零点结束地址*/

//片外存储配置文件--1024字节
#define  CFG_FILE_EMBEDFLASH_STARTADDR         0x0800F800        /*配置文件存储起始地址*/
#define  CFG_FILE_EMBEDFLASH_ENDADDR           0x0800FBFF        /*配置文件存储结束地址*/

//片外初始备份区 1024字节
#define  BACKUP_CFG_FILE_EMBEDFLASH_STARTADDR  0x0800FC00        /*配置文件存储起始地址*/
#define  BACKUP_CFG_FILE_EMBEDFLASH_ENDADDR    0x0800FFFF        /*配置文件存储结束地址*/

#define  MAX_DURATIONTIME               6000000UL

#define 	START_WORD    0xeb90eb90
#define 	END_WORD      0x90eb90eb

#define   CFG_BUFF_LEN  1024

#define   LSIFREQ       40000
#define   WDTCLR()       // IWDG_ReloadCounter()


#define CFG_FILE_DURATIONAREADEEPTH 32         //32级缓存
#define CFG_FILE_BUFF_LEN                1024
#define CFG_FILE_DURATION_PAGE      32

#pragma pack(1)
   

/*Mat串口配置参数*/     
struct tagMatCom_T//22
{
    uint16_t m_usId;        /*通信地址*/
    uint32_t m_ulBaud;      /*通信波特率*/
	  uint8_t  m_ucaBack[16];
};
typedef  struct tagMatCom_T matCom_T; 

/*Mat袋子信息*/ 
struct tagMatSystemCfg_T
{
    uint8_t  m_ucDebugLevel;      /*调试等级*/ 
    uint8_t  m_ucQsmEnable;       /*0=禁止发送数据，1=使能数据传输*/ 
    uint8_t  m_ucFlatThreshold;   /*平躺门限*/ 
    uint8_t  m_ucSideThreshold;   /*侧躺门限*/ 
    uint8_t  m_ucaDataFrame;      /*传输帧格式1=8bits，2=12bits*/ 
    uint16_t m_usSamplePoint;     /*采样点数*/ 
    uint8_t  m_ucRow;             /*扫描逻辑行*/
    uint8_t  m_ucColumn;          /*扫描逻辑列*/
    uint8_t  m_ucCaliMode;        /*校准模式*/    
    uint16_t m_usLocalId;
    uint32_t m_ulTick;            //16
	
    float    m_fTotalSensorArea;       /*传感器面积*/
    float    m_fSensorSizeX;           /*传感器长*/
    float    m_fSensorSizeY;           /*传感器宽*/
    uint16_t m_usSensorPoint;          /*传感器点数*/
    float    m_fPressureRangeLow;      /*传感器校准范围低端*/
    float    m_fPressureRangeHigh;     /*传感器校准范围高端*///22
    uint16_t m_usTheshold[10];         /*校准模式三*/  //20
    float    m_fBackUp;                //46
		uint8_t  m_ucZero;                 /*零点*///47
		float    m_fScale;
		uint8_t  m_ucRtcSpeed;
		uint16_t m_usAbsValue;
		uint16_t m_usOverPressure;
		uint16_t m_usAbs;
    uint8_t  m_ucaBack[128];	         //190           
};
typedef  struct tagMatSystemCfg_T matSystemCfg_T;    
    

/*MAT分段校准信息*/   
struct tagMatCalibrateMode1_T  //36
{
    uint8_t  m_ucSeg1;
    float    m_fCailA1;
    float    m_fCailB1;
    uint8_t  m_ucSeg2;
    float    m_fCailA2;
    float    m_fCailB2;
    uint8_t  m_ucSeg3;
    float    m_fCailA3;
    float    m_fCailB3;
    uint8_t  m_ucSeg4;
    float    m_fCailA4;
    float    m_fCailB4;   
};
typedef  struct tagMatCalibrateMode1_T matCalibrateMode1_T;
   
/* MAT Duration parm*/
struct tagMatDurationTime_T
{   
    uint32_t m_ulDuration[10];
	  uint16_t m_usErrpoint;
	  uint16_t m_usAbsValue;
	  uint32_t m_ulThershold;
	  uint32_t m_ulInterval;
};
typedef  struct tagMatDurationTime_T matDurationTime_T;

/* MAT备份*/
struct tagMatBackup_T//140
{   
    uint8_t m_ucaBackup[140];
};
typedef  struct tagMatBackup_T matBackup_T;

//配置参数列表
struct tagSystemConfiguration_T
{       
    uint32_t       ulStart;
    uint32_t       ulLen;
	  matCom_T       m_tMatCom;
    matSystemCfg_T m_tMatSystemCfg; 
    matCalibrateMode1_T m_tMatCalibrateMode1; 
		matDurationTime_T   m_tMatDuration;            /*垫子衰减*/	
    matBackup_T    m_tMatBackup;
    uint16_t       m_usCrc;
};
typedef  struct tagSystemConfiguration_T systemConfiguration_T;


/*配置文件列表*/
struct tagCfgSystemList_T
{
  uint32_t                        m_ulInitSucessed;  //初始化变量成功 
  const systemConfiguration_T    *pctCfg;            //常量指针
  systemConfiguration_T          *ptCfg;             //结构指针
  uint32_t                        m_ulCfgAddr;       //起始地址
  uint8_t                         *ucpDataBuffer;    //缓存指针
  uint16_t                        m_usBufLen;        //缓存长度
};
typedef struct tagCfgSystemList_T cfgSystemList_T;

/*System Tick*/     
struct tagSystemTick_T//22
{
    uint32_t m_ulTick;
	  uint32_t m_ulSavTick;
    uint32_t m_ulTick10MS; 	
	  uint32_t m_ulTick1S; 
	  uint32_t m_ulTickSp;
};
typedef  struct tagSystemTick_T systemTick_T; 

struct tagWIFICfgStruct_T 
{
	  uint8_t  m_ucaSsid[34];
	  uint8_t  m_ucaPassword[18];
		float    m_fLongitude;
	  float    m_fLatitude;	
	  uint8_t  m_ucaBackup[2];
};
typedef struct tagWIFICfgStruct_T wifiCfgStruct_T;


#pragma pack()

extern cfgSystemList_T  g_tCfgSystemList;
extern cfgSystemList_T *g_ptCfgSystemList;
extern systemConfiguration_T* g_ptSystemConfiguration;

void  systemAppInit(void);

extern uint8_t const c_ucaHardware[2];  //硬件版本号
extern uint8_t const c_ucaFirmware[2];  //固件版本号
extern uint8_t const c_ucaDeviceMsg[4]; //设备ID
extern uint8_t const c_ucaPassword[8];
extern uint8_t const c_ucaWireProtocolVersion[4];
extern uint8_t const c_ucaWirelessProtocolVersion[4];

extern uint8_t  g_ucaSerialNumber[SERNO_LEN];
extern uint8_t  g_ucaCfgBuff[CFG_BUFF_LEN];/*存放配置文件*/

extern uint32_t g_ulSystemTick;

uint8_t systemParmInit(void);
uint8_t systemSetStructToExFram(uint8_t *ucaBuff,uint16_t usLen);
uint8_t systemSetStructToEmbedFlash(uint8_t *ucaBuff,uint16_t usLen);
uint8_t systemGetRecordFormExMem(uint8_t *ucaBuff,uint16_t usLen);
uint8_t systemLoadCfgFile(void);
uint8_t systemParmInit(void);
uint8_t systemSetBuffToCfgPage(uint32_t ulAddr,uint8_t *ucaBuff,uint16_t usLen);
uint8_t systemSetStructToCfgPage(uint32_t ulAddr,uint8_t *ucaBuff,uint16_t usLen);
uint8_t systemGetRecordFromEmbedFlash(uint32_t ulAddr,uint8_t *ucaBuff,uint16_t usLen);
uint8_t systemSetBuffToFlash(uint32_t ulAddr,uint16_t *ucsBuff,uint16_t usLen);
void systemSetRecord(void);
void systemReadDurationTimeArray(void);
void systemWriteDurationTime(void);
void systemClrDurationTime(void);
void systemReadDurationTime(void);

extern systemTick_T g_tSystemTick;
extern wifiCfgStruct_T g_tWifiCfgStruct;

extern uint8_t g_ucDurationTimeIndex;
extern uint8_t g_ucWriteDurationTimeFlg;
extern uint8_t g_ucDurationTimeBuff[CFG_FILE_DURATIONA_LEN];
extern uint32_t  g_ulDurationTime;//耐久时间
extern uint16_t  g_usDurationIndex;
#endif 

