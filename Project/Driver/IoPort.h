/**
  **************************************************************************
  * File   : IoPort.h
  * Version: V1.3.0
  * Date   : 2021-03-18
  * Brief  : Header file for AT-START board
  *
  *  Adapted for SS-CX26xx-Prj1 (2026-10-20):
  *    - LEDs moved to PB8 (RED) / PB9 (GREEN), active low.
  *    - DR_IO_1..10 now drive the CR1..CR10 RS2323 inputs.
  *      IN high -> CRx = Vs+ (row ON); IN low -> CRx = GND (row OFF).
  *    - DAT1..DAT12 drive the RS2323 inputs of the 12 columns.
  *      IN high -> DATx = Vref (guard); IN low -> DATx = GND (selected).
  **************************************************************************
  */

#ifndef __IOPORT_H
#define __IOPORT_H
#include <at32f4xx.h>

/*LED端口初始化*/
#define LEDRED_PIN              GPIO_Pins_8
#define LEDRED_GPIO             GPIOB
#define LEDRED_GPIO_RCC_CLK     RCC_AHBPERIPH_GPIOB
#define LEDRED_SET()            GPIO_WriteBit(LEDRED_GPIO, LEDRED_PIN, Bit_SET)
#define LEDRED_CLR()            GPIO_WriteBit(LEDRED_GPIO, LEDRED_PIN, Bit_RESET)

#define LEDGREEN_PIN             GPIO_Pins_9
#define LEDGREEN_GPIO            GPIOB
#define LEDGREEN_GPIO_RCC_CLK    RCC_AHBPERIPH_GPIOB
#define LEDGREEN_SET()           GPIO_WriteBit(LEDGREEN_GPIO, LEDGREEN_PIN, Bit_SET)
#define LEDGREEN_CLR()           GPIO_WriteBit(LEDGREEN_GPIO, LEDGREEN_PIN, Bit_RESET)


/* CR row switches: IN1..IN10 -> CR1..CR10.
 * The old sampleTask.c uses DR_IO_n_CLR() to select a row and
 * DR_IO_n_SET() to release it.  For the new RS2323 front end the
 * selected row must be driven HIGH, so the two macros are swapped:
 *   DR_IO_n_CLR() -> row ON  (IN high, CRx = Vs+)
 *   DR_IO_n_SET() -> row OFF (IN low,  CRx = GND)
 */
#define DR_IO_1_PIN             GPIO_Pins_4
#define DR_IO_1_GPIO            GPIOA
#define DR_IO_1_GPIO_RCC_CLK    RCC_AHBPERIPH_GPIOA
#define DR_IO_1_SET()           GPIO_WriteBit(DR_IO_1_GPIO, DR_IO_1_PIN, Bit_RESET)
#define DR_IO_1_CLR()           GPIO_WriteBit(DR_IO_1_GPIO, DR_IO_1_PIN, Bit_SET)

#define DR_IO_2_PIN             GPIO_Pins_5
#define DR_IO_2_GPIO            GPIOA
#define DR_IO_2_GPIO_RCC_CLK    RCC_AHBPERIPH_GPIOA
#define DR_IO_2_SET()           GPIO_WriteBit(DR_IO_2_GPIO, DR_IO_2_PIN, Bit_RESET)
#define DR_IO_2_CLR()           GPIO_WriteBit(DR_IO_2_GPIO, DR_IO_2_PIN, Bit_SET)

#define DR_IO_3_PIN             GPIO_Pins_6
#define DR_IO_3_GPIO            GPIOA
#define DR_IO_3_GPIO_RCC_CLK    RCC_AHBPERIPH_GPIOA
#define DR_IO_3_SET()           GPIO_WriteBit(DR_IO_3_GPIO, DR_IO_3_PIN, Bit_RESET)
#define DR_IO_3_CLR()           GPIO_WriteBit(DR_IO_3_GPIO, DR_IO_3_PIN, Bit_SET)

#define DR_IO_4_PIN             GPIO_Pins_7
#define DR_IO_4_GPIO            GPIOA
#define DR_IO_4_GPIO_RCC_CLK    RCC_AHBPERIPH_GPIOA
#define DR_IO_4_SET()           GPIO_WriteBit(DR_IO_4_GPIO, DR_IO_4_PIN, Bit_RESET)
#define DR_IO_4_CLR()           GPIO_WriteBit(DR_IO_4_GPIO, DR_IO_4_PIN, Bit_SET)

