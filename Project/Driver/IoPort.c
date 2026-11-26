#include "IoPort.h"

typedef struct
{
    GPIO_Type *port;
    uint16_t   pin;
} io_pin_t;

static const io_pin_t s_cr_pins[10] =
{
    { GPIOA, GPIO_Pins_4  },
    { GPIOA, GPIO_Pins_5  },
    { GPIOA, GPIO_Pins_6  },
    { GPIOA, GPIO_Pins_7  },
    { GPIOB, GPIO_Pins_0  },
    { GPIOB, GPIO_Pins_1  },
    { GPIOB, GPIO_Pins_2  },
    { GPIOB, GPIO_Pins_10 },
    { GPIOB, GPIO_Pins_11 },
    { GPIOB, GPIO_Pins_12 }
};

static const io_pin_t s_dat_pins[12] =
{
    { GPIOB, GPIO_Pins_13 },
    { GPIOB, GPIO_Pins_14 },
    { GPIOB, GPIO_Pins_15 },
    { GPIOA, GPIO_Pins_8  },
    { GPIOA, GPIO_Pins_11 },
    { GPIOA, GPIO_Pins_12 },
    { GPIOF, GPIO_Pins_6  },
    { GPIOF, GPIO_Pins_7  },
    { GPIOA, GPIO_Pins_15 },
    { GPIOB, GPIO_Pins_3  },
    { GPIOB, GPIO_Pins_4  },
    { GPIOB, GPIO_Pins_5  }
};

static void gpio_out_init(GPIO_Type *port, uint16_t pin, uint8_t level)
{
    GPIO_InitType gpio;

    GPIO_WriteBit(port, pin, level ? Bit_SET : Bit_RESET);

    GPIO_StructInit(&gpio);
    gpio.GPIO_Pins     = pin;
    gpio.GPIO_Mode     = GPIO_Mode_OUT;
    gpio.GPIO_OutType  = GPIO_OutType_PP;
    gpio.GPIO_Pull     = GPIO_Pull_NOPULL;
    gpio.GPIO_MaxSpeed = GPIO_MaxSpeed_2MHz;
    GPIO_Init(port, &gpio);
}

/**
  * @brief  delay
  * @param  None
  * @retval None
  */
void ioDelay(uint32_t us)
{
    uint32_t i, j;

    for (j = 0; j < us; j++)
    {
        for (i = 0; i < 115; i++)
        {
            __NOP();
        }
    }
}

/**
  * @brief  LED灯端口初始化
  * @param  None
  * @retval None
  */
void GpioInit(void)
{
    GPIO_InitType GPIO_InitStructure;

    RCC_AHBPeriphClockCmd(LEDRED_GPIO_RCC_CLK, ENABLE);

    LEDRED_SET();
    LEDGREEN_SET();

    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pins     = LEDRED_PIN | LEDGREEN_PIN;
    GPIO_InitStructure.GPIO_Mode     = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OutType  = GPIO_OutType_PP;
    GPIO_InitStructure.GPIO_Pull     = GPIO_Pull_NOPULL;
    GPIO_InitStructure.GPIO_MaxSpeed = GPIO_MaxSpeed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    LEDRED_SET();       /* off */
    LEDGREEN_SET();     /* off */
}

/**
  * @brief  Configure the 22 RS2323 control lines.
  *         CR1..CR10 = low  (rows off, CRx = GND)
  *         DAT1..12  = high (guard, DATx = Vref)
  */
void sampleSwitchInit(void)
{
    uint8_t i;

    RCC_AHBPeriphClockCmd(RCC_AHBPERIPH_GPIOA |
                          RCC_AHBPERIPH_GPIOB |
                          RCC_AHBPERIPH_GPIOF, ENABLE);

    for (i = 0; i < 10; i++)
    {
        gpio_out_init(s_cr_pins[i].port, s_cr_pins[i].pin, 0);
    }
    for (i = 0; i < 12; i++)
    {
        gpio_out_init(s_dat_pins[i].port, s_dat_pins[i].pin, 1);
    }
}

/**
  * @brief  All DAT lines to GND (guard).
  */
void sampleDatAllGuard(void)
{
    uint8_t i;

    for (i = 0; i < 12; i++)
    {
        GPIO_WriteBit(s_dat_pins[i].port, s_dat_pins[i].pin, Bit_RESET);
    }
}

/**
  * @brief  Select one DAT column (to VREF), all others stay at Vref.
  * @param  index 0..11
  */
void sampleDatSelect(uint8_t index)
{
    sampleDatAllGuard();

    if (index < 12)
    {
        GPIO_WriteBit(s_dat_pins[index].port, s_dat_pins[index].pin, Bit_SET);
    }
}

void sampleRstChannel(void)
{
}
