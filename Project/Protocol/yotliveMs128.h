#ifndef _YOTLIVEMS128_H
#define _YOTLIVEMS128_H

#include <at32f4xx.h>

#define YT_HEAD   0x68
#define  CFG_FILE_EMBEDFLASH_TOTALPACKAGES     15
#define  CFG_FILE_PACKAGE     256
#define  CFG_FILE_PACKAGE_64B     64


#define  CALI7_FILE_EMBEDFLASH_TOTALPACKAGES     19
#define  CALI4_FILE_EMBEDFLASH_TOTALPACKAGES     20   /*模式4：20包×256字节=5120字节(整块模式4区)*/

#define  CFG_FILE_EMBEDFLASH_TOTALPACKAGES     15
#define  CFG_FILE_PACKAGE     256
#define  CFG_FILE_PACKAGE_64B     64

#define WINS_2017D_FIX_FRAME               10

#define WIN_2017D_BRO             0x7f  //广播      
#define WIN_2017D_UNLOCK          0x05  //开锁
#define WIN_2017D_UPGRADE         0x06  //升级文件
#define WIN_2017D_UPGRADE_END     0x07  //升级文件结束
#define WIN_2017D_UPGRADE_ALLOW   0X08  //允许升级        
#define WIN_2017D_CFG             0x09  //配置文件写入
#define WIN_2017D_READCFG         0x0A  //配置文件读取
#define WIN_2017D_WRFILE          0x0B  //写标定文件(ms256工具"写模式4"用此命令)
#define WIN_2017D_READFILE        0x0C  //配置文件读取

#define WIN_2017D_CFG_INIT        0x16  //配置文件写入
#define WIN_2017D_READCFG_INIT    0x17  //配置文件读取



#define WIN_2017D_WRTHRESHOLED    0x0F  //配置文件写入
#define WIN_2017D_READTHRESHOLED  0x10  //配置文件读取
#define WIN_2017D_WRSERNO         0x11   //写序列号
#define WIN_2017D_RATEING         20   //标定数据
#define WIN_2017D_RSTRATEING      21   //复位标定数据

#define WIN_2017D_WR_SEGMENT      24   //写分段文件门限和初值
#define WIN_2017D_RD_SEGMENT      25   //读分段门限和初值

#define WIN_2017D_VERSION         0x7E  //版本信息
#define WIN_2017D_RST             0x0E  //复位 


#define WIN_2017D_RATEING         20   //????
#define WIN_2017D_RSTRATEING      21   //??????

#define WIN_2017D_WR_THRESHOLD_COUNT    27   //写计数器
#define WIN_2017D_RD_THRESHOLD_COUNT    28   //读计数器

#define WIN_2017D_WR_QSMCFG    30   //写文件
#define WIN_2017D_RD_QSMCFG    31   //读文件

#define WIN_2017D_RESET_EXEEPROM          0x42
#define WIN_2017D_READ_RECORD             0x43
#define WIN_2017D_READ_LEARNING_ITEMS     0x44
#define WIN_2017D_RESET_LEARNINGITEMS     0x45
#define WIN_2017D_SET_WIFI_LOCATION       0x46
#define WIN_2017D_GET_WIFI_LOCATION       0x47
#define WIN_2017D_WRCALMODE7      0x26  //写入模式7
#define WIN_2017D_RDCALMODE7      0x27  //读取模式7
#define WIN_2017D_WRCALMODE4      0x20  //写入模式4(旧版工具命令，兼容保留)
#define WIN_2017D_RDCALMODE4      0x21  //读取模式4
#define WIN_2017D_CLRDURATIONTIME 40  // 
#define WIN_2017D_RDDURATIONTIME  41
#define WIN_2017D_WRDURATIONTIME  42

#define CMD_SET_UNCALIBRATED 0x30 //传输原始数据
#define CMD_SET_CALIBRATED 0x31 //传输校验后的数据(模式7)
#define CMD_SET_CALIBRATED_MODE4 0x32 //传输校验后的数据(模式4)

#pragma pack(1)


struct tagCytL
{
  uint16_t FIN:1;
  uint16_t FIR:1;
  uint16_t Count:12;
  uint16_t rev1:1;
  uint16_t rev2:1;
};
typedef struct tagCytL cytL_T;

//动态帧长
struct tagYtFrame
{
    uint8_t  m_ucHead;  
    uint32_t m_ulAddr;
    uint8_t  m_ucCmd;
    cytL_T   m_tCd;
    uint16_t m_usLen;
    uint8_t  m_ucpDat[];//柔性数组
};
typedef struct tagYtFrame ytFrame_T;


struct tagYotlivePt_T
{
    uint8_t  m_ucHeadH;
    uint8_t  m_ucHeadL;
    uint16_t m_usAddr;
    uint8_t  m_ucCmd;
    uint16_t m_usLen;
    uint8_t  m_ucpDat[];//柔性数组
};
typedef struct tagYotlivePt_T YotlivePt_T;

#pragma pack()

uint8_t usbCmdSndDat(void);
uint8_t debugMtCmdSndDat(void);
extern uint8_t  g_ucDebugSndFlg;

#endif
