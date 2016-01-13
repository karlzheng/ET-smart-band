#ifndef _ET_FONT_APP_H_
#define _ET_FONT_APP_H_
#include"ET1502B.h"

#if(ENABLE_GT24L16M1Y_UT==0)
#define ASCII_BASE_ADDR             0x00000000//0x00300000//FLASH_FANT_START_ADDR
#define GB18030_BASE_ADDR           (0x00001000+ASCII_BASE_ADDR)
#define UNICODE_TO_GBK_BASE_ADDR    (0x000E1000+ASCII_BASE_ADDR)
extern unsigned long chinese_addr_calc(unsigned char addr1, unsigned char addr2, unsigned char addr3, unsigned char addr4, unsigned char *len);
extern unsigned long ascii_addr_calc(unsigned char ch);
extern void read_font_data(unsigned long StartAddr,unsigned char *DataBuff,unsigned short len);
#else
//#define ASCII_BASE_ADDR             0x00000000//0x00300000//FLASH_FANT_START_ADDR
//#define GB18030_BASE_ADDR           (0x00001000+ASCII_BASE_ADDR)
#define UNICODE_TO_GBK_BASE_ADDR    0
#endif

#endif
