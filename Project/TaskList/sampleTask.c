#include "systemInit.h"
#include "sampleTask.h"
#include "exAdc.h"
#include "crc.h"
#include <math.h>
#include <string.h>
#include "yotliveMs128.h"
#include "systemInit.h"
#include "IoPort.h"
#include "uart.h"
#define RIGHT_SHIFT  4
#define RIGHT_SHIFT_10BIT  2
#define MAX12BIT_THRESHOLD   1023.0f


extern uint8_t g_ucYtSndFlg;
uint8_t   g_ucaPressureRaw[ROW][COLUMN];
uint16_t  g_ucaPerPressureRaw12Bit[ROW][COLUMN];
uint16_t  g_ucaPressureRaw12Bit[ROW][COLUMN];
uint16_t  g_ucaStablePressureRaw12Bit[ROW][COLUMN];
uint16_t  g_ucsPressureZero12Bit[ROW][COLUMN];
uint8_t   g_ucaPressureZeroRaw[ROW][COLUMN];

uint8_t   g_ucaPressureModbus[ROW*COLUMN*2+16];


uint16_t  g_usPressureRawArrayCnt,g_usCount;
uint16_t  g_usPressureRawArray[5][ROW][COLUMN];
uint8_t   g_ucPressureRawArray[5][ROW][COLUMN];

uint16_t  g_ucsPressureRaw12BitIn[ROW*COLUMN];    //滤波输入
uint16_t  g_ucsPressureRaw12BitOut[ROW*COLUMN];   //滤波输出
int       g_sPressureRawErrorIndex[ROW*COLUMN];   //扎穿异常数值的索引号
uint8_t   g_ucErrCnt;                             //记录异常扎穿点的数量


void antiExtreme(uint16_t* inMat, uint16_t* outMat, int rows, int cols, int* positions, int number);

void calibrationMat1020Mode7(void);
void calibrationMatMode4(void);
//中值滤波
uint16_t median_filter(uint8_t flag,uint16_t row,uint16_t colum,uint16_t n)
{
    uint16_t t_usSum,i,j,t_usTemp;

    if(flag == 0)//8bit
    {
        for (i = 1; i <= n-1; i++)
        {
            for (j = 0; j <= n - i -1; j++)
            {
                if (g_ucPressureRawArray[j][row][colum] > g_ucPressureRawArray[j + 1][row][colum])
                {
                    t_usTemp = g_ucPressureRawArray[j][row][colum];
                    g_ucPressureRawArray[j][row][colum] = g_ucPressureRawArray[j + 1][row][colum];
                    g_ucPressureRawArray[j + 1][row][colum] = t_usTemp;
                }
            }
        }
        //t_usSum = g_ucPressureRawArray[1][row][colum] + g_ucPressureRawArray[2][row][colum] + g_ucPressureRawArray[3][row][colum];
        //t_usSum = t_usSum / 3;
        t_usSum = g_ucPressureRawArray[1][row][colum];
    }
    else//10bit
    {
        for (i = 1; i <= n-1; i++)
        {
            for (j = 0; j <= n - i -1; j++)
            {
                if (g_usPressureRawArray[j][row][colum] > g_usPressureRawArray[j + 1][row][colum])
                {
                    t_usTemp = g_usPressureRawArray[j][row][colum];
                    g_usPressureRawArray[j][row][colum] = g_usPressureRawArray[j + 1][row][colum];
                    g_usPressureRawArray[j + 1][row][colum] = t_usTemp;
                }
            }
        }
        //t_usSum = g_usPressureRawArray[1][row][colum] + g_usPressureRawArray[2][row][colum] + g_usPressureRawArray[3][row][colum];
        //t_usSum = t_usSum / 3;
        t_usSum = g_usPressureRawArray[1][row][colum];
    }

    return t_usSum;
}



/* sampleTask.c —— 替换原来的 sampleDelayNus */
static uint8_t s_ucDelayInit = 0;

