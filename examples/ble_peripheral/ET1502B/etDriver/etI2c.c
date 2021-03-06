//#include "IomDrv.h"
#include "etI2c.h"
#include "ET1502B.h"
#include "et_debug.h"
#include "nrf_delay.h"
//#include "products_feature.h"


//#define IIC_SDA_PIN		GP17_CMS1
//#define IIC_SCK_PIN		GP16_CMS0

//#define SCL_MASK  (0x00000001)
//#define SCL_MASK_REV (0xffffffFE)
//#define SDA_MASK  (0x00000002)
//#define SDA_MASK_REV (0xffffffFD)
/*
#define IIC_SDA_HIGH   nrf_gpio_pin_set(LCD_SDA_PIN);
#define IIC_SDA_LOW   nrf_gpio_pin_clear(LCD_SDA_PIN)
#define IIC_SDA__OUTPUT  nrf_gpio_cfg_output(LCD_SDA_PIN);

#define IIC_SCK_LOW   nrf_gpio_pin_clear(LCD_SCL_PIN)
#define IIC_SCK_HIGH   nrf_gpio_pin_set(LCD_SCL_PIN);
#define IIC_SCK_OUTPUT  nrf_gpio_cfg_output(LCD_SCL_PIN);

#define IIC_RES_LOW   nrf_gpio_pin_clear(LCD_RES_PIN)
#define IIC_RES_HIGH   nrf_gpio_pin_set(LCD_RES_PIN);
#define IIC_RES_OUTPUT  nrf_gpio_cfg_output(LCD_RES_PIN);
*/


//const unsigned char Write_code[8]={0x80, 0x40, 0x20, 0x10, 0x08, 0x04,0x02,0x01};
//const unsigned char Write_code_rev[8]={0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb,0xfd,0xfe};


static void IIcDelay_middle(void)
{
	 __nop();
	 __nop();
	 __nop();
	 __nop();
   __nop();
	 __nop();
	 __nop();
	 __nop();
	 //__nop();	
}
static void IIcDelay_long(void)
{
	 __nop();
	 __nop();
	 __nop();
	 __nop();
	 __nop();
	 __nop();
   __nop();
}
static void IIcDelay_short(void)
{
	 __nop();
	 __nop();
	 __nop();
	 __nop();
  __nop();
}
static void IIcDelay_Veryshort(void)
{
	 __nop();
	 __nop();
	 __nop();
   __nop();
}

static void IicStart(void)                 
{
	  //IIC_SDA_OUTPUT;//KEVIN
      IIC_SDA_HIGH;//IicSda=1;
      IIcDelay_middle();
      IIC_SCK_HIGH;//IicScl=1;
      IIcDelay_middle();
      IIC_SDA_LOW;//IicSda=0;
      IIcDelay_middle();
      IIC_SCK_LOW;//IicScl=0;
      IIcDelay_middle();
}

static void IicStop(void)                 
{
	  //IIC_SDA_OUTPUT;//KEVIN
      IIC_SDA_LOW;//IicSda=0;
      IIcDelay_middle();
      IIC_SCK_HIGH;//IicScl=1;
      IIcDelay_middle();
      IIC_SDA_HIGH;//IicSda=1;
      IIcDelay_middle();
}

static void SendAcknowledge(unsigned char ack)               
{
      //IIC_SDA_OUTPUT;//KEVIN
      if(ack)//IicSda=ack;
      {
          IIC_SDA_HIGH;
      }
      else
      {
          IIC_SDA_LOW;
      }
      IIcDelay_middle();
      IIC_SCK_HIGH;//IicScl=1;
      IIcDelay_middle();
      IIC_SCK_LOW;//IicScl=0;
      IIcDelay_long();
	  IIcDelay_long();
} 

static unsigned char IicReceiveByte(void)
{
      unsigned char i;
      unsigned char bytedata=0;

      //IIC_SDA_INPUT;//kevin
      for(i=0;i<8;i++)
      {
          bytedata<<=1;
          IIC_SCK_HIGH;//IicScl=1;
          IIcDelay_middle();
          //IIcDelay_short();
          if(IIC_SDA_DATA_GET)
          {
                  bytedata |=0x01;
                  //bytedata |=Write_code[i];
          }
          else
          {
                  bytedata &=0xfe;
                  //bytedata &=Write_code_rev[i];
          }
          IIC_SCK_LOW;//IicScl=0;
          IIcDelay_Veryshort();
      }
      return bytedata;
}

static void IicSentByte(unsigned char DataByte)               
{
    unsigned char i;
	//IIC_SDA_OUTPUT;//KEVIN
    for(i=0;i<8;i++)
    {
     if(DataByte & 0x80)
     //if(DataByte & Write_code[i])
     {
      IIC_SDA_HIGH;//IicSda=1;
     }
     else
     {
      IIC_SDA_LOW;//IicSda=0;
     }
	 DataByte <<=1;//delay
	 IIcDelay_short();
     IIC_SCK_HIGH;//IicScl=1;
     IIcDelay_middle();
     IIC_SCK_LOW;//IicScl=0;
     //IIcDelay_short();
     //DataByte <<=1;
    }
	  IIcDelay_middle();
    IIC_SDA_HIGH;//IicSda=1;-----Y------D---IicSdaDirIn
    //IIC_SDA_INPUT;//kevin
    //IIcDelay();
    IIC_SCK_HIGH;//IicScl=1;
    IIcDelay_middle();
    IIC_SCK_LOW;//IicScl=0;
    IIcDelay_Veryshort();
    
}

static void GPIO_i2c_read(unsigned char I2C_addr,unsigned char reg_addr, unsigned char *buffer, unsigned short len)
{
    unsigned char i;

    IicStart();
    IicSentByte(I2C_addr);
    IicSentByte(reg_addr);
    IicStart();
    IicSentByte(I2C_addr+0x01);
	  IIcDelay_long();
    //for(i=len-1;i>0;i--)
    for(i=0;i<len;i++)
    {
    	*buffer=IicReceiveByte();
		if(i==(len-1))
		SendAcknowledge(1);
		else
		SendAcknowledge(0);
        buffer++;
        
    }
    //*buffer=IicReceiveByte();
    //SendAcknowledge(1);
    IicStop();
}

static void GPIO_i2c_write(unsigned char I2C_addr,unsigned char reg_addr,const unsigned char *buffer, unsigned short len)
{
	unsigned char i;

    IicStart();
    IicSentByte(I2C_addr);
    IicSentByte(reg_addr);
	/*
    for(i=len;i>0;i--)
    {
        IicSentByte(*buffer);
        buffer++;   
    }
	*/
	for(i=0;i<len;i++)
    {
        IicSentByte(buffer[i]); 
    }
    IicStop();
}
static void GPIO_i2c_write_conValue(unsigned char I2C_addr,unsigned char reg_addr,unsigned char value,unsigned short len)
{
	unsigned char i;

    IicStart();
    IicSentByte(I2C_addr);
    IicSentByte(reg_addr);
	for(i=0;i<len;i++)
    {
        IicSentByte(value); 
    }
    IicStop();
}
static void GPIO_i2c_init(void)
{
  
  NRF_GPIO->DIRSET |= I2C_PIN_MASK;	
	//IIC_SDA_OUTPUT;
	//IIC_SCK_OUTPUT;
	IIC_SCK_HIGH;
	IIC_SDA_HIGH;
  //nrf_gpio_cfg_output(I2C_SCL_PIN);
  //nrf_gpio_cfg_output(I2C_SDA_PIN);
  //nrf_delay_100us(2);
  
}

void GPIO_i2c_lowpower(void)
{
 /*
  //nrf_gpio_cfg_input(I2C_SCL_PIN, NRF_GPIO_PIN_NOPULL);
  //nrf_gpio_cfg_input(I2C_SDA_PIN, NRF_GPIO_PIN_NOPULL);
  Gensor_nrf_gpio_cfg_output(I2C_SCL_PIN, NRF_GPIO_PIN_NOPULL);
  Gensor_nrf_gpio_cfg_output(I2C_SDA_PIN, NRF_GPIO_PIN_NOPULL);
  IIC_SCK_HIGH;
	IIC_SDA_HIGH;
  */
}


//=============================================================================================================================================//
void et_i2c_init(void)
{
  GPIO_i2c_init();
}

