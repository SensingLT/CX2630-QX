#include "time.h"

TMR_TimerBaseInitType  TMR_TMReBaseStructure;
TMR_OCInitType  TMR_OCInitStructure;
uint16_t PrescalerValue = 0;
uint32_t g_ulTickUs;
uint32_t g_ulTickMs;

/*delay variable*/
static __IO uint32_t fac_us;
static __IO uint32_t fac_ms;

/**
  * @brief  Configure the nested vectored interrupt controller.
  * @param  None
  * @retval None
  */
void NVIC_Time3Configuration(void)
{
		NVIC_InitType NVIC_InitStructure;

		/* Enable the TMR3 global Interrupt */
		NVIC_InitStructure.NVIC_IRQChannel = TMR3_GLOBAL_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

		NVIC_Init(&NVIC_InitStructure);
}


/**
  * @brief  定时器3,10ms时钟节拍
  * @param  None
  * @retval None
  */
uint8_t time3Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1PERIPH_TMR3, ENABLE);
	  NVIC_Time3Configuration();
		PrescalerValue = (uint16_t) (SystemCoreClock / 1200000) - 1;/*100分频*/
		TMR_TimeBaseStructInit(&TMR_TMReBaseStructure);
		TMR_TMReBaseStructure.TMR_Period = 12000;
		TMR_TMReBaseStructure.TMR_DIV = PrescalerValue;
		TMR_TMReBaseStructure.TMR_ClockDivision = 0;
		TMR_TMReBaseStructure.TMR_CounterMode = TMR_CounterDIR_Up;
		TMR_TimeBaseInit(TMR3, &TMR_TMReBaseStructure);
		//TMR_DIVConfig(TMR3, PrescalerValue, TMR_DIVReloadMode_Immediate);
		TMR_INTConfig(TMR3, TMR_INT_Overflow, ENABLE);
		/* TMR3 enable counter */
		TMR_Cmd(TMR3, ENABLE);
		return 0;
}

/**
  * @brief  initialize Delay function   
  * @param  None
  * @retval None
  */		   
void Delay_init(void)
{
  /*Config Systick*/
  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
  fac_us = SystemCoreClock / (1000000U);
  fac_ms = fac_us * (1000U);
}


/**
  * @brief  Inserts a delay time.
  * @param  nus: specifies the delay time length, in microsecond.
  * @retval None
  */
void Delay_us(u32 nus)
{
  u32 temp;
  SysTick->LOAD = (u32)(nus*fac_us);
  SysTick->VAL = 0x00;
  SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk ;
  do
  {
    temp = SysTick->CTRL;
  }while((temp & 0x01) &&! (temp & (1<<16)));

  SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
  SysTick->VAL = 0x00;
}

/**
  * @brief  Inserts a delay time.
  * @param  nms: specifies the delay time length, in milliseconds.
  * @retval None
  */
void Delay_ms(u16 nms)
{
  u32 temp;
  while(nms)
  {
    if(nms > STEP_DELAY_MS)
    {
      SysTick->LOAD = (u32)(STEP_DELAY_MS * fac_ms);
      nms -= STEP_DELAY_MS;
    }
    else
    {
      SysTick->LOAD = (u32)(nms * fac_ms);
      nms = 0;
    }
    SysTick->VAL = 0x00;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    do
    {
      temp = SysTick->CTRL;
    }while( (temp & 0x01) && !(temp & (1<<16)) );

    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->VAL = 0x00;
  }
}


/**
  * @brief  Inserts a delay time.
  * @param  nms: specifies the delay time length, in milliseconds.
  * @retval None
  */
void delayMs(u16 nms)
{
	uint32_t i,j,k;
	
	for(k=0;k<nms;k++)
	{
			for(j=0;j<1000;j++)
			{
					for(i=0;i<120;i++)
					{
							__NOP();
					}
			}
	}
	
}

/**
  * @brief  Inserts a delay time.
  * @param  sec: specifies the delay time length, in seconds.
  * @retval None
  */
void delaySec(u16 sec)
{
  u16 i;
  for(i=0; i<sec; i++)
  {
    delayMs(500);
    delayMs(500);
  }
}