void sampleDelayNus(uint32_t usTime)
{
    uint32_t t_ulStart, t_ulTicks;

    if(!s_ucDelayInit)
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;  /* 使能 DWT */
        DWT->CYCCNT  = 0;
        DWT->CTRL   |= DWT_CTRL_CYCCNTENA_Msk;           /* 开始计数 */
        s_ucDelayInit = 1;
    }
    if(usTime == 0U) return;

    t_ulTicks = usTime * (SystemCoreClock / 1000000U);   /* 120 MHz -> 120 tick/µs */
    t_ulStart = DWT->CYCCNT;
    while((DWT->CYCCNT - t_ulStart) < t_ulTicks) { }
}

/**
  * @brief  10-channel mux select (channel order flipped)
  * @param  ucChannel  CH0..CH9, CHALL
  * @retval 0: ok, 1: invalid channel
  */
uint8_t sampleSelectChannel(uint8_t ucChannel)
{
    /* All CR rows off first: IN low -> CRx = GND. */
    DR_IO_1_SET();
    DR_IO_2_SET();
    DR_IO_3_SET();
    DR_IO_4_SET();
    DR_IO_5_SET();
    DR_IO_6_SET();
    DR_IO_7_SET();
    DR_IO_8_SET();
    DR_IO_9_SET();
    DR_IO_10_SET();

    if (ucChannel == CHALL)
    {
        sampleDelayNus(50);
        return 0;
    }

    switch (ucChannel)
    {
    case CH0: DR_IO_1_CLR();  break;   /* CR1  -> VS+ */
    case CH1: DR_IO_2_CLR();  break;   /* CR2  -> VS+ */
    case CH2: DR_IO_3_CLR();  break;   /* CR3  -> VS+ */
    case CH3: DR_IO_4_CLR();  break;   /* CR4  -> VS+ */
    case CH4: DR_IO_5_CLR();  break;   /* CR5  -> VS+ */
    case CH5: DR_IO_6_CLR();  break;   /* CR6  -> VS+ */
    case CH6: DR_IO_7_CLR();  break;   /* CR7  -> VS+ */
    case CH7: DR_IO_8_CLR();  break;   /* CR8  -> VS+ */
    case CH8: DR_IO_9_CLR();  break;   /* CR9  -> VS+ */
    case CH9: DR_IO_10_CLR(); break;   /* CR10 -> VS+ */
    default: return 1;
    }
    return 0;
}


/**
  * @brief  ADC??????
  * @param  None
  * @retval None
  */
void sampleSelectAdcChannel(uint8_t ucChannel)
{
    if(ucChannel < COLUMN)
    {
        sampleDatSelect(ucChannel);     /* all DAT guard, then DATx = GND */
    }
    else
    {
        sampleDatAllGuard();            /* all DAT = Vref */
    }
}


/**
  ******************************************************************************
  * 函数名：  loadCalParm
  * 功能：    加载分段参数表
  * 输入：    TxData 
  * 输出：    返回分段号
  */
uint8_t loadCalParm(void)
{
    uint8_t  t_ucCalNumber;  
 
		if(g_ulDurationTime < g_ptSystemConfiguration->m_tMatDuration.m_ulDuration[0])//计数时间第1段
		{
				t_ucCalNumber	 = 0;
		}
		else if((g_ulDurationTime >= g_ptSystemConfiguration->m_tMatDuration.m_ulDuration[0])
			   && (g_ulDurationTime < g_ptSystemConfiguration->m_tMatDuration.m_ulDuration[1]))
		{
				t_ucCalNumber	 = 1;
		}
		else if((g_ulDurationTime >= g_ptSystemConfiguration->m_tMatDuration.m_ulDuration[1])
			   && (g_ulDurationTime < g_ptSystemConfiguration->m_tMatDuration.m_ulDuration[2]))
		{
				t_ucCalNumber	 = 2;
		}
		else if((g_ulDurationTime >= g_ptSystemConfiguration->m_tMatDuration.m_ulDuration[2])
			   && (g_ulDurationTime < g_ptSystemConfiguration->m_tMatDuration.m_ulDuration[3]))
		{
				t_ucCalNumber	 = 3;
		}
		else
		{
				t_ucCalNumber	 = 4;
		}	
    
    return t_ucCalNumber;
    
}


