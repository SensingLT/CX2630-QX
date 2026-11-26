/**
  **************************************************************************
  * File   : exAdc.h
  * Version: V1.3.0
  * Date   : 2021-03-18
  * Brief  : Header file for AT-START board
  *          1. 
  */

#ifndef __EXADC_H
#define __EXADC_H	 
#include <at32f4xx.h>

/*ADC端口初始化*/
#define INH1_PIN                GPIO_Pins_9
#define INH1_GPIO               GPIOB
#define INH1_GPIO_RCC_CLK       RCC_AHBPERIPH_GPIOB
#define INH1_SET()              GPIO_WriteBit(INH1_GPIO, INH1_PIN, Bit_SET)      
#define INH1_CLR()              GPIO_WriteBit(INH1_GPIO, INH1_PIN, Bit_RESET)

#define INH2_PIN                GPIO_Pins_8
#define INH2_GPIO               GPIOB
#define INH2_GPIO_RCC_CLK       RCC_AHBPERIPH_GPIOB
#define INH2_SET()              GPIO_WriteBit(INH2_GPIO, INH2_PIN, Bit_SET)      
#define INH2_CLR()              GPIO_WriteBit(INH2_GPIO, INH2_PIN, Bit_RESET)

#define ADA_PIN                 GPIO_Pins_5
#define ADA_GPIO                GPIOB
#define ADA_GPIO_RCC_CLK        RCC_AHBPERIPH_GPIOB
#define ADA_SET()               GPIO_WriteBit(ADA_GPIO, ADA_PIN, Bit_SET)      
#define ADA_CLR()               GPIO_WriteBit(ADA_GPIO, ADA_PIN, Bit_RESET)

#define ADB_PIN                 GPIO_Pins_6
#define ADB_GPIO                GPIOB
#define ADB_GPIO_RCC_CLK        RCC_AHBPERIPH_GPIOB
#define ADB_SET()               GPIO_WriteBit(ADB_GPIO, ADB_PIN, Bit_SET)      
#define ADB_CLR()               GPIO_WriteBit(ADB_GPIO, ADB_PIN, Bit_RESET)

#define ADC_PIN                 GPIO_Pins_7
#define ADC_GPIO                GPIOB
#define ADC_GPIO_RCC_CLK        RCC_AHBPERIPH_GPIOB
#define ADC_SET()               GPIO_WriteBit(ADC_GPIO, ADC_PIN, Bit_SET)      
#define ADC_CLR()               GPIO_WriteBit(ADC_GPIO, ADC_PIN, Bit_RESET)




/* 只采 ADC_IN0 一个通道：DMA_BufferSize / ADC_NumOfChannel 必须与它一致 */
#define TOTAL_CHANNEL          1
//---变量配置
struct tagAdc1_T
{
    uint16_t m_usaADCbuff[TOTAL_CHANNEL];
    volatile uint8_t m_ucSampleCompleted;   /* 中断里置位，必须 volatile */
    
};
typedef struct tagAdc1_T adc1_T; 
extern adc1_T g_tAdc1;

void sampleInitAdcPort(void);
void exAdcInit(void);
void startAdc1(void);
#endif

