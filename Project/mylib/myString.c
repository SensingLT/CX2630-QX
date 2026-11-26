
#include "myString.h"


//bcd 转换为ascii
uint8_t bcdtoascii(uint8_t dat,uint8_t *out)
{
    uint8_t temp;
    
    temp=dat/16;
    if(temp<=9)
    {
        out[0] = temp + '0';
    }
    else
    {
        out[0] = temp + 0x37;
    }
    temp=dat%16;
    if(temp<=9)
    {
        out[1] = temp + '0';
    }
    else
    {
        out[1] = temp + 0x37;
    }    
    return 2;
}

//hex32bit 转换为ascii
uint8_t hex32toascii(uint32_t dat,uint8_t *out)
{
    uint32_t i,j,x,y;
    uint8_t flg;
    
    j=0;
    y = 1000000000;
    flg = 0;
    for(i=0;i<10;i++)
    {
        x   = dat / y;
        dat = dat % y;
        y   = y/10;
        if(!x && !flg)
        {
            continue;
        }
        else
        {
            flg = 1;
            *(out+j)=x+'0';
            j++;
        }
    }
    return j;
}

//hex16bit 转换为ascii
uint8_t hex16toascii(uint16_t dat,uint8_t *out)
{
    uint16_t i,j,x,y;
    uint8_t flg;
    
    j=0;
    y = 10000;
    flg = 0;
    for(i=0;i<5;i++)
    {
        x   = dat / y;
        dat = dat % y;
        y   = y/10;
        if(!x && !flg)
        {
            continue;
        }
        else
        {
            flg = 1;
            *(out+j)=x+'0';
            j++;
        }
    }
    return j;
}

//asciitobit8 ascii转为uint8_t
uint16_t asciitouint16_t(uint8_t *in,uint8_t len)
{
    uint16_t temp,i,j;
    
    if(len<1)return 0;
    j=1;
    temp = 0;
    for(i=0;i<len;i++)
    {
        temp = temp + (in[len-i-1]-'0') * j;
        j = j*10;
    }
    return temp;
}


//比较字符串
uint8_t CompareData(uint8_t *data1,uint8_t *data2,uint16_t length)
{
      while(length--)
      {
	if (*data1 > *data2)
        {
	   return 1;
        }   
	else if (*data1 < *data2)
        {
	   return 2;
        }
	data1++;
	data2++;
       }
       return 0;
}


/**
 * float转换为字节传输
 * @param:  data
 * @return: NONE
*/ 
void TransFloat2Bytes(float data, uint8_t *bytes)
{
    uint16_t i;
    
    uint8_t* p = (uint8_t*)&data;  //将double类型的指针强制转换为unsigned char型
    for(i=0; i<4; i++)
    {
        bytes[i] = *p++;  //将相应地址中的数据保存到unsigned char数组中
    }
}