/**
  * @brief  采样任务
  * @param  None
  * @retval None
  */
uint32_t g_ulAdcTimeout;
uint8_t  g_ucScaleA,g_ucScaleB;
uint16_t t_usThreshold_1,t_usThreshold_2,t_usThreshold_3,t_usThreshold_4;
float t_fK1,t_fB1,t_fK2,t_fB2,t_fK3,t_fB3,t_fK4,t_fB4;
float t_fTemp,t_fMid;

uint32_t g_ulAdcTimeout;
uint8_t t_ucSta;
uint32_t g_ulAdcTimeout;
uint8_t  g_ucScale;
uint16_t t_usTempZero;
#define RIGHT_SHIFT_10BIT   2
#define ZERO_MAX_12BIT      1023u
#define MAT_ADC_TIMEOUT     600u


#ifndef SETTLE_ROW_US
#define SETTLE_ROW_US       100
#endif
#ifndef SETTLE_COL_US
#define SETTLE_COL_US       50
#endif

void sampleTask(void)
{
    uint16_t t_usRow, t_usCol;
    uint16_t t_usRaw;                                   /* 局部中间量 */
    uint16_t t_usZero, t_usDiff;
    uint32_t t_ulGuard;
    uint32_t t_ulIndex;
    uint8_t  t_ucAbort = 0;
    volatile uint8_t *pDone = &g_tAdc1.m_ucSampleCompleted;

    g_ucScaleA = 0;
    g_ucScaleB = 1;
    g_usCount  = 3;
    WDTCLR();

    t_ulIndex = 0;
    for(t_usRow = 0; t_usRow < ROW; t_usRow++)
    {
        sampleSelectChannel((uint8_t)t_usRow);
        sampleDelayNus(SETTLE_ROW_US);

        for(t_usCol = 0; t_usCol < COLUMN; t_usCol++)
        {
            sampleSelectAdcChannel((uint8_t)t_usCol);
            sampleDelayNus(SETTLE_COL_US);
            startAdc1();

            t_ulGuard = MAT_ADC_TIMEOUT;
            while(*pDone == 0)
            {
                if(--t_ulGuard == 0) { t_ucAbort = 1; break; }
            }
            if(t_ucAbort) break;

            t_usRaw = (uint16_t)(g_tAdc1.m_usaADCbuff[g_ucScaleA] >> RIGHT_SHIFT_10BIT);

            /* ===== 零点处理 ===== */
            t_usZero = *(volatile uint16_t *)(ZERO_FILE_STARTADDR + t_ulIndex * 2);
            t_ulIndex++;
            if(t_usZero > ZERO_MAX_12BIT) t_usZero >>= 2;

            /* ===== 反转 ===== */
            if(t_usZero == 0)
            {
                /* 无零点：满量程反转，压力越大值越大（空载不为 0，带固定偏置） */
                t_usDiff = (uint16_t)(ZERO_MAX_12BIT - t_usRaw);
            }
            else
            {
                /* 有零点：扣零点 */
                t_usDiff = (t_usZero > t_usRaw) ? (uint16_t)(t_usZero - t_usRaw) : 0;
            }
            if(t_usDiff > ZERO_MAX_12BIT) t_usDiff = ZERO_MAX_12BIT;

            g_ucaPressureRaw12Bit[t_usRow][t_usCol] = t_usDiff;          /* 12bit */
            g_ucaPressureRaw[t_usRow][t_usCol]      = (uint8_t)(t_usDiff >> 2);  /* 8bit 观察用 */
        }

        sampleSelectChannel(CHALL);
        WDTCLR();
        if(t_ucAbort) break;
    }

    if(!t_ucAbort)
    {
        if(g_ptSystemConfiguration->m_tMatSystemCfg.m_ucCaliMode != 0)
        {
            calibrationMatMode4();
        }
    }
}

