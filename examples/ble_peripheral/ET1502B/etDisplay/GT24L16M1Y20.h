//unsigned long hfmbdd;
//ASCII±àÂë
#include "ET1502B.h"
#define ASCII_5X7    1  //ASCII×Ö·û5X7µãÕó
#define ASCII_7X8    2  //ASCII×Ö·û7X8µãÕó
#define ASCII_8X16   3  //ASCII×Ö·û6X12µãÕó
#define ASCII_16X16_A    4  //ASCII×Ö·û7X8µãÕó
#define ASCII_16X16_T    5  //ASCII×Ö·û6X12µãÕó

#define SEL_GB                 0

#define PAGE_SIZE 256
#define SECTOR_SIZE 4096

//#define GT24L16M1Y_UT 

//#ifdef GT24L16M1Y_UT
#ifdef ENABLE_GT24L16M1Y_UT
#define PROGRAMMING_ROM_ADDR_BASE 0x000000
#define PROGRAMMING_ROM_ADDR_END  0x13FFFF
#else
#define PROGRAMMING_ROM_ADDR_BASE 0x00000
#define PROGRAMMING_ROM_ADDR_END  0x3FFFF
#endif

unsigned char ASCII_GetData(unsigned char ASCIICode,unsigned char Ascii_Kind,unsigned char *DZ_Data);
unsigned int U2G(unsigned int UN_CODE,unsigned char *DZ_Data);
unsigned int BIG52GBK( unsigned char h,unsigned char l);
void hzbmp24(unsigned char charset, unsigned int code, unsigned int codeex, unsigned char size,unsigned char *buf);
void hzbmp16(unsigned char charset, unsigned int code, unsigned int codeex, unsigned char size,unsigned char *buf);//
extern void r_dat_bat(unsigned long ADDRESS,unsigned int len,unsigned char *DZ_Data);

extern void sector_erase(unsigned long addr);
extern void Write_Page(unsigned long addr,unsigned char *p,unsigned long len);
extern void Read_Page(unsigned long addr,unsigned char *p,unsigned long len);

