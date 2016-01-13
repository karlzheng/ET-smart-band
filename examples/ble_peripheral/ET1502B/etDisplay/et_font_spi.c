#include "et_font_spi.h"
#include "etSpi.h"
#include "et_Spi_gpio.h"
#include "nrf_delay.h"
#include "et_debug.h"
//u8 r_dat_bat(u32 ADDRESS,u8 byte_long,u8 *p_arr)
void r_dat_bat(unsigned long ADDRESS,unsigned int len,unsigned char *DZ_Data)
{
	u8 addr[4] = {0};
  //FLASH_CS_HIGH;
  //nrf_delay_10us(1);
	//addr[0] = (u8)(address>>24);
  FLASH_CS_LOW;
	addr[0] = 0x03;
	addr[1] = (u8)(ADDRESS>>16);
	addr[2] = (u8)(ADDRESS>>8);
	addr[3] = (u8)(ADDRESS>>0);	
  //nrf_delay_10us(1);
	SpiSentByte(addr, 4);
	SpiReadByte(DZ_Data, len);
	FLASH_CS_HIGH;
}
#if 0
static void Write_E(void)  //
{
  u8 temp = 0x06;
	
	FLASH_CS_LOW;
	SpiSentByte(&temp, 1);
	FLASH_CS_HIGH;
}
/*
static void Write_D(void)
{
	u8 temp = 0x04;
	
	FLASH_CS_LOW;
	SpiSentByte(&temp, 1);
	FLASH_CS_HIGH;
}
*/

static u8 SPI_Gtzk_ReadSR(void)   
{  
    u8 byte = 0x05;   
		
    FLASH_CS_LOW;                            //????   
    SpiSentByte(&byte, 1);    //???????????  
		
    //byte = SpiReceByte();             //??????  
		SpiReadByte(&byte, 1);
    FLASH_CS_HIGH;                            //????     
    return byte;   
}


//????
static void SPI_Gtzk_Wait_Busy(void)   
{   
    while ((SPI_Gtzk_ReadSR()&0x01)==0x01);   // ??BUSY???
}  


void sector_erase(unsigned long addr)
{
  u8 Addr[4] = {0x20};
	
  Write_E();
	SPI_Gtzk_Wait_Busy();
	
	FLASH_CS_LOW;
	Addr[1]=addr>>16;
	Addr[2]=addr>>8;
	Addr[3]=addr;
	SpiSentByte(Addr, 4);
  FLASH_CS_HIGH;
	SPI_Gtzk_Wait_Busy();	
}

void Write_Page(unsigned long addr,unsigned char *p,unsigned long len)
{
	u8 Addr[4] = {0x02};
	//unsigned long i;
	
			 
	Write_E();
	FLASH_CS_LOW;
	Addr[1]=addr>>16;
	Addr[2]=addr>>8;
	Addr[3]=addr;
	SpiSentByte(Addr, 4);
	/*for (i=0;i<len;i++)
	{
		SpiSendByte(*(p+i));
	}*/
	SpiSentByte(p, len);
	FLASH_CS_HIGH;
	SPI_Gtzk_Wait_Busy();
}

void Read_Page(unsigned long addr,unsigned char *p,unsigned long len)
{
	u8 Addr[4] = {0x03};
	//unsigned long i;
	
			 
	Write_E();
	FLASH_CS_LOW;
	Addr[1]=addr>>16;
	Addr[2]=addr>>8;
	Addr[3]=addr;
	SpiSentByte(Addr, 4);

	SpiReadByte(p, len);
	FLASH_CS_HIGH;
	SPI_Gtzk_Wait_Busy();
}
#endif


