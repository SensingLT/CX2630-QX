/**
  **************************************************************************
  * File   : sampleTask.h
  * Version: V1.3.0
  * Date   : 2021-03-18
  * Brief  : Header file for AT-START board
  *          1. 
  */

#ifndef __SAMPLETASK_H
#define __SAMPLETASK_H	 
#include <at32f4xx.h>

#define  H8XL9  1
#define  H4XL10 2
#define  H3XL3  3

#define  SCAN_MODE               H4XL10

#define TOTAL_SAMPLE_POINT ROW*COLUMN
#if   SCAN_MODE == H8XL9
#define ROW 8
#define COLUMN 9
#define LENGTH 144
#elif   SCAN_MODE == H4XL10
#define ROW 4
#define COLUMN 10
#define LENGTH 80
#elif   SCAN_MODE == H3XL3
#define ROW 3
#define COLUMN 3
#define LENGTH 18
#endif

#define  NUMBER_OF_PARTITION     120
#define  NROWBLOCK             39

#define  SAMPLETHRESHOLD       20     /*????5*/
			
#define  MAX_12BITVAL          1023.0f 
#define  MAX_8BITVAL           254.0f 
#define  MAX_8BITCAIL          255 
#define  ADC10BIT 1
#define  ADC12BIT 2
#define  ADCSAMPLE               ADC10BIT
#define  ERROR_POINT          1000     //扎穿异常点位

enum
{
    CH0,
    CH1,
    CH2,
    CH3,
    CH4,
    CH5,
    CH6,
    CH7,
    CH8,
    CH9,
    CH10,
    CH11,
    CH12,
    CH13,
    CH14,
    CH15,
    CH16,
    CH17,
    CH18,
    CH19,
    CHALL	
};
#pragma pack(1)
typedef struct
{
	uint16_t magic; // 固定0x6868
	uint16_t code;	// 设备标号，这里应该可以忽略
	uint8_t cmd;	// 命令码, @msg_cmd_t
	uint16_t length; // 正常使用length即可，上面的结构体忽略

} msg_header_t;

typedef struct {
	msg_header_t	header;
	uint8_t 		sides; //@side_option_e
	uint8_t 		rows;
	uint8_t 		cols; 
	//esp上不直接使用下面的字段，即不直接使用CRC和MAT_ROWS、MAT_COLS，使其支持不同规格的垫子
	uint16_t 		data[ROW * COLUMN];
	uint16_t 		crc; 
} decoded_sample_t;

#pragma pack()


extern uint8_t  g_ucaPressureRaw[ROW][COLUMN];
extern uint8_t  g_ucaPressureZeroRaw[ROW][COLUMN];
extern uint16_t g_ucsPressureZero12Bit[ROW][COLUMN];

void sampleTask(void);
void sndMsg(void);
uint16_t median_filter(uint8_t flag,uint16_t row,uint16_t colum,uint16_t n);
#endif

