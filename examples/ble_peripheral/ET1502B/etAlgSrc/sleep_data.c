#include "sleep_data.h"
#if SLEEP_RECORD_DATA_ENABLE
#include "sleep_analysis.h"
#include <string.h>
#include "common_arithmetic.h"
#include "et_debug.h"
#include "etSpi.h"
#include "et_WDT.h"

extern unsigned char g_flash_sleeep_state_index;
extern CESleepStateStruct g_flash_pSleepStateItem[SLEEP_STATUS_SIZE];

#define SLEEP_RECORD_FLASH_ADDR		(0x15000)
#define SLEEP_RECORD_HEAD_SIZE		(16)
#define MAX_SLEEP_RECOED_SIZE		(15)

static void flash_read(unsigned int addr,unsigned char *buf,unsigned short len)
{
	etSpim1ReadNorFlashStdMode(addr, buf, len);
}
static void flash_write(unsigned int addr,unsigned char *buf,unsigned short len)
{
	etSpim1NorFlashPageProgrameStdMode(addr, buf, len);
}
static void flash_erase(unsigned int addr, unsigned short n)
{
	while (n--)
    {
        etSpim1NorFlashSectorErase(addr);
      
	#if ENABLE_WDT	 
     	et_WDT_feed();/*WDT Clear*/
	#endif			  
        addr += 4*1024;            //all flash sector erase command erasing size is 4K
    }
}

void sleep_record_flash_init(void)
{
	unsigned char flashBuf[256] = {0};
	unsigned char i=0;

	flash_read(SLEEP_RECORD_FLASH_ADDR,flashBuf,256);

	for(i=0;i<100;i++)
	{
		if(flashBuf[i] != 0xff)
			break;
	}

	if(i>90 || !(flashBuf[0] == 0x1a && flashBuf[1] == 0x2b && flashBuf[2] == 0x3c))//frist init sleep flash ,neet to init
	{
		QPRINTF("frist boot from sleep in flash erase******\r\n");
		memset(flashBuf,0,256);
		flashBuf[0] = 0x1a;
		flashBuf[1] = 0x2b;
		flashBuf[2] = 0x3c;
		flashBuf[3] = 0x0;//length
		flash_erase(SLEEP_RECORD_FLASH_ADDR, 1);
		flash_write(SLEEP_RECORD_FLASH_ADDR,flashBuf,256);

		#if SLEEP_RECORD_DATA_ENABLE
		memset(flashBuf,0,256);
		flash_read(SLEEP_RECORD_FLASH_ADDR,flashBuf,256);
		QPRINTF("2222read 0x%02x,0x%02x,0x%02x,0x%02x\r\n",flashBuf[0],flashBuf[1],flashBuf[2],flashBuf[3]);
		if(flashBuf[0] == 0x1a && flashBuf[1] == 0x2b && flashBuf[2] == 0x3c)
			QPRINTF("sleep data frist write is success\r\n");
		else
			QPRINTF("sleep data frist write is false\r\n");
		#endif
	}

}

void write_sleep_record_to_flash(unsigned char *buf,unsigned char size)
{
	unsigned char flashBuf[256] = {0},reocrdLen = 0,recordIndex=0,i=0;
	
	flash_read(SLEEP_RECORD_FLASH_ADDR,flashBuf,256);

	if(flashBuf[0] == 0x1a && flashBuf[1] == 0x2b && flashBuf[2] == 0x3c)
	{
		reocrdLen = flashBuf[3];
		if(reocrdLen < MAX_SLEEP_RECOED_SIZE)
		{
			recordIndex = reocrdLen*16 + SLEEP_RECORD_HEAD_SIZE;
			memcpy(&flashBuf[recordIndex],buf,size);
			reocrdLen++;
			flashBuf[3] = reocrdLen;
		}
		else
		{
			for(i=0;i<(MAX_SLEEP_RECOED_SIZE-1)*16;i++)
			{
				flashBuf[SLEEP_RECORD_HEAD_SIZE+i] = flashBuf[SLEEP_RECORD_HEAD_SIZE+i + 16];
			}
			
			reocrdLen = MAX_SLEEP_RECOED_SIZE;
			flashBuf[3] = reocrdLen;
			recordIndex = (reocrdLen-1)*16 + SLEEP_RECORD_HEAD_SIZE;
			memcpy(&flashBuf[recordIndex],buf,size);
		}
		
		flash_erase(SLEEP_RECORD_FLASH_ADDR,1);
		flash_write(SLEEP_RECORD_FLASH_ADDR,flashBuf,256);
	}
}

void read_sleep_record_from_flash(unsigned char *buf,unsigned short len)
{
	if(len == 0)
		return;
	flash_read(SLEEP_RECORD_FLASH_ADDR,buf,len);
}

#define SLEEP_STATE_FLASH_ADDR		(0x16000)
void write_sleep_state_to_flash(void)
{
	unsigned char i =0;
	unsigned char flashBuf[(SLEEP_STATUS_SIZE+1)*8]={0};
	QPRINTF("write_sleep_state_to_flash g_flash_sleeep_state_index=%d,flash addr:0x%02x\r\n",g_flash_sleeep_state_index,SLEEP_STATE_FLASH_ADDR);
	flash_erase(SLEEP_STATE_FLASH_ADDR,1);
	
	flashBuf[0] = 0x1a;
	flashBuf[1] = 0x2b;
	flashBuf[2] = 0x3c;
	flashBuf[3] = g_flash_sleeep_state_index;
	for(i=0;i<g_flash_sleeep_state_index;i++)
	{
		flashBuf[(i+1)*8] = g_flash_pSleepStateItem[i].sleepState;
		IntSetFourChar((char*)&flashBuf[(i+1)*8+1], g_flash_pSleepStateItem[i].startSec);
		IntSetThreeChar((char*)&flashBuf[(i+1)*8+5], 0);//set receive 3 char
	}

	flash_write(SLEEP_STATE_FLASH_ADDR,flashBuf,(g_flash_sleeep_state_index+1)*8);
}

/*
	return 0:no sleep data
	return n:stateBuf count (n>0) n*8 < 256
*/
unsigned char get_sleep_state_from_ram(unsigned char *stateBuf,
									unsigned short bufLength,
									unsigned char *transferFg,
									unsigned char curPageIndex,
									unsigned char *totalPage)
{
	unsigned char i =0,j=0,maxPage = 0,retRealLen = 0;
	if(g_flash_sleeep_state_index == 0)
		return 0;

	maxPage  = bufLength / 8;
	*totalPage = g_flash_sleeep_state_index;

	if(g_flash_sleeep_state_index - curPageIndex > maxPage )
	{
		for(i=curPageIndex,j=0;i<maxPage;i++,j++)
		{
			stateBuf[j*8] = g_flash_pSleepStateItem[i].sleepState;
			IntSetFourChar((char*)&stateBuf[j*8+1], g_flash_pSleepStateItem[i].startSec);
			IntSetThreeChar((char*)&stateBuf[j*8+5], 0);//set receive 3 char
		}
		retRealLen = maxPage;
		*transferFg = 0;
	}
	else
	{
		for(i=curPageIndex,j=0;i<g_flash_sleeep_state_index;i++,j++)
		{
			stateBuf[j*8] = g_flash_pSleepStateItem[i].sleepState;
			IntSetFourChar((char*)&stateBuf[j*8+1], g_flash_pSleepStateItem[i].startSec);
			IntSetThreeChar((char*)&stateBuf[j*8+5], 0);//set receive 3 char
		}
		retRealLen = j;
		*transferFg = 1;
	}

	return retRealLen;
}

#endif

