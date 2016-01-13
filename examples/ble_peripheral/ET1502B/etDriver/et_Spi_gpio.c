#include "et_Spi_gpio.h"
#include "etSpi.h"
#include "et_define.h"
#include "nrf_delay.h"
#include "ET1502B.h"

#define SPI_NO_INIT 0
#define SPI_INIT 1


static unsigned char SPI_GPIO_init=SPI_NO_INIT;

void SpiSent_1Byte(UINT8 dat)
{
#if ENABLE_SPI
	unsigned char temp1[8]={0x80, 0x40, 0x20, 0x10, 0x08, 0x04,0x02,0x01};
	unsigned char tlen = 0;


		for(tlen=0; tlen<8; tlen++)
		{
			SPI_CLK_LOW;
			if(dat & temp1[tlen])
				SPI_MOSI_HIGH;
			else
				SPI_MOSI_LOW;   
			SPI_CLK_HIGH;
		}
#endif  
}

void SpiSentByte(UINT8 *write,UINT32 len)
{
#if ENABLE_SPI
	unsigned char temp1[8]={0x80, 0x40, 0x20, 0x10, 0x08, 0x04,0x02,0x01},count=0;
	UINT32 tlen = 0;
	
	while(tlen < len)
	{

		for(count=0; count<8; count++)
		{
			SPI_CLK_LOW;
			if(write[tlen] & temp1[count])
				SPI_MOSI_HIGH;
			else
				SPI_MOSI_LOW;   
			SPI_CLK_HIGH;
		}
		/*SPI_CLK_HIGH;*/
		tlen++;   
	}
#endif 
}

void SpiReadByte(UINT8 *read, UINT32 len)
{
#if ENABLE_SPI
	unsigned char temp=0,count=0;
	
	UINT32 tlen = 0;
	while(tlen < len)
	{   
		temp = 0x00;
		for(count=0; count<8; count++)
		{
			temp = temp<<1;
			SPI_CLK_LOW;
			if((SPI_DATA_GET ==1))
				temp |= 0x01;
			SPI_CLK_HIGH;

		}
    read[tlen] = temp & 0xff;
    tlen++;
	}    
#endif
}


void GPIO_Spi_init(void)
{
#if ENABLE_SPI
  if(SPI_GPIO_init==SPI_NO_INIT)
  {
#ifdef ENABLE_NEW_BOARD
  NRF_GPIO->DIRSET = SFLASH_SPIM0_SS_PIN_MASK|SPIM0_SCK_PIN_MASK|SPIM0_MOSI_PIN_MASK;  
#else
	NRF_GPIO->DIRSET = SFLASH_SPIM0_SS_PIN_MASK|SPIM0_SCK_PIN_MASK|SPIM0_MOSI_PIN_MASK|GSENSOR_SPIM0_SS_PIN_MASK;
#endif
	nrf_gpio_cfg_input(SPIM0_MISO_PIN, NRF_GPIO_PIN_PULLUP);
  //G_nrf_gpio_cfg_output(SPIM0_SCK_PIN);
  //G_nrf_gpio_cfg_output(FLASH_SPIM0_SS_PIN);
  //G_nrf_gpio_cfg_output(SPIM0_MOSI_PIN);  
	SPI_CLK_HIGH;
	FLASH_CS_HIGH;
	SPI_MOSI_HIGH;
#ifndef  ENABLE_NEW_BOARD
	G_SPI_CS_HIGH;
#endif
	 SPI_GPIO_init=SPI_INIT;
  }
#endif
}

void GPIO_Spi_MISO_high(void)
{
#if ENABLE_SPI
  if(SPI_GPIO_init==SPI_NO_INIT)
  {
 /*   SPI_CLK_HIGH;
    SPI_MOSI_HIGH;
   // GPIO_Spi_init();
	 nrf_gpio_cfg_input(SPIM0_MISO_PIN, NRF_GPIO_PIN_PULLUP);
   //Gensor_nrf_gpio_cfg_output(SPIM0_MOSI_PIN, NRF_GPIO_PIN_PULLUP);
   //Gensor_nrf_gpio_cfg_output(SPIM0_SCK_PIN, NRF_GPIO_PIN_PULLUP); 
   // nrf_delay_100us(1);
	 SPI_GPIO_init=SPI_INIT;
    */
  }
#endif
}

void GPIO_Spi_MISO_low(void)
{
#if ENABLE_SPI
  if(SPI_GPIO_init==SPI_INIT)
  {
/*	 nrf_gpio_cfg_input(SPIM0_MISO_PIN, NRF_GPIO_PIN_PULLDOWN);
  // nrf_gpio_cfg_input(SPIM0_MOSI_PIN, NRF_GPIO_PIN_PULLDOWN);
  // nrf_gpio_cfg_input(SPIM0_SCK_PIN, NRF_GPIO_PIN_PULLDOWN);
    SPI_MOSI_LOW; 
    SPI_CLK_LOW;
	 SPI_GPIO_init=SPI_NO_INIT;
    */
  }
#endif
}



