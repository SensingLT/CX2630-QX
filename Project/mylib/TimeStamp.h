
#ifndef __HW_TIMESTAMP_H__
#define __HW_TIMESTAMP_H__

#include <at32f4xx.h>

//定义时间宏定义
#define IBM_YEAR_START (1970)
#define IBM_YEAR_END   (2099)
#define LEAPYEAR_CONVERTTO_SECONEDS    (31622400)
#define AVERAGEYEAR_CONVERTTO_SECONEDS (31536000)
#define DAY_CONVERTTO_SECONEDS         (86400)
#define HOUR_CONVERTTO_SECONEDS        (3600)
#define MINUTE_CONVERTTO_SECONEDS      (60)
#define LEAPYEAR_DAYS      (366)
#define AVERAGEYEAR_DAYS   (365)
#define LEAPYEAR_FEBRUARY_DAYS      (29)
#define AVERAGEYEAR_FEBRUARY_DAYS   (28)
//判断平年和闰年宏定义
#define IS_LEAPYEAR(YEAR) (YEAR % 4 ? 0 : YEAR % 100 ? 1 : YEAR % 400 ? 0 : 1 )
//获得编译器时钟
#define GET_COMPILE_YEAR  ((__DATE__[7] - '0')*1000+\
                          (__DATE__[8] - '0')*100+\
                          (__DATE__[9] - '0')*10+\
                          (__DATE__[10] - '0'))
#define GET_COMPILE_DAY  ((__DATE__[4]==0x20?0:(__DATE__[4] - '0')*10) + (__DATE__[5] - '0'))
#define GET_COMPILE_HOUR ((__TIME__[0] - '0')*10 + (__TIME__[1] - '0'))
#define GET_COMPILE_MINUTE ((__TIME__[3] - '0')*10 + (__TIME__[4] - '0'))
#define GET_COMPILE_SECOND ((__TIME__[6] - '0')*10 + (__TIME__[7] - '0'))
//枚举月份
typedef enum 
{  
  JANUARY,
  FEBRUARY,
  MARCH,
  APRIL,
  MAY,
  JUNE,
  JULY,
  AUGUST,
  SEPTEMBER,
  OCTOBER,
  NOVOMBER,
  DECEMBER
}Month_TypeEnum;
//时间结构体
typedef struct 
{
  uint8_t hour;
  uint8_t minute;
  uint8_t second;         
  uint8_t week; 
  uint8_t day;
  uint8_t month; 
  uint16_t year;     
}TimeStamp_FormatTypeDef;
//将时间结构体转换为秒
uint32_t DateTimeToTimeStamp(TimeStamp_FormatTypeDef*);
//将秒转换为时间结构体
uint8_t TimeStampToDateTime(uint32_t ,TimeStamp_FormatTypeDef*);
//获得周数结构体
uint8_t GetWeek(TimeStamp_FormatTypeDef*);
//根据编译器时钟初始化TimeStamp_FormatTypeDef结构体
void AutoInitTimeStamp(TimeStamp_FormatTypeDef *);
#endif