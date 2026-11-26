#include "exAdc.h"
#include "IoPort.h"

ADC_InitType ADC_InitStructure;
DMA_InitType DMA_InitStructure;
adc1_T g_tAdc1;

/**
  * @brief  ADC采样通道
  * @param  None
  * @retval None
  */
void sampleInitAdcPort(void)
{
    /* CR1..CR10 and DAT1..DAT12 control lines of the new RS2323 front end. */
    sampleSwitchInit();
}

/**
  * @brief  Configures the nested vectored interrupt controller.
  * @param  None
  * @retval None
  */
void NVIC_Adc1Configuration(void)
{
   NVIC_InitType NVIC_InitStructure;

  /* Enable the USART1 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = ADC_COMP_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  ADC采样通道
  * @param  None
  * @retval None
  */
void exAdcInit(void)
{
	  GPIO_InitType GPIO_InitStructure;
	
	  /* ADCCLK = PCLK2/6 */
		RCC_ADCCLKConfig(RCC_APB2CLK_Div6);
		
		/* Enable peripheral clocks ------------------------------------------------*/
		/* Enable DMA1 clocks */
		RCC_AHBPeriphClockCmd(RCC_AHBPERIPH_DMA1, ENABLE);
		/* Enable ADC1 and GPIOA GPIOB clocks */
		RCC_APB2PeriphClockCmd(RCC_APB2PERIPH_ADC1,ENABLE);
		RCC_AHBPeriphClockCmd(RCC_AHBPERIPH_GPIOA, ENABLE);
	
		GPIO_StructInit(&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pins = GPIO_Pins_0 ;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	
		/* DMA1 channel1 configuration ----------------------------------------------*/
		DMA_Reset(DMA1_Channel1);
		DMA_DefaultInitParaConfig(&DMA_InitStructure);
		DMA_InitStructure.DMA_PeripheralBaseAddr    = (uint32_t)&ADC1->RDOR;
		DMA_InitStructure.DMA_MemoryBaseAddr        = (uint32_t)g_tAdc1.m_usaADCbuff;
		DMA_InitStructure.DMA_Direction             = DMA_DIR_PERIPHERALSRC;
		DMA_InitStructure.DMA_BufferSize            = TOTAL_CHANNEL;
		DMA_InitStructure.DMA_PeripheralInc         = DMA_PERIPHERALINC_DISABLE;
		DMA_InitStructure.DMA_MemoryInc             = DMA_MEMORYINC_ENABLE;
		DMA_InitStructure.DMA_PeripheralDataWidth   = DMA_PERIPHERALDATAWIDTH_HALFWORD;
		DMA_InitStructure.DMA_MemoryDataWidth       = DMA_MEMORYDATAWIDTH_HALFWORD;
		DMA_InitStructure.DMA_Mode                  = DMA_MODE_CIRCULAR;
		DMA_InitStructure.DMA_Priority              = DMA_PRIORITY_HIGH;
		DMA_InitStructure.DMA_MTOM                  = DMA_MEMTOMEM_DISABLE;
		DMA_Init(DMA1_Channel1, &DMA_InitStructure);
		/* Enable DMA1 channel1 */
		DMA_ChannelEnable(DMA1_Channel1, ENABLE);

		/* ADC1 configuration ------------------------------------------------------*/
		ADC_StructInit(&ADC_InitStructure);
		ADC_InitStructure.ADC_Mode              = ADC_Mode_Independent;
		ADC_InitStructure.ADC_ScanMode          = ENABLE;
		ADC_InitStructure.ADC_ContinuousMode    = DISABLE;
		ADC_InitStructure.ADC_ExternalTrig      = ADC_ExternalTrig_None;
		ADC_InitStructure.ADC_DataAlign         = ADC_DataAlign_Right;
		ADC_InitStructure.ADC_NumOfChannel      = 1;
		ADC_Init(ADC1, &ADC_InitStructure);
		
		/* ADC1 regular channels configuration */ 
		ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_71_5);
		
		
		ADC_ExternalTrigConvCtrl(ADC1, ENABLE);
		/* Enable ADC1 DMA */
		ADC_DMACtrl(ADC1, ENABLE);
		
		/* Enable ADC1 */
		ADC_Ctrl(ADC1, ENABLE);
		ADC_INTConfig(ADC1, ADC_INT_EC, ENABLE);

		/* Enables Temperature Sensor and Vrefint Channel */
		ADC_TempSensorVrefintCtrl(ENABLE);
		/* Enable ADC1 reset calibration register */   
		ADC_RstCalibration(ADC1);
		/* Check the end of ADC1 reset calibration register */
		while(ADC_GetResetCalibrationStatus(ADC1));

		/* Start ADC1 calibration */
		ADC_StartCalibration(ADC1);
		/* Check the end of ADC1 calibration */
		while(ADC_GetCalibrationStatus(ADC1));
		/* Start ADC1 Software Conversion */ 
		NVIC_Adc1Configuration();
}

/**
  * @brief  ADC启动采样（单次转换，无均值滤波）
  * @param  None
  * @retval None
  */
void startAdc1(void)
{
    g_tAdc1.m_ucSampleCompleted = 0;
    ADC_SoftwareStartConvCtrl(ADC1, ENABLE);
}




