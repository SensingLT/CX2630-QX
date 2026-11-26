#ifndef __MYSTRING__
#define __MYSTRING__

#include <at32f4xx.h>

uint8_t hex16toascii(uint16_t dat,uint8_t *out);
uint8_t hex32toascii(uint32_t dat,uint8_t *out);
uint16_t asciitouint16_t(uint8_t *in,uint8_t len);
uint8_t CompareData(uint8_t *data1,uint8_t *data2,uint16_t length);
void TransFloat2Bytes(float data, uint8_t *bytes);
#endif