decoded_sample_t g_tSampleCode;
void sndMsg(void)
{
	g_tSampleCode.header.magic = 0x6868;
	g_tSampleCode.header.code = g_ptSystemConfiguration->m_tMatCom.m_usId;
	g_tSampleCode.header.cmd = 0x3E;
	g_tSampleCode.header.length = LENGTH + 3;
	g_tSampleCode.cols = COLUMN;
	g_tSampleCode.rows = ROW; 
	g_tSampleCode.sides = 1;
	memcpy(g_tSampleCode.data,g_ucaPressureRaw12Bit,sizeof(g_ucaPressureRaw12Bit));
	g_tSampleCode.crc = calcCrc16Modbus(0XFFFF,(uint8_t*)&g_tSampleCode.header.code,g_tSampleCode.header.length+sizeof(msg_header_t)-2);
	uart1SendMsg((uint8_t*)&g_tSampleCode.header.magic,sizeof(decoded_sample_t));
}


uint32_t g_ulSum,g_ulAvg;

/**
  * @brief  12bit 扎穿异常点中值滤波(模式4、模式7 公用)
  * @param  None
  * @retval None
  */
void filterExtreme12Bit(void)
{
    uint16_t i,j;
    uint16_t t_usNumber;
    uint32_t t_ulIndex;

    g_ucErrCnt = 0;
    t_ulIndex = 0;
    t_usNumber = 0;
    for(i=0;i<ROW;i++)
    {
        for(j=0;j<COLUMN;j++)
        {
            if(g_ucaPressureRaw12Bit[i][j] > ERROR_POINT)
            {
                g_sPressureRawErrorIndex[t_usNumber] = t_ulIndex;
                t_usNumber++;
                g_ucErrCnt++;
            }
            t_ulIndex++;
        }
    }
    if(g_ucErrCnt!=0)
    {
        memcpy(g_ucsPressureRaw12BitIn,&g_ucaPressureRaw12Bit,sizeof(g_ucaPressureRaw12Bit));
        memcpy(g_ucsPressureRaw12BitOut,&g_ucaPressureRaw12Bit,sizeof(g_ucaPressureRaw12Bit));
        antiExtreme(g_ucsPressureRaw12BitIn, g_ucsPressureRaw12BitOut, ROW, COLUMN,g_sPressureRawErrorIndex,g_ucErrCnt);
        memcpy(g_ucaPressureRaw12Bit,g_ucsPressureRaw12BitOut,sizeof(g_ucaPressureRaw12Bit));
    }
}

/**
  * @brief  模式7标定参数有效性判断
  * @param  fK 当前使用的补偿系数
  * @retval 1=有效 0=未标定(0xFF/异常值)
  */
uint8_t calMode7ParmValid(float fK)
{
    if(fK != fK)return 0;      /*NaN，数据区未写入(全0xFF)*/
    if((fK > -0.001f) && (fK < 0.001f))return 0;
    if(fK > 1000.0f)return 0;
    if(fK < -1000.0f)return 0;
    return 1;
}

/**
  * @brief  模式4标定参数有效性判断
  * @param  pBuf     每点40字节参数
  * @param  usMaxRaw 满量程(8bit=255,12bit=1023)
  * @retval 1=有效 0=未标定
  */
uint8_t calMode4ParmValid(uint8_t *pBuf,uint16_t usMaxRaw)
{
    uint8_t  t_ucaMid[4];
    uint16_t t_usT1,t_usT2,t_usT3,t_usCount;
    float    t_fK,t_fB;

    t_usT1 = pBuf[1]*256 + pBuf[0];
    t_usT2 = pBuf[11]*256 + pBuf[10];
    t_usT3 = pBuf[21]*256 + pBuf[20];
    if((t_usT1==0xFFFF) && (t_usT2==0xFFFF) && (t_usT3==0xFFFF))return 0;/*未写入*/
    if(t_usT1 > t_usT2)return 0;
    if(t_usT2 > t_usT3)return 0;
    if(t_usT3 > usMaxRaw)return 0;
    for(t_usCount=0;t_usCount<4;t_usCount++)
    {
        memcpy(t_ucaMid,&pBuf[2+t_usCount*10],4);
        t_fK = *(float *)t_ucaMid;
        memcpy(t_ucaMid,&pBuf[6+t_usCount*10],4);
        t_fB = *(float *)t_ucaMid;
        if(t_fK != t_fK)return 0;
        if(t_fB != t_fB)return 0;
        if((t_fK > -0.001f) && (t_fK < 0.001f))return 0;
        if(t_fK > 1000.0f)return 0;
        if(t_fK < -1000.0f)return 0;
        if(t_fB < -100000.0f)return 0;
        if(t_fB > 100000.0f)return 0;
    }
    return 1;
}

/**
  * @brief  标定模式4：每点4段折线补偿
  *         参数区 CALMODE4_ADDR_START，每点 CALMODE4_POINT_LEN(40) 字节：
  *         [0..1]门限1   [2..5]K1   [6..9]B1
  *         [10..11]门限2 [12..15]K2 [16..19]B2
  *         [20..21]门限3 [22..25]K3 [26..29]B3
  *         [30..31]门限4 [32..35]K4 [36..39]B4
  *         补偿公式：out = clamp(in*K + B, 0, 满量程)
  *         参数为全0xFF(未写过)的点保持原值
  * @param  None
  * @retval None
  */
void calibrationMatMode4(void)
{
    uint16_t i,j,n,m;
    uint16_t t_usCount;
    uint32_t t_ulIndex;
    uint8_t  t_ucaTemp[CALMODE4_POINT_LEN];
    uint8_t  t_ucaMid[4];
    uint16_t t_usMaxRaw;
    uint16_t t_usThreshold_1,t_usThreshold_2,t_usThreshold_3;
    float    t_fTemp,t_fMid,t_fK,t_fB;

    t_usMaxRaw = 1023;
    if(g_ptSystemConfiguration->m_tMatSystemCfg.m_ucaDataFrame == 1)
    {
        t_usMaxRaw = 255;
    }

    switch(g_ptSystemConfiguration->m_tMatSystemCfg.m_ucaDataFrame)
    {
    case 1://8bit
        n = 0;
        for(i=0;i<ROW;i++)
        {
            for(j=0;j<COLUMN;j++)
            {
                t_ulIndex = n * CALMODE4_POINT_LEN;
                if((t_ulIndex + CALMODE4_POINT_LEN) > CALMODE4_FILE_LEN)
                {
                    return;/*超出模式4参数区长度，后面的点无法补偿*/
                }
                for(t_usCount=0;t_usCount<CALMODE4_POINT_LEN;t_usCount++)
                {
                    t_ucaTemp[t_usCount] = *(uint8_t *)(CALMODE4_ADDR_START+t_ulIndex + t_usCount);
                }
                if(calMode4ParmValid(t_ucaTemp,t_usMaxRaw)==1)
                {
                    t_usThreshold_1 = t_ucaTemp[1]*256 + t_ucaTemp[0];
                    t_usThreshold_2 = t_ucaTemp[11]*256 + t_ucaTemp[10];
                    t_usThreshold_3 = t_ucaTemp[21]*256 + t_ucaTemp[20];
                    if(g_ucaPressureRaw[i][j] < t_usThreshold_1)
                    {
                        m = 0;
                    }
                    else if(g_ucaPressureRaw[i][j] < t_usThreshold_2)
                    {
                        m = 1;
                    }
                    else if(g_ucaPressureRaw[i][j] < t_usThreshold_3)
                    {
                        m = 2;
                    }
                    else
                    {
                        m = 3;
                    }
                    for(t_usCount=0;t_usCount<4;t_usCount++)
                    {
                        t_ucaMid[t_usCount] = t_ucaTemp[2+m*10+t_usCount];
                    }
                    t_fK = *(float *)t_ucaMid;
                    for(t_usCount=0;t_usCount<4;t_usCount++)
                    {
                        t_ucaMid[t_usCount] = t_ucaTemp[6+m*10+t_usCount];
                    }
                    t_fB = *(float *)t_ucaMid;
                    t_fTemp = (float)g_ucaPressureRaw[i][j];
                    t_fMid  = t_fTemp * t_fK + t_fB;
                    if(t_fMid<0)
                        t_fMid=0;
                    if(t_fMid>MAX_8BITVAL)
                        t_fMid=MAX_8BITVAL;
                    g_ucaPressureRaw[i][j] = (uint8_t)t_fMid;
                }
                n++;
            }
        }
        break;
    case 2://12bit
        filterExtreme12Bit();
        n = 0;
        for(i=0;i<ROW;i++)
        {
            for(j=0;j<COLUMN;j++)
            {
                t_ulIndex = n * CALMODE4_POINT_LEN;
                if((t_ulIndex + CALMODE4_POINT_LEN) > CALMODE4_FILE_LEN)
                {
                    return;/*超出模式4参数区长度，后面的点无法补偿*/
                }
                for(t_usCount=0;t_usCount<CALMODE4_POINT_LEN;t_usCount++)
                {
                    t_ucaTemp[t_usCount] = *(uint8_t *)(CALMODE4_ADDR_START+t_ulIndex + t_usCount);
                }
                if(calMode4ParmValid(t_ucaTemp,t_usMaxRaw)==1)
                {
                    t_usThreshold_1 = t_ucaTemp[1]*256 + t_ucaTemp[0];
                    t_usThreshold_2 = t_ucaTemp[11]*256 + t_ucaTemp[10];
                    t_usThreshold_3 = t_ucaTemp[21]*256 + t_ucaTemp[20];
                    if(g_ucaPressureRaw12Bit[i][j] < t_usThreshold_1)
                    {
                        m = 0;
                    }
                    else if(g_ucaPressureRaw12Bit[i][j] < t_usThreshold_2)
                    {
                        m = 1;
                    }
                    else if(g_ucaPressureRaw12Bit[i][j] < t_usThreshold_3)
                    {
                        m = 2;
                    }
                    else
                    {
                        m = 3;
                    }
                    for(t_usCount=0;t_usCount<4;t_usCount++)
                    {
                        t_ucaMid[t_usCount] = t_ucaTemp[2+m*10+t_usCount];
                    }
                    t_fK = *(float *)t_ucaMid;
                    for(t_usCount=0;t_usCount<4;t_usCount++)
                    {
                        t_ucaMid[t_usCount] = t_ucaTemp[6+m*10+t_usCount];
                    }
                    t_fB = *(float *)t_ucaMid;
                    t_fTemp = (float)g_ucaPressureRaw12Bit[i][j];
                    t_fMid  = t_fTemp * t_fK + t_fB;
                    if(t_fMid<0)
                        t_fMid=0;
                    if(t_fMid>MAX_12BITVAL)
                        t_fMid=MAX_12BITVAL;
                    g_ucaPressureRaw12Bit[i][j] = (uint16_t)t_fMid;
                }
                n++;
            }
        }
        break;
    }
}

