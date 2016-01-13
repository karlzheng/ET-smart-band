#include "ET1502B.h"
#include "etFontApp.h"
#include "etSpi.h"
//#include "spi_ext.h"

//#include "../etdrv/etspimflash.h"

//#include "../etdrv/etSpi.H"
#if(ENABLE_GT24L16M1Y_UT==0)

unsigned long ascii_addr_calc(unsigned char ch)
{
	if(ch>=0x20 && ch<=0xFF)
		return ((ch-0x20)*16+ASCII_BASE_ADDR);
		//return (((ch-0x20)<<4)+ASCII_BASE_ADDR);
	else
		return 0;
}

//chinese address
unsigned long chinese_addr_calc(unsigned char addr1,unsigned char addr2,unsigned char addr3,unsigned char addr4,unsigned char *len)
{
	unsigned long  h=0;
	unsigned char  c1=0,c2=0,c3=0,c4=0;

	c1=addr1,c2=addr2,c3=addr3,c4=addr4;
	*len = 0;

	if(c2 == 0x7f)
	{
		return (h);
	}

	if(c1 >= 0xA1 && c1 <= 0Xa9 && c2 >= 0xa1) /*Section 1*/
	{
		h = 94*c1 + c2 - 15295;
		*len = 2;
	}
	else if(c1 >= 0xa8 && c1 <= 0xa9 && c2 < 0xa1) /*Section 5*/
	{
		if(c2>0x7f)
			c2--;
		h = 96*c1 + c2 -15346;

		*len = 2;

	}
	if(c1>=0xb0 && c1 <= 0xf7 && c2>=0xa1) /*Section 2*/
	{
		h = 94*c1 + c2 - 15667;

		*len = 2;
	}
	else if(c1<0xa1 && c1>=0x81 && c2>=0x40 ) /*Section 3*/
	{
		if(c2>0x7f)
			c2--;

		h = 190*c1 + c2 -16768 ;

	*len = 2;
	}

	else if(c1>=0xaa && c2<0xa1) /*Section 4*/
	{
		if(c2>0x7f)
			c2--;

		h = 96*c1 + c2 - 2498;

		*len = 2;
	}
	else if(c1==0x81 && c2>=0x39) /*4-bytes secton 1 */
	{
		h =1038 + 21008+(c3-0xEE)*10+c4-0x39;

		*len = 4;
	}
	else if(c1==0x82)/*4-bytes secton 2*/
	{
		h =1038 + 21008+161+(c2-0x30)*1260+(c3-0x81)*10+c4-0x30;

		*len = 4;
	}
	return((h<<5) + GB18030_BASE_ADDR);
}

void read_font_data(unsigned long StartAddr,unsigned char *DataBuff,unsigned short len)
{
		 //SPI_1_ReadDatas(StartAddr,DataBuff,len);
		 //Spim1ReadNorFlashStdMode(StartAddr,DataBuff,len);
		 etSpim1ReadNorFlashStdMode(StartAddr,DataBuff,len);
}
#endif
