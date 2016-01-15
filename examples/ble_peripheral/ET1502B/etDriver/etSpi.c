#include <string.h>
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "app_error.h"
#include "app_util_platform.h"
//#include "spi_master.h"
//#include "bsp.h"
#include "ET1502B.h"
#include "app_timer.h"
#include "nordic_common.h"
#include <stdint.h>

//===============================
#include "et_define.h"
#include "etSpi.h"
//#include "N_spiflash.h"
#include "et_Spi_gpio.h"
#include "et_debug.h"
//#include "et_inc.h"

//#define APP_TIMER_PRESCALER      0                      /**< Value of the RTC1 PRESCALER register. */
//#define APP_TIMER_MAX_TIMERS     BSP_APP_TIMERS_NUMBER  /**< Maximum number of simultaneously created timers. */
//#define APP_TIMER_OP_QUEUE_SIZE  2                      /**< Size of timer operation queues. */

//#define DELAY_MS                 1000                   /**< Timer Delay in milli-seconds. */

//=========================================================================
//=============================kevin define SPI=============================
//==========================================================================
void etSpim1ReadNorFlashID(void)
{

#if ENABLE_SPI
    uint8_t data_buf[4]={0x00,0x00,0x00};

    FLASH_CS_LOW;
    SpiSent_1Byte(FLASH_JedecDeviceID);
    SpiReadByte(data_buf,3);
	FLASH_CS_HIGH;
	//DbgPrintf("flash ID:0x%x,0x%x,x%x\r\n",data_buf[0],data_buf[1],data_buf[2]);



#endif	



}
void etSpim1CheckNorFlashBusy(void)
{
#if ENABLE_SPI
   uint8_t data_buf[1]={0x00};

   while(1)
   {
   FLASH_CS_LOW;
   SpiSent_1Byte(FLASH_ReadStatusReg1);
   SpiReadByte(data_buf,1);
   FLASH_CS_HIGH;
   //DbgPrintf("flash status:0x%x\r\n",data_buf[0]);
	if(!(data_buf[0] & NORFLASH_BUSY))
	{		  
			break;
	} 		 
   }


#endif
}

/** 
 * @Spim1 check norflash write enable function
 * @param  
 * @return none
 * @note
 * - Spim1 check norflash write enable function
 */
void etSpim1CheckNorFlashWriteEnable(void)
{
#if ENABLE_SPI
    FLASH_CS_LOW;
		SpiSent_1Byte(FLASH_WriteEnable);
		FLASH_CS_HIGH;
		etSpim1CheckNorFlashBusy();
    //SPI_FLASH_WriteEnable();
#endif
}
void etSpim1NorFlashSectorErase(uint32_t start_add)
{
#if ENABLE_SPI
	uint8_t databuf[4];

	etSpim1CheckNorFlashBusy();
	etSpim1CheckNorFlashWriteEnable();	
	FLASH_CS_LOW;
	databuf[0]= FLASH_SectorErase;
	databuf[1]=(uint8_t)((start_add>>16)&0x000000ff);
	databuf[2]=(uint8_t)((start_add>>8)&0x000000ff);
	databuf[3]=(uint8_t)((start_add>>0)&0x000000ff);
	
	
	SpiSentByte(databuf,4);
	FLASH_CS_HIGH;
  etSpim1CheckNorFlashBusy();

#endif

} 
void etSpim1NorFlashBlockErase(unsigned start_add)
{
#if ENABLE_SPI
	//etSpim1CheckNorFlashWriteEnable();

	uint8_t databuf[4];

	etSpim1CheckNorFlashBusy();
	etSpim1CheckNorFlashWriteEnable();

	FLASH_CS_LOW;
	databuf[0]= FLASH_BlockErase;
	databuf[1]=(uint8_t)((start_add>>16)&0x000000ff);
	databuf[2]=(uint8_t)((start_add>>8)&0x000000ff);
	databuf[3]=(uint8_t)((start_add>>0)&0x000000ff);
	SpiSentByte(databuf,4);
	FLASH_CS_HIGH;
  etSpim1CheckNorFlashBusy();

#endif

} 
void etSpim1NorFlashChipErase(void)
{
#if ENABLE_SPI
	uint8_t databuf[4];

	etSpim1CheckNorFlashBusy();
	etSpim1CheckNorFlashWriteEnable();
	FLASH_CS_LOW;
	databuf[0]= chip_erase;
	//databuf[1]=(uint8_t)((start_add>>16)&0x000000ff);
	//databuf[2]=(uint8_t)((start_add>>8)&0x000000ff);
	//databuf[3]=(uint8_t)((start_add>>0)&0x000000ff);
	SpiSentByte(databuf,1);
	FLASH_CS_HIGH;
    etSpim1CheckNorFlashBusy();

#endif

} 