void calibrationMat1020Mode7(void)
{
	  uint16_t i,j,n,m;
	  uint8_t t_ucaMid[5][4];
	  uint32_t t_ulIndex;
	  uint16_t t_usCount;
	  uint8_t t_ucaTemp[40];
	
	g_usDurationIndex = loadCalParm();
    if(g_ptSystemConfiguration->m_tMatSystemCfg.m_ucCaliMode == 7)
    { 			
				switch(g_ptSystemConfiguration->m_tMatSystemCfg.m_ucaDataFrame)
        {
          case 1://8bit
          n=0; 
          for(i=0;i<ROW;i++)
          {
              for(j=0;j<COLUMN;j++)
              {
                  t_ulIndex = n * 20;
                  for(t_usCount=0;t_usCount<20;t_usCount++)
                  {
                      t_ucaTemp[t_usCount] = *(uint8_t *)(CALMODE7_ADDR_START+t_ulIndex + t_usCount);
                  }
                  t_fTemp = (float)g_ucaPressureRaw[i][j]; 
									for(m=0;m<4;m++)
									{
											t_ucaMid[g_usDurationIndex][m] = t_ucaTemp[m+4*g_usDurationIndex];
									}
									t_fK1 = *(float *)t_ucaMid[g_usDurationIndex];
									if(calMode7ParmValid(t_fK1)==1)/*数据区未标定(0xFF/NaN)时保持原值*/
									{
										t_fMid= t_fTemp * t_fK1;
										if(t_fMid<0)
											t_fMid=0;
										if(t_fMid>MAX_8BITVAL)
											t_fMid=MAX_8BITVAL;
										 g_ucaPressureRaw[i][j] = (uint8_t)t_fMid;
									}
                  n++;
              }
         }
         break;
         case 2://10BIT
					filterExtreme12Bit();
					
         n=0;
         for(i=0;i<ROW;i++)
         {
						for(j=0;j<COLUMN;j++)
            {
                t_ulIndex = n * 20;
                for(t_usCount=0;t_usCount<20;t_usCount++)
                {
										t_ucaTemp[t_usCount] = *(uint8_t *)(CALMODE7_ADDR_START+t_ulIndex + t_usCount);
                }
                t_fTemp = (float)g_ucaPressureRaw12Bit[i][j]; 
								for(m=0;m<4;m++)
								{
										t_ucaMid[g_usDurationIndex][m] = t_ucaTemp[m+4*g_usDurationIndex];
								}
								t_fK1 = *(float *)t_ucaMid[g_usDurationIndex];
								if(calMode7ParmValid(t_fK1)==1)/*数据区未标定(0xFF/NaN)时保持原值*/
								{
									t_fMid= t_fTemp * t_fK1;
									if(t_fMid<0)
										t_fMid=0;
									if(t_fMid>MAX_12BITVAL)
										t_fMid=MAX_12BITVAL;
									 g_ucaPressureRaw12Bit[i][j] = (uint16_t)t_fMid;
								}
                n++;
              }
          }
          break;        
					}			
      }		
}