#define DR_IO_5_PIN             GPIO_Pins_0
#define DR_IO_5_GPIO            GPIOB
#define DR_IO_5_GPIO_RCC_CLK    RCC_AHBPERIPH_GPIOB
#define DR_IO_5_SET()           GPIO_WriteBit(DR_IO_5_GPIO, DR_IO_5_PIN, Bit_RESET)
#define DR_IO_5_CLR()           GPIO_WriteBit(DR_IO_5_GPIO, DR_IO_5_PIN, Bit_SET)

#define DR_IO_6_PIN             GPIO_Pins_1
#define DR_IO_6_GPIO            GPIOB
#define DR_IO_6_GPIO_RCC_CLK    RCC_AHBPERIPH_GPIOB
#define DR_IO_6_SET()           GPIO_WriteBit(DR_IO_6_GPIO, DR_IO_6_PIN, Bit_RESET)
#define DR_IO_6_CLR()           GPIO_WriteBit(DR_IO_6_GPIO, DR_IO_6_PIN, Bit_SET)

#define DR_IO_7_PIN             GPIO_Pins_2
#define DR_IO_7_GPIO            GPIOB
#define DR_IO_7_GPIO_RCC_CLK    RCC_AHBPERIPH_GPIOB
#define DR_IO_7_SET()           GPIO_WriteBit(DR_IO_7_GPIO, DR_IO_7_PIN, Bit_RESET)
#define DR_IO_7_CLR()           GPIO_WriteBit(DR_IO_7_GPIO, DR_IO_7_PIN, Bit_SET)

#define DR_IO_8_PIN             GPIO_Pins_10
#define DR_IO_8_GPIO            GPIOB
#define DR_IO_8_GPIO_RCC_CLK    RCC_AHBPERIPH_GPIOB
#define DR_IO_8_SET()           GPIO_WriteBit(DR_IO_8_GPIO, DR_IO_8_PIN, Bit_RESET)
#define DR_IO_8_CLR()           GPIO_WriteBit(DR_IO_8_GPIO, DR_IO_8_PIN, Bit_SET)

#define DR_IO_9_PIN             GPIO_Pins_11
#define DR_IO_9_GPIO            GPIOB
#define DR_IO_9_GPIO_RCC_CLK    RCC_AHBPERIPH_GPIOB
#define DR_IO_9_SET()           GPIO_WriteBit(DR_IO_9_GPIO, DR_IO_9_PIN, Bit_RESET)
#define DR_IO_9_CLR()           GPIO_WriteBit(DR_IO_9_GPIO, DR_IO_9_PIN, Bit_SET)

#define DR_IO_10_PIN            GPIO_Pins_12
#define DR_IO_10_GPIO           GPIOB
#define DR_IO_10_GPIO_RCC_CLK   RCC_AHBPERIPH_GPIOB
#define DR_IO_10_SET()          GPIO_WriteBit(DR_IO_10_GPIO, DR_IO_10_PIN, Bit_RESET)
#define DR_IO_10_CLR()          GPIO_WriteBit(DR_IO_10_GPIO, DR_IO_10_PIN, Bit_SET)


/* DAT column switches: IN11..IN22 -> DAT1..DAT12 */
#define DAT1_PIN                GPIO_Pins_13
#define DAT1_GPIO               GPIOB
#define DAT2_PIN                GPIO_Pins_14
#define DAT2_GPIO               GPIOB
#define DAT3_PIN                GPIO_Pins_15
#define DAT3_GPIO               GPIOB
#define DAT4_PIN                GPIO_Pins_8
#define DAT4_GPIO               GPIOA
#define DAT5_PIN                GPIO_Pins_11
#define DAT5_GPIO               GPIOA
#define DAT6_PIN                GPIO_Pins_12
#define DAT6_GPIO               GPIOA
#define DAT7_PIN                GPIO_Pins_6
#define DAT7_GPIO               GPIOF
#define DAT8_PIN                GPIO_Pins_7
#define DAT8_GPIO               GPIOF
#define DAT9_PIN                GPIO_Pins_15
#define DAT9_GPIO               GPIOA
#define DAT10_PIN               GPIO_Pins_3
#define DAT10_GPIO              GPIOB
#define DAT11_PIN               GPIO_Pins_4
#define DAT11_GPIO              GPIOB
#define DAT12_PIN               GPIO_Pins_5
#define DAT12_GPIO              GPIOB


void GpioInit(void);
void startScanIo(uint32_t ulData);

/* New sampling front-end helpers */
void sampleSwitchInit(void);
void sampleDatAllGuard(void);
void sampleDatSelect(uint8_t index);    /* 0..11, all others are guarded */

#endif