void etSpim1NorFlashPageProgrameStdMode(uint32_t start_add, uint8_t *write_buf, uint32_t write_len)
{
#if ENABLE_SPI
	uint8_t databuf[4];

  etSpim1CheckNorFlashBusy();
	etSpim1CheckNorFlashWriteEnable();

	FLASH_CS_LOW;
	databuf[0]= FLASH_PageProgram;
	databuf[1]=(uint8_t)((start_add>>16)&0x000000ff);
	databuf[2]=(uint8_t)((start_add>>8)&0x000000ff);
	databuf[3]=(uint8_t)((start_add)&0x000000ff);
	SpiSentByte(databuf,4);
	SpiSentByte(write_buf,write_len);
	FLASH_CS_HIGH;

    //etSpim1CheckNorFlashBusy();
    //SPI_FLASH_BufferWrite(write_buf,start_add,write_len & 0xffff);
#endif
}
void etSpim1ReadNorFlashStdMode(uint32_t start_add, uint8_t *data_buf, uint32_t read_len)
{
#if ENABLE_SPI
	uint8_t databuf[4];

  etSpim1CheckNorFlashBusy();
	FLASH_CS_LOW;
  //nrf_delay_10us(1);
	databuf[0]= FLASH_ReadData;
	databuf[1]=(uint8_t)((start_add>>16)&0x000000ff);
	databuf[2]=(uint8_t)((start_add>>8)&0x000000ff);
	databuf[3]=(uint8_t)((start_add>>0)&0x000000ff);
	SpiSentByte(databuf,4);
	SpiReadByte(data_buf,read_len);
	FLASH_CS_HIGH;

#endif
}

#ifndef ENABLE_NEW_BOARD
void etGensor_spi_read(unsigned char reg_addr, unsigned char *buffer, unsigned short len)
{
#if ENABLE_SPI

	G_SPI_CS_LOW;
	SpiSentByte(&reg_addr,1);
	SpiReadByte(buffer,len);
	G_SPI_CS_HIGH;

#endif
}

void etGensor_spi_write(unsigned char reg_addr, unsigned char *buffer, unsigned short len)
{
#if ENABLE_SPI

	G_SPI_CS_LOW;
	SpiSentByte(&reg_addr,1);
	SpiSentByte(buffer,len);
	G_SPI_CS_HIGH;

#endif
}
#endif