/**
  * @brief  选择异常状态下处理扎穿
  * @param  inMat  输入矩阵
	          outMat 输出矩阵
						rows   传感器扫描行
						cols   传感器扫描列
						positions 异常数据的索引号存放的数据
						number 扎穿异常点的数量
  * @retval None
  */
void selectionSort(uint16_t arr[], int n);
void antiExtreme(uint16_t* inMat, uint16_t* outMat, int rows, int cols, int* positions, int number){
	int i, j;
	int k, index_row, index_col;
	uint16_t windowBuffer[9];
	// Apply customized median filter to the extreme positions with repeated paddings.
	for(int n=0;n<number;n++){	// "number" is the number of extreme points
		k=0;
		i = positions[n]/cols;
		j = positions[n]%cols;
		// A sliding 3-by-3 2D window.
		for(int x=0;x<3;x++){
			for(int y=0;y<3;y++){
				// Check row boundaries.
				if(i-1+x<0){
					index_row = i-1+x+1;
				}else if(i-1+x>rows-1){
					index_row = i-1+x-1;
				}else{	// not at row boundaries
					index_row = i-1+x;
				}
				// Check column boundaries.
				if(j-1+y<0){
					index_col = j-1+y+1;
				}else if(j-1+y>cols-1){
					index_col = j-1+y-1;
				}else{	// not at column boundaries
					index_col = j-1+y;
				}
				windowBuffer[k] = inMat[index_row*cols+index_col];
				k=k+1;
			}
		}
		// Sort the buffered values.
		selectionSort(windowBuffer, 9);
		// Add the median to output.
		outMat[i*cols+j]=windowBuffer[4];
	}
}


// Function to sawp values (used by selectionSort).
void swap(uint16_t* xp, uint16_t* yp){ 
    uint16_t temp = *xp; 
    *xp = *yp; 
    *yp = temp; 
}

// Function to perform selection sort in ascending order (used by antiExtreme).
void selectionSort(uint16_t arr[], int n){ 
    int i, j, min_idx; 
    // One by one move boundary of unsorted subarray 
    for(i=0; i<n-1; i++){ 
        // Find the minimum element in unsorted array 
        min_idx = i; 
        for(j=i+1; j<n; j++){
            if (arr[j] < arr[min_idx]){
                min_idx = j;
			}
		}
        // Swap the found minimum element with the first element.
        swap(&arr[min_idx], &arr[i]);
    }
}