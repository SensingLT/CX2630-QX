/**
  **************************************************************************
  * File   : FLASH.h
  * Version: V1.3.0
  * Date   : 2021-03-18
  * Brief  : Header file for AT-START board
  *          1. 
  */

#ifndef __FLASH_H
#define __FLASH_H	 
#include <at32f4xx.h>
#include "stdio.h"

#define FLASH_SIZE 64			//所选AT32F4xx的FLASH容量大小(单位为K)
#define FLASH_WREN 1				//使能FLASH写入(0，不是能;1，使能)

uint16_t FLASH_ReadHalfWord(uint32_t faddr);		  //读出半字  
void FLASH_WriteLenByte(uint32_t WriteAddr,uint32_t DataToWrite,uint16_t Len);	//指定地址开始写入指定长度的数据
u32 FLASH_ReadLenByte(uint32_t ReadAddr,u16 Len);										//指定地址开始读取指定长度数据
void FLASH_Write(uint32_t WriteAddr,uint16_t *pBuffer,uint16_t NumToWrite);		//从指定地址开始写入指定长度的数据
void FLASH_Read(uint32_t ReadAddr,uint16_t *pBuffer,uint16_t NumToRead);   		//从指定地址开始读出指定长度的数据
void FLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);
void FLASH_Write_BYTES(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);
#endif
