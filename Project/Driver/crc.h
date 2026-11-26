#ifndef __CRC_H
#define __CRC_H
#include <at32f4xx.h>
//CRC
uint32_t calcCrc32( const uint8_t *buf, uint32_t size);
uint32_t calcCrc32Seed(uint32_t seed,uint8_t *buf,uint16_t size);
uint16_t calcCrc16Modbus(unsigned   seed,uint8_t *puchMsg,uint16_t usDataLen);
uint16_t calAcc(uint8_t *ucpData, uint16_t usLen);
#endif