/*
//==========================================================================
void etRead_KEYID_from_flash(unsigned char *buffer, unsigned short len)
{
#if ENABLE_SPI

  etSpim1ReadNorFlashStdMode(KEYID_START_ADDR,buffer,len);
#endif
}
*/
void etWrite_KEYID_to_flash(uint32_t reg_addr,unsigned char *buffer, unsigned short len)
{ 
#if ENABLE_SPI
 // uint8_t databuf[4];
   etSpim1CheckNorFlashBusy(); 
   etSpim1NorFlashSectorErase(reg_addr);
    etSpim1NorFlashPageProgrameStdMode(reg_addr,buffer,len); 
 //=====================
 /*	uint8_t databuf[4];  
  FLASH_CS_HIGH;
	GPIO_Spi_MISO_high();	
  etSpim1CheckNorFlashBusy();
  etSpim1NorFlashSectorErase(KEYID_START_ADDR);
	etSpim1CheckNorFlashWriteEnable();

	FLASH_CS_LOW;
	databuf[0]= FLASH_PageProgram;
	databuf[1]=(uint8_t)((KEYID_START_ADDR>>16)&0x000000ff);
	databuf[2]=(uint8_t)((KEYID_START_ADDR>>8)&0x000000ff);
	databuf[3]=(uint8_t)((KEYID_START_ADDR)&0x000000ff);
	SpiSentByte(databuf,4);
  databuf[0]=0xaa;
  databuf[1]=0x55;
  databuf[2]=(len-6) & 0xff;
  SpiSentByte(databuf,3);  
	SpiSentByte(buffer,len);
	FLASH_CS_HIGH;
	GPIO_Spi_MISO_low();*/
#endif
}

//======================================================================
/*
void etSPItest(void)
{
  unsigned char w_data[256];
  unsigned char r_data[256];
  UINT32 i=0,j=0;
  for(i=0;i<4;i++)
  {
   etSpim1CheckNorFlashBusy(); 
   etSpim1NorFlashSectorErase(0x00000000);
    //DbgPrintf("erase OK:\r\n");
    for(j=0;j<256;j++)
   	 w_data[j]=i+1; 
   //etSpim1CheckNorFlashBusy(); 
   etSpim1NorFlashPageProgrameStdMode(0x00000000,w_data,0x100);
   DbgPrintf("write OK:\r\n");
   //etSpim1CheckNorFlashBusy(); 
   for(j=0;j<256;j++)
   	 r_data[j]=0; 
   etSpim1ReadNorFlashStdMode(0x00000000,r_data,0x100);
   DbgPrintf("read data:0x%x\r\n",i);
   for(j=0;j<256;j++)
   	DbgPrintf("0x%x ",(0xff&r_data[j]));
   DbgPrintf("\r\n");
   
  }
  
}
*/
/*
void etSPItest(void)
{
		unsigned char w_data[256];
		unsigned char r_data[256];
		UINT32 i=0,j=0;
		for(i=0;i<5;i++)
		{
		 //etSpim1CheckNorFlashBusy(); 
		 //etSpim1NorFlashSectorErase(0x00000000);
			//DbgPrintf("erase OK:\r\n");
			for(j=0;j<20;j++)
			 w_data[j]=i+1; 
      //w_data[0]=0xaa;
      //w_data[1]=0x55;
      //w_data[2]=0x05;
		 //etSpim1CheckNorFlashBusy(); 
		 //etWrite_KEYID_to_flash(w_data,20);
		 //DbgPrintf("write OK:\r\n");
		 etSpim1CheckNorFlashBusy(); 
		 for(j=0;j<30;j++)
			 r_data[j]=0; 
		 etRead_KEYID_from_flash(r_data,30);
		 DbgPrintf("read data %x:\r\n",i);
		 for(j=0;j<30;j++)
			DbgPrintf("0x%x ",(0xff&r_data[j]));
		 DbgPrintf("\r\n");		 
		}
		
}
*/
void etSpim1NorFlashInit(void)
{
#if ENABLE_SPI
  
    GPIO_Spi_init();

   //SPI_FLASH_Init();
	nrf_delay_ms(1);
	etSpim1CheckNorFlashBusy(); 

	//etSPItest();
	//etSpim1ReadNorFlashID(); 
	//etSpim1CheckNorFlashBusy(); 
	//etSpim1CheckNorFlashWriteEnable(); 
	//etSpim1CheckNorFlashBusy(); 
	//etSPItest();
#endif
}