void et_i2c_write_conValue(unsigned char I2C_addr,unsigned char reg_addr,unsigned char value,unsigned short len)
{
   //GPIO_i2c_init();
	 GPIO_i2c_write_conValue(I2C_addr,reg_addr,value,len);
   //GPIO_i2c_lowpower();
}

void et_i2c_write(unsigned char I2C_addr,unsigned char reg_addr,const unsigned char *buffer, unsigned short len)
{
   //GPIO_i2c_init();
	 GPIO_i2c_write(I2C_addr,reg_addr,buffer,len);
  //GPIO_i2c_lowpower();
}

void et_i2c_read(unsigned char I2C_addr,unsigned char reg_addr, unsigned char *buffer, unsigned short len)
{
  //GPIO_i2c_init();
	 GPIO_i2c_read(I2C_addr,reg_addr,buffer,len);
  //GPIO_i2c_lowpower();
}
//=============================================================================================
//============================Sensor I2C=======================================================
//=============================================================================================
#ifdef ENABLE_NEW_BOARD

#if I2C_BUS_TYPE == I2C_BUS_TYPE_GPIO
static void sensor_IicStart(void)                 
{
	    G_IIC_SDA_OUTPUT;//KEVIN
      G_IIC_SCK_OUTPUT;
      IIcDelay_long();
	    IIcDelay_long();     
      G_IIC_SDA_HIGH;//IicSda=1;
      //IIcDelay_middle();
      G_IIC_SCK_HIGH;//IicScl=1;
      IIcDelay_middle();
      IIcDelay_middle();
      G_IIC_SDA_LOW;//IicSda=0;
      IIcDelay_middle();
      IIcDelay_middle();
      G_IIC_SCK_LOW;//IicScl=0;
      IIcDelay_middle();
      IIcDelay_middle();
}

static void sensor_IicStop(void)                 
{
	    G_IIC_SDA_OUTPUT;//KEVIN
      G_IIC_SCK_OUTPUT;
      G_IIC_SDA_LOW;//IicSda=0;
      //IIcDelay_middle();
      IIcDelay_long();
      G_IIC_SCK_HIGH;//IicScl=1;
      //IIcDelay_middle();
      IIcDelay_long();
      G_IIC_SDA_HIGH;//IicSda=1;
      //IIcDelay_middle();
      IIcDelay_long();  
}
/*
static void sensor_Iic_lowPW(void)// 
{
  //G_IIC_SDA_INPUT_LOW;
  //G_IIC_SCK_INPUT_LOW;
  //G_IIC_SDA_OUTPUT_LOW;
  //G_IIC_SCK_OUTPUT_LOW;
  //G_IIC_SCK_HIGH;
  //G_IIC_SDA_HIGH;
}
*/
static void sensor_SendAcknowledge(unsigned char ack)               
{
      G_IIC_SDA_OUTPUT;//KEVIN
      if(ack)//IicSda=ack;
      {
        G_IIC_SDA_HIGH;
      }
      else
      {
        G_IIC_SDA_LOW;
      }
      IIcDelay_middle();
      G_IIC_SCK_HIGH;//IicScl=1;
      IIcDelay_middle();
      G_IIC_SCK_LOW;//IicScl=0;
      //IIcDelay_long();
	    //IIcDelay_long();
      IIcDelay_middle();
} 

static unsigned char sensor_IicReceiveByte(void)
{
      unsigned char i;
      unsigned char bytedata=0;

      G_IIC_SDA_INPUT;//kevin
      IIcDelay_Veryshort();
      for(i=0;i<8;i++)
      {
          bytedata<<=1;
          G_IIC_SCK_HIGH;//IicScl=1;
          IIcDelay_middle();
          //IIcDelay_short();
          if(G_IIC_SDA_DATA_GET)
          {
                  bytedata |=0x01;
                  //bytedata |=Write_code[i];
          }
          else
          {
                  bytedata &=0xfe;
                  //bytedata &=Write_code_rev[i];
          }
          G_IIC_SCK_LOW;//IicScl=0;
          //IIcDelay_Veryshort();
          IIcDelay_short();
      }        
      return bytedata;
}

static void sensor_IicSentByte(unsigned char DataByte)               
{
    unsigned char i;       
	  G_IIC_SDA_OUTPUT;//KEVIN
    for(i=0;i<8;i++)
    {
     if(DataByte & 0x80)
     {
      G_IIC_SDA_HIGH;//IicSda=1;
     }
     else
     {
      G_IIC_SDA_LOW;//IicSda=0;
     }
     DataByte <<=1;//delay
     IIcDelay_short();
     G_IIC_SCK_HIGH;//IicScl=1;
     IIcDelay_middle();
     G_IIC_SCK_LOW;//IicScl=0;
     //IIcDelay_short();
     //DataByte <<=1;
    }
	  //IIcDelay_middle();
    IIcDelay_short();
    G_IIC_SDA_HIGH;//IicSda=1;-----Y------D---IicSdaDirIn
    G_IIC_SDA_INPUT;//kevin add 160119
    IIcDelay_short();
    G_IIC_SCK_HIGH;//IicScl=1;
    IIcDelay_middle();
    G_IIC_SCK_LOW;//IicScl=0;
    IIcDelay_Veryshort();
    
}

void sensor_GPIO_i2c_read(unsigned char I2C_addr,unsigned char reg_addr, unsigned char *buffer, unsigned short len)
{
    unsigned char i;
    sensor_IicStop();
    sensor_IicStart();
    //IIcDelay_Veryshort();
    sensor_IicSentByte(I2C_addr);
    sensor_IicSentByte(reg_addr);
    sensor_IicStart();
    //IIcDelay_Veryshort();
    sensor_IicSentByte(I2C_addr+0x01);
    //IIcDelay_Veryshort();
    for(i=0;i<len;i++)
    {
    	*buffer=sensor_IicReceiveByte();
      if(i==(len-1))
      sensor_SendAcknowledge(1);
      else
      sensor_SendAcknowledge(0);
      buffer++;
        
    }
    sensor_IicStop();
}

void sensor_GPIO_i2c_write(unsigned char I2C_addr,unsigned char reg_addr, unsigned char *buffer, unsigned short len)
{
    unsigned char i;
    //sensor_IicStop();
    sensor_IicStart();
    //IIcDelay_Veryshort();
    sensor_IicSentByte(I2C_addr);
    sensor_IicSentByte(reg_addr);    
    for(i=0;i<len;i++)
    {
      sensor_IicSentByte(buffer[i]); 
    }
    sensor_IicStop();
}
void sensor_GPIO_i2c_init(void)
{
  NRF_GPIO->DIRSET |= G_I2C_PIN_MASK;	
  sensor_IicStop();
	//IIC_SDA_OUTPUT;
	//IIC_SCK_OUTPUT;
	//IIC_SCK_HIGH;
	//IIC_SDA_HIGH;
}

#elif I2C_BUS_TYPE == I2C_BUS_TYPE_HARDWARE
#include "twi_master.h"
void sensor_GPIO_i2c_init(void)
{
	twi_master_init();
}

void sensor_GPIO_i2c_read(unsigned char I2C_addr,unsigned char reg_addr, unsigned char *buffer, unsigned short len)
{
    twi_master_transfer(I2C_addr,&reg_addr, 1, false);
	twi_master_transfer(I2C_addr+0x01, buffer, len, true);
#if DEBUG_UART_EN    
  /*  DbgPrintf("sensor_hw_i2c_read,addr:%02x,reg:%02x,len:%02x\r\n",I2C_addr,reg_addr,len);
    for(unsigned char i=0;i<len;i++)
    DbgPrintf("%02x",buffer[i]);
    DbgPrintf("\r\n");
  */
#endif  
}

void sensor_GPIO_i2c_write(unsigned char I2C_addr,unsigned char reg_addr,unsigned char *buffer, unsigned short len)
{
	unsigned char buf[10],i=0;
	buf[0] = reg_addr;
	for(i=0;i<len;i++)
		buf[i+1] = buffer[i];
	twi_master_transfer(I2C_addr,buf, len+1, true);
#if DEBUG_UART_EN    
 /*   DbgPrintf("sensor_hw_i2c_write,addr:%02x,reg:%02x,len=%02x\r\n",I2C_addr,reg_addr,len);
    for(unsigned char i=0;i<len;i++)
    DbgPrintf("%02x",buffer[i]);
    DbgPrintf("\r\n");
  */
#endif 
}

#endif
#endif



