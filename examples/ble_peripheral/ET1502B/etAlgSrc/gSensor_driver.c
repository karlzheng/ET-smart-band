#include "ET1502B.h"
#include "gSensor.h"
#include "gSensor_driver.h"
static unsigned char sensor_INT_enable=1;
static unsigned char sensor_INT_check=0;
#if  GSENSOR_ACCESS_TYPE	== GSENSOR_ACCESS_BY_SPI
#include "etSpi.h"
#endif
#if GSENSOR_ACCESS_TYPE	== GSENSOR_ACCESS_BY_I2C
#define GSENSOR_ADDR	(0x32)

#include "etI2c.h"
//static void lis3dh_i2c_init(void)
//{
//	et_i2c_init();//GPIO_i2c_init();
//}

static void lis3dh_i2c_read(unsigned char reg_addr, unsigned char *buffer, unsigned short len)
{
  sensor_GPIO_i2c_read(GSENSOR_ADDR,reg_addr, buffer,len);
}

static void lis3dh_i2c_write(unsigned char reg_addr, unsigned char *buffer, unsigned short len)
{
	sensor_GPIO_i2c_write(GSENSOR_ADDR,reg_addr,buffer,len);
}

#elif  GSENSOR_ACCESS_TYPE	== GSENSOR_ACCESS_BY_SPI

//#define Spim1SetCsLow()			(SPIM1->EN &= (~(SPIM1EN_SSMOD|SPIM1EN_FRAME)))
//#define Spim1SetCsHigh()		(SPIM1->EN |= (SPIM1EN_FRAME))

//static void lis3dh_spi_init(void)
//{
//	Spim1Init(0, HSIZE_BYTE, DSSP_8BIT);
//	Spim1SetBaudrate(2, 2);/*…Ë÷√≤®Ãÿ¬ */
//}

static void lis3dh_spi_read(unsigned char reg_addr, unsigned char *buffer, unsigned short len)
{ 
 	//Spim1SetCsLow();
	//reg_addr=reg_addr|0x80;
	//Spim1SendInsToNor(0, reg_addr, buffer, len);
	//Spim1SetCsHigh();
	etGensor_spi_read(reg_addr|0xc0,buffer,len);
}

static void lis3dh_spi_write(unsigned char reg_addr, unsigned char *buffer, unsigned short len)
{
 	//Spim1SetCsLow();
	//reg_addr=reg_addr&0x7f;
	//Spim1SendData(0, &reg_addr, 1);
	//Spim1SendData(0, buffer, len);
	//Spim1SetCsHigh();
	etGensor_spi_write((reg_addr&0x7f),buffer, len);
}

#endif

void lis3dh_init(void)
{
#if GSENSOR_ACCESS_TYPE	== GSENSOR_ACCESS_BY_I2C
	//lis3dh_i2c_init();
#elif  GSENSOR_ACCESS_TYPE	== GSENSOR_ACCESS_BY_SPI
	//lis3dh_spi_init();
#endif
}

static u8_t LIS3DH_ReadReg(u8_t Reg, u8_t* Data) 
{
#if GSENSOR_ACCESS_TYPE	== GSENSOR_ACCESS_BY_I2C
	lis3dh_i2c_read(Reg,Data,1);
#elif  GSENSOR_ACCESS_TYPE	== GSENSOR_ACCESS_BY_SPI
  	lis3dh_spi_read(Reg,Data,1);
#endif
  return 1;
}

static u8_t LIS3DH_WriteReg(u8_t WriteAddr, u8_t Data) 
{
#if GSENSOR_ACCESS_TYPE	== GSENSOR_ACCESS_BY_I2C
	unsigned char i=0,test_value=0;

	while(i++ <10)
	{
		test_value = 0;
		lis3dh_i2c_write(WriteAddr,&Data,1);
		lis3dh_i2c_read(WriteAddr,&test_value,1);

		if(test_value == Data)
			break;
		else
		{
			lis3dh_i2c_write(WriteAddr,&Data,1);
		}
	}	
#elif  GSENSOR_ACCESS_TYPE	== GSENSOR_ACCESS_BY_SPI
 	lis3dh_spi_write(WriteAddr,&Data,1);
#endif
  return 1;
}

status_t LIS3DH_GetWHO_AM_I(u8_t* val)
{
  
  if( !LIS3DH_ReadReg(LIS3DH_WHO_AM_I, val) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}



/*******************************************************************************
* Function Name  : LIS3DH_SetODR
* Description    : Sets LIS3DH Output Data Rate
* Input          : Output Data Rate
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetODR(LIS3DH_ODR_t ov){
  u8_t value = 0;
  
  if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG1, &value) )
    return MEMS_ERROR;
  
  value &= 0x0f;
  value |= ov<<LIS3DH_ODR_BIT;
  
  if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG1, value) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS3DH_SetMode
* Description    : Sets LIS3DH Operating Mode
* Input          : Modality (LIS3DH_NORMAL, LIS3DH_LOW_POWER, LIS3DH_POWER_DOWN)
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetMode(LIS3DH_Mode_t md) {
  u8_t value = 0;
  u8_t value2 = 0;
  static   u8_t ODR_old_value;
  
  if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG1, &value) )
    return MEMS_ERROR;
  
  if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG4, &value2) )
    return MEMS_ERROR;
  
  if((value & 0xF0)==0) 
    value = value | (ODR_old_value & 0xF0); //if it comes from POWERDOWN  
  
  switch(md) {
    
  case LIS3DH_POWER_DOWN:
    ODR_old_value = value;
    value &= 0x0F;
    break;
    
  case LIS3DH_NORMAL:
    value &= 0xF7;
    value |= (MEMS_RESET<<LIS3DH_LPEN);
    value2 &= 0xF7;
    value2 |= (MEMS_SET<<LIS3DH_HR);   //set HighResolution_BIT
    break;
    
  case LIS3DH_LOW_POWER:		
    value &= 0xF7;
    value |=  (MEMS_SET<<LIS3DH_LPEN);
    value2 &= 0xF7;
    value2 |= (MEMS_RESET<<LIS3DH_HR); //reset HighResolution_BIT
    break;
    
  default:
    return MEMS_ERROR;
  }
  
  if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG1, value) )
    return MEMS_ERROR;
  
  if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG4, value2) )
    return MEMS_ERROR;  
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS3DH_SetAxis
* Description    : Enable/Disable LIS3DH Axis
* Input          : LIS3DH_X_ENABLE/DISABLE | LIS3DH_Y_ENABLE/DISABLE | LIS3DH_Z_ENABLE/DISABLE
* Output         : None
* Note           : You MUST use all input variable in the argument, as example
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetAxis(LIS3DH_Axis_t axis) {
  u8_t value;
  
  if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG1, &value) )
    return MEMS_ERROR;
  value &= 0xF8;
  value |= (0x07 & axis);
  
  if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG1, value) )
    return MEMS_ERROR;   
  
  return MEMS_SUCCESS;
}


/*******************************************************************************
* Function Name  : LIS3DH_SetFullScale
* Description    : Sets the LIS3DH FullScale
* Input          : LIS3DH_FULLSCALE_2/LIS3DH_FULLSCALE_4/LIS3DH_FULLSCALE_8/LIS3DH_FULLSCALE_16
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetFullScale(LIS3DH_Fullscale_t fs) {
  u8_t value = 0;
  
  if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG4, &value) )
    return MEMS_ERROR;
  
  value &= 0xCF;	
  value |= (fs<<LIS3DH_FS);
  
  if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG4, value) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS3DH_SetBDU
* Description    : Enable/Disable Block Data Update Functionality
* Input          : ENABLE/DISABLE
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetBDU(State_t bdu)
{
    u8_t value;

    if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG4, &value) )
        return MEMS_ERROR;

    value &= 0x7F;
    value |= (bdu<<LIS3DH_BDU);

    if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG4, value) )
        return MEMS_ERROR;

    return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS3DH_SetSelfTest
* Description    : Set Self Test Modality
* Input          : LIS3DH_SELF_TEST_DISABLE/ST_0/ST_1
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetSelfTest(LIS3DH_SelfTest_t st) {
  u8_t value = 0;
  
  if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG4, &value) )
    return MEMS_ERROR;
  
  value &= 0xF9;
  value |= (st<<LIS3DH_ST);
  
  if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG4, value) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS3DH_HPFAOI1
* Description    : Enable/Disable High Pass Filter for AOI on INT_1
* Input          : MEMS_ENABLE/MEMS_DISABLE
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_HPFAOI1Enable(State_t hpfe) {
  u8_t value = 0;
  
  if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG2, &value) )
    return MEMS_ERROR;
  
  value &= 0xFE;
  value |= (hpfe<<LIS3DH_HPIS1);
  
  if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG2, value) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}


/*******************************************************************************
* Function Name  : LIS3DH_HPFAOI2
* Description    : Enable/Disable High Pass Filter for AOI on INT_2
* Input          : MEMS_ENABLE/MEMS_DISABLE
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_HPFAOI2Enable(State_t hpfe) {
  u8_t value = 0;
  
  if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG2, &value) )
    return MEMS_ERROR;
  
  value &= 0xFD;
  value |= (hpfe<<LIS3DH_HPIS2);
  
  if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG2, value) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS3DH_SetInt1Pin
* Description    : Set Interrupt1 pin Function
* Input          :  LIS3DH_CLICK_ON_PIN_INT1_ENABLE/DISABLE    | LIS3DH_I1_INT1_ON_PIN_INT1_ENABLE/DISABLE |              
                    LIS3DH_I1_INT2_ON_PIN_INT1_ENABLE/DISABLE  | LIS3DH_I1_DRDY1_ON_INT1_ENABLE/DISABLE    |              
                    LIS3DH_I1_DRDY2_ON_INT1_ENABLE/DISABLE     | LIS3DH_WTM_ON_INT1_ENABLE/DISABLE         |           
                    LIS3DH_INT1_OVERRUN_ENABLE/DISABLE  
* example        : SetInt1Pin(LIS3DH_CLICK_ON_PIN_INT1_ENABLE | LIS3DH_I1_INT1_ON_PIN_INT1_ENABLE |              
                    LIS3DH_I1_INT2_ON_PIN_INT1_DISABLE | LIS3DH_I1_DRDY1_ON_INT1_ENABLE | LIS3DH_I1_DRDY2_ON_INT1_ENABLE |
                    LIS3DH_WTM_ON_INT1_DISABLE | LIS3DH_INT1_OVERRUN_DISABLE   ) 
* Note           : To enable Interrupt signals on INT1 Pad (You MUST use all input variable in the argument, as example)
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetInt1Pin(LIS3DH_IntPinConf_t pinConf,unsigned char flg) {
  u8_t value = 0;
  if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG3, &value) )
    return MEMS_ERROR;

  if(flg)
  {
  	if(value & pinConf)
		return MEMS_SUCCESS;
  	value |= pinConf;
  }
  else
  	value &= ~(pinConf);
  
  if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG3, value) )
    return MEMS_ERROR;

  return MEMS_SUCCESS;
}


/*******************************************************************************
* Function Name  : LIS3DH_SetInt2Pin
* Description    : Set Interrupt2 pin Function
* Input          : LIS3DH_CLICK_ON_PIN_INT2_ENABLE/DISABLE   | LIS3DH_I2_INT1_ON_PIN_INT2_ENABLE/DISABLE |               
                   LIS3DH_I2_INT2_ON_PIN_INT2_ENABLE/DISABLE | LIS3DH_I2_BOOT_ON_INT2_ENABLE/DISABLE |                   
                   LIS3DH_INT_ACTIVE_HIGH/LOW
* example        : LIS3DH_SetInt2Pin(LIS3DH_CLICK_ON_PIN_INT2_ENABLE/DISABLE | LIS3DH_I2_INT1_ON_PIN_INT2_ENABLE/DISABLE |               
                   LIS3DH_I2_INT2_ON_PIN_INT2_ENABLE/DISABLE | LIS3DH_I2_BOOT_ON_INT2_ENABLE/DISABLE |                   
                   LIS3DH_INT_ACTIVE_HIGH/LOW)
* Note           : To enable Interrupt signals on INT2 Pad (You MUST use all input variable in the argument, as example)
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetInt2Pin(LIS3DH_IntPinConf_t pinConf) {
  u8_t value = 0;
  
  if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG6, &value) )
    return MEMS_ERROR;
  
  value &= 0x00;
  value |= pinConf;
  
  if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG6, value) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}                       

#if TAP_DOUBLE_CLCIK_ENABLE
/*******************************************************************************
* Function Name  : LIS3DH_SetClickCFG
* Description    : Set Click Interrupt config Function
* Input          : LIS3DH_ZD_ENABLE/DISABLE | LIS3DH_ZS_ENABLE/DISABLE  | LIS3DH_YD_ENABLE/DISABLE  | 
                   LIS3DH_YS_ENABLE/DISABLE | LIS3DH_XD_ENABLE/DISABLE  | LIS3DH_XS_ENABLE/DISABLE 
* example        : LIS3DH_SetClickCFG( LIS3DH_ZD_ENABLE | LIS3DH_ZS_DISABLE | LIS3DH_YD_ENABLE | 
                               LIS3DH_YS_DISABLE | LIS3DH_XD_ENABLE | LIS3DH_XS_ENABLE)
* Note           : You MUST use all input variable in the argument, as example
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetClickCFG(u8_t status) {
  u8_t value;
  
  if( !LIS3DH_ReadReg(LIS3DH_CLICK_CFG, &value) )
    return MEMS_ERROR;
  
  value &= 0xC0;
  value |= status;
  
  if( !LIS3DH_WriteReg(LIS3DH_CLICK_CFG, value) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}  


/*******************************************************************************
* Function Name  : LIS3DH_SetClickTHS
* Description    : Set Click Interrupt threshold
* Input          : Click-click Threshold value [0-127]
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetClickTHS(u8_t ths) {
  
  if(ths>127)     
    return MEMS_ERROR;
  
  if( !LIS3DH_WriteReg(LIS3DH_CLICK_THS, ths) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
} 


/*******************************************************************************
* Function Name  : LIS3DH_SetClickLIMIT
* Description    : Set Click Interrupt Time Limit
* Input          : Click-click Time Limit value [0-127]
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetClickLIMIT(u8_t t_limit) {
  
  if(t_limit>127)     
    return MEMS_ERROR;
  
  if( !LIS3DH_WriteReg(LIS3DH_TIME_LIMIT, t_limit) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
} 


/*******************************************************************************
* Function Name  : LIS3DH_SetClickLATENCY
* Description    : Set Click Interrupt Time Latency
* Input          : Click-click Time Latency value [0-255]
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetClickLATENCY(u8_t t_latency) {
  
  if( !LIS3DH_WriteReg(LIS3DH_TIME_LATENCY, t_latency) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
} 


/*******************************************************************************
* Function Name  : LIS3DH_SetClickWINDOW
* Description    : Set Click Interrupt Time Window
* Input          : Click-click Time Window value [0-255]
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetClickWINDOW(u8_t t_window) {
  
  if( !LIS3DH_WriteReg(LIS3DH_TIME_WINDOW, t_window) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}


/*******************************************************************************
* Function Name  : LIS3DH_GetClickResponse
* Description    : Get Click Interrupt Response by CLICK_SRC REGISTER
* Input          : char to empty by Click Response Typedef
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_GetClickResponse(u8_t* res) {
  u8_t value;
  
  if( !LIS3DH_ReadReg(LIS3DH_CLICK_SRC, &value) ) 
    return MEMS_ERROR;
  
  value &= 0x7F;
  
  if((value & LIS3DH_IA)==0) {        
    *res = LIS3DH_NO_CLICK;     
    return MEMS_SUCCESS;
  }
  else {
    if (value & LIS3DH_DCLICK){
      if (value & LIS3DH_CLICK_SIGN){
        if (value & LIS3DH_CLICK_Z) {
          *res = LIS3DH_DCLICK_Z_N;   
          return MEMS_SUCCESS;
        }
        if (value & LIS3DH_CLICK_Y) {
          *res = LIS3DH_DCLICK_Y_N;   
          return MEMS_SUCCESS;
        }
        if (value & LIS3DH_CLICK_X) {
          *res = LIS3DH_DCLICK_X_N;   
          return MEMS_SUCCESS;
        }
      }
      else{
        if (value & LIS3DH_CLICK_Z) {
          *res = LIS3DH_DCLICK_Z_P;   
          return MEMS_SUCCESS;
        }
        if (value & LIS3DH_CLICK_Y) {
          *res = LIS3DH_DCLICK_Y_P;   
          return MEMS_SUCCESS;
        }
        if (value & LIS3DH_CLICK_X) {
          *res = LIS3DH_DCLICK_X_P;   
          return MEMS_SUCCESS;
        }
      }       
    }
    else{
      if (value & LIS3DH_CLICK_SIGN){
        if (value & LIS3DH_CLICK_Z) {
          *res = LIS3DH_SCLICK_Z_N;   
          return MEMS_SUCCESS;
        }
        if (value & LIS3DH_CLICK_Y) {
          *res = LIS3DH_SCLICK_Y_N;   
          return MEMS_SUCCESS;
        }
        if (value & LIS3DH_CLICK_X) {
          *res = LIS3DH_SCLICK_X_N;   
          return MEMS_SUCCESS;
        }
      }
      else{
        if (value & LIS3DH_CLICK_Z) {
          *res = LIS3DH_SCLICK_Z_P;   
          return MEMS_SUCCESS;
        }
        if (value & LIS3DH_CLICK_Y) {
          *res = LIS3DH_SCLICK_Y_P;   
          return MEMS_SUCCESS;
        }
        if (value & LIS3DH_CLICK_X) {
          *res = LIS3DH_SCLICK_X_P;   
          return MEMS_SUCCESS;
        }
      }
    }
  }
  return MEMS_ERROR;
} 
#endif

/*******************************************************************************
* Function Name  : LIS3DH_SetIntConfiguration
* Description    : Interrupt 1 Configuration (without LIS3DH_6D_INT)
* Input          : LIS3DH_INT1_AND/OR | LIS3DH_INT1_ZHIE_ENABLE/DISABLE | LIS3DH_INT1_ZLIE_ENABLE/DISABLE...
* Output         : None
* Note           : You MUST use all input variable in the argument, as example
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetIntConfiguration(LIS3DH_Int1Conf_t ic) {
  u8_t value = 0;
  
  if( !LIS3DH_ReadReg(LIS3DH_INT1_CFG, &value) )
    return MEMS_ERROR;
  
  value &= 0x40; 
  value |= ic;
  
  if( !LIS3DH_WriteReg(LIS3DH_INT1_CFG, value) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
} 

     
/*******************************************************************************
* Function Name  : LIS3DH_SetIntMode
* Description    : Interrupt 1 Configuration mode (OR, 6D Movement, AND, 6D Position)
* Input          : LIS3DH_INT_MODE_OR, LIS3DH_INT_MODE_6D_MOVEMENT, LIS3DH_INT_MODE_AND, 
				   LIS3DH_INT_MODE_6D_POSITION
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetIntMode(LIS3DH_Int1Mode_t int_mode) {
  u8_t value = 0;
  
  if( !LIS3DH_ReadReg(LIS3DH_INT1_CFG, &value) )
    return MEMS_ERROR;
  
  value &= 0x3F; 
  value |= (int_mode<<LIS3DH_INT_6D);
  
  if( !LIS3DH_WriteReg(LIS3DH_INT1_CFG, value) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}


/*******************************************************************************
* Function Name  : LIS3DH_SetInt1Threshold
* Description    : Sets Interrupt 1 Threshold
* Input          : Threshold = [0,31]
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetInt1Threshold(u8_t ths) {
  if (ths > 127)
    return MEMS_ERROR;
  
  if( !LIS3DH_WriteReg(LIS3DH_INT1_THS, ths) )
    return MEMS_ERROR;    
  
  return MEMS_SUCCESS;
}


/*******************************************************************************
* Function Name  : LIS3DH_SetInt1Duration
* Description    : Sets Interrupt 1 Duration
* Input          : Duration value
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetInt1Duration(LIS3DH_Int1Conf_t id) {
  
  if (id > 127)
    return MEMS_ERROR;
  
  if( !LIS3DH_WriteReg(LIS3DH_INT1_DURATION, id) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}


/*******************************************************************************
* Function Name  : LIS3DH_FIFOModeEnable
* Description    : Sets Fifo Modality
* Input          : LIS3DH_FIFO_DISABLE, LIS3DH_FIFO_BYPASS_MODE, LIS3DH_FIFO_MODE, 
				   LIS3DH_FIFO_STREAM_MODE, LIS3DH_FIFO_TRIGGER_MODE
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_FIFOModeEnable(LIS3DH_FifoMode_t fm) 
{
	u8_t value;

	if(fm == LIS3DH_FIFO_DISABLE) {
		if( !LIS3DH_ReadReg(LIS3DH_FIFO_CTRL_REG, &value) )
			return MEMS_ERROR;

		value &= 0x1F;
		value |= (LIS3DH_FIFO_BYPASS_MODE<<LIS3DH_FM);

		if( !LIS3DH_WriteReg(LIS3DH_FIFO_CTRL_REG, value) ) 		  //fifo mode bypass
			return MEMS_ERROR;
		if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG5, &value) )
			return MEMS_ERROR;

		value &= 0xBF;

		if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG5, value) ) 			  //fifo disable
			return MEMS_ERROR;
	}

	if(fm == LIS3DH_FIFO_BYPASS_MODE)	{
		if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG5, &value) )
			return MEMS_ERROR;

		value &= 0xBF;
		value |= MEMS_SET<<LIS3DH_FIFO_EN;

		if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG5, value) ) 			  //fifo enable
			return MEMS_ERROR;
		if( !LIS3DH_ReadReg(LIS3DH_FIFO_CTRL_REG, &value) )
			return MEMS_ERROR;

		value &= 0x1f;
		value |= (fm<<LIS3DH_FM);					  //fifo mode configuration

		if( !LIS3DH_WriteReg(LIS3DH_FIFO_CTRL_REG, value) )
			return MEMS_ERROR;
	}

	if(fm == LIS3DH_FIFO_MODE)	 {
		if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG5, &value) )
			return MEMS_ERROR;

		value &= 0xBF;
		value |= MEMS_SET<<LIS3DH_FIFO_EN;

		if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG5, value) ) 			  //fifo enable
			return MEMS_ERROR;
		if( !LIS3DH_ReadReg(LIS3DH_FIFO_CTRL_REG, &value) )
			return MEMS_ERROR;

		value &= 0x1f;
		value |= (fm<<LIS3DH_FM);					   //fifo mode configuration

		if( !LIS3DH_WriteReg(LIS3DH_FIFO_CTRL_REG, value) )
			return MEMS_ERROR;
	}

	if(fm == LIS3DH_FIFO_STREAM_MODE)	{
		if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG5, &value) )
			return MEMS_ERROR;

		value &= 0xBF;
		value |= MEMS_SET<<LIS3DH_FIFO_EN;

		if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG5, value) ) 			  //fifo enable
			return MEMS_ERROR;
		if( !LIS3DH_ReadReg(LIS3DH_FIFO_CTRL_REG, &value) )
			return MEMS_ERROR;

		value &= 0x1f;
		value |= (fm<<LIS3DH_FM);					   //fifo mode configuration

		if( !LIS3DH_WriteReg(LIS3DH_FIFO_CTRL_REG, value) )
			return MEMS_ERROR;
	}

	if(fm == LIS3DH_FIFO_TRIGGER_MODE)	 {
		if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG5, &value) )
			return MEMS_ERROR;

		value &= 0xBF;
		value |= MEMS_SET<<LIS3DH_FIFO_EN;

		if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG5, value) ) 			  //fifo enable
			return MEMS_ERROR;
		if( !LIS3DH_ReadReg(LIS3DH_FIFO_CTRL_REG, &value) )
			return MEMS_ERROR;

		value &= 0x1f;
		value |= (fm<<LIS3DH_FM);					   //fifo mode configuration

		if( !LIS3DH_WriteReg(LIS3DH_FIFO_CTRL_REG, value) )
			return MEMS_ERROR;
	}

	return MEMS_SUCCESS;
}



/*******************************************************************************
* Function Name  : LIS3DH_SetTriggerInt
* Description    : Trigger event liked to trigger signal INT1/INT2
* Input          : LIS3DH_TRIG_INT1/LIS3DH_TRIG_INT2
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetTriggerInt(LIS3DH_TrigInt_t tr) {
  u8_t value = 0;  
  
  if( !LIS3DH_ReadReg(LIS3DH_FIFO_CTRL_REG, &value) )
    return MEMS_ERROR;
  
  value &= 0xDF;
  value |= (tr<<LIS3DH_TR); 
  
  if( !LIS3DH_WriteReg(LIS3DH_FIFO_CTRL_REG, value) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}


/*******************************************************************************
* Function Name  : LIS3DH_SetWaterMark
* Description    : Sets Watermark Value
* Input          : Watermark = [0,31]
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_SetWaterMark(u8_t wtm) {
  u8_t value = 0;
  
  if(wtm > 31)
    return MEMS_ERROR;  
  
  if( !LIS3DH_ReadReg(LIS3DH_FIFO_CTRL_REG, &value) )
    return MEMS_ERROR;
  
  value &= 0xE0;
  value |= wtm; 
  
  if( !LIS3DH_WriteReg(LIS3DH_FIFO_CTRL_REG, value) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS3DH_GetInt1Src
* Description    : Reset Interrupt 1 Latching function
* Input          : Char to empty by Int1 source value
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_GetInt1Src(u8_t* val) {
  
  if( !LIS3DH_ReadReg(LIS3DH_INT1_SRC, val) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS3DH_GetFifoSourceFSS
* Description    : Read current number of unread samples stored in FIFO
* Input          : Byte to empty by FIFO unread sample value
* Output         : None
* Return         : Status [value of FSS]
*******************************************************************************/
status_t LIS3DH_GetFifoSourceFSS(u8_t* val)
{
    u8_t value;

    if( !LIS3DH_ReadReg(LIS3DH_FIFO_SRC_REG, &value) )
        return MEMS_ERROR;

    value &= 0x1F;

    *val = value;

    return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS3DH_GetFifoSourceBit
* Description    : Read Fifo WaterMark source bit
* Input          : statusBIT: LIS3DH_FIFO_SRC_WTM, LIS3DH_FIFO_SRC_OVRUN, LIS3DH_FIFO_SRC_EMPTY
*       val: Byte to fill  with the bit value
* Output         : None
* Return         : Status of BIT [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_GetFifoSourceBit(u8_t statusBIT,  u8_t* val)
{
    u8_t value;

    if( !LIS3DH_ReadReg(LIS3DH_FIFO_SRC_REG, &value) )
        return MEMS_ERROR;


    if(statusBIT == LIS3DH_FIFO_SRC_WTM) {
        if(value &= LIS3DH_FIFO_SRC_WTM) {
            *val = MEMS_SET;
            return MEMS_SUCCESS;
        } else {
            *val = MEMS_RESET;
            return MEMS_SUCCESS;
        }
    }

    if(statusBIT == LIS3DH_FIFO_SRC_OVRUN) {
        if(value &= LIS3DH_FIFO_SRC_OVRUN) {
            *val = MEMS_SET;
            return MEMS_SUCCESS;
        } else {
            *val = MEMS_RESET;
            return MEMS_SUCCESS;
        }
    }
    if(statusBIT == LIS3DH_FIFO_SRC_EMPTY) {
        if(value &= statusBIT == LIS3DH_FIFO_SRC_EMPTY) {
            *val = MEMS_SET;
            return MEMS_SUCCESS;
        } else {
            *val = MEMS_RESET;
            return MEMS_SUCCESS;
        }
    }
    return MEMS_ERROR;
}

/*******************************************************************************
* Function Name  : LIS3DH_GetFifoSourceReg
* Description    : Read Fifo source Register
* Input          : Byte to empty by FIFO source register value
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_GetFifoSourceReg(u8_t* val) {
  
  if( !LIS3DH_ReadReg(LIS3DH_FIFO_SRC_REG, val) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}

/*******************************************************************************
* Function Name  : LIS3DH_Int1LatchEnable
* Description    : reboot sensor, reset mem registers function
* Input          : ENABLE/DISABLE
* Output         : None
* Return         : Status [MEMS_ERROR, MEMS_SUCCESS]
*******************************************************************************/
status_t LIS3DH_RESET_MEM(void)
{
    u8_t value;

    if( !LIS3DH_ReadReg(LIS3DH_CTRL_REG5, &value) )
        return MEMS_ERROR;

    value &= 0x7F;
    value |= 1<<LIS3DH_BOOT;

    if( !LIS3DH_WriteReg(LIS3DH_CTRL_REG5, value) )
        return MEMS_ERROR;

    return MEMS_SUCCESS;
}

      
void gSensor_Slope_Intr_config(void)
{
	LIS3DH_SetIntConfiguration(LIS3DH_INT1_ZHIE_ENABLE | LIS3DH_INT1_ZLIE_ENABLE |
					 LIS3DH_INT1_YHIE_ENABLE | LIS3DH_INT1_YLIE_ENABLE |
					 LIS3DH_INT1_XHIE_ENABLE | LIS3DH_INT1_XLIE_ENABLE); 
}

void gSensor_Slope_Intr_Enable(unsigned char flag)
{
  if(flag==0)
  {
   sensor_INT_enable=0;	
  }
  else
  {
   sensor_INT_enable=1;
    sensor_INT_check=1;
  }
  LIS3DH_SetInt1Pin(LIS3DH_I1_INT1_ON_PIN_INT1_ENABLE,flag);
}
unsigned char gSensor_Slope_Intr_Enable_check(void)
{
  return (sensor_INT_enable);
}
void sensor_INT_enable_check(void)
{ 
  if(sensor_INT_check==0)
  {    
    //ke_timer_set(G_SENSOR_INTR1_TIMER,1); 
  #if GSENSOR_INER_PIN1_ENABLE
     app_event_g_sensor_intr1_handler(0);
  #endif
    sensor_INT_check=1;
  }
}
void sensor_INT_set_api(void)
{
  sensor_INT_check=0;
}
void gSensor_set_duration_and_threshold(unsigned char duration,unsigned char threshold)
{
	LIS3DH_SetInt1Duration(duration);
	LIS3DH_SetInt1Threshold(threshold);
}

void LIS3DH_Get_XYZ_Raw(char* buff)
{
#if GSENSOR_ACCESS_TYPE	== GSENSOR_ACCESS_BY_I2C
	lis3dh_i2c_read(LIS3DH_OUT_X_L|0x80, (unsigned char*)buff,6);
  //lis3dh_i2c_read(LIS3DH_OUT_X_L, (unsigned char*)buff,6);
#elif  GSENSOR_ACCESS_TYPE	== GSENSOR_ACCESS_BY_SPI
	lis3dh_spi_read(LIS3DH_OUT_X_L|0xc0, (unsigned char*)buff,6);
#endif
}

unsigned short LIS3DH_Get_X_Raw(void)
{
	unsigned char buff[2]={0x00};
	unsigned short x_value = 0;
	lis3dh_i2c_read(LIS3DH_OUT_X_L, (unsigned char*)buff,1);
	x_value = buff[0];
	lis3dh_i2c_read(LIS3DH_OUT_X_H, (unsigned char*)buff,1);
	x_value = x_value + buff[0]<<8;

	return x_value;
}


status_t LIS3DH_GetAccAxesRaw(AxesRaw_t* buff)
{
    u8_t accReg[6];

#if GSENSOR_ACCESS_TYPE	== GSENSOR_ACCESS_BY_I2C
	lis3dh_i2c_read(LIS3DH_OUT_X_L|0x80, (unsigned char*)accReg,6);
#elif  GSENSOR_ACCESS_TYPE	== GSENSOR_ACCESS_BY_SPI
	lis3dh_spi_read(LIS3DH_OUT_X_L|0xc0, (unsigned char*)accReg,6);
#endif

    buff->AXIS_X = accReg[1]<<8|accReg[0];
    buff->AXIS_Y = accReg[3]<<8|accReg[2];
    buff->AXIS_Z = accReg[5]<<8|accReg[4];

    return MEMS_SUCCESS;
}


#if LIS3DH_FIFO_READ_MODE == LIS3DH_FIFO_READ_BY_INTR
void gSensor_FWM_Interrupt_Enable(unsigned char flag)
{
	LIS3DH_SetInt1Pin(LIS3DH_WTM_ON_INT1_ENABLE,flag);
}
#endif

unsigned short LIS3DH_Get_ADC(void) 
{
	char buff[2]={0};
	unsigned short retValue = 0;
#if GSENSOR_ACCESS_TYPE	== GSENSOR_ACCESS_BY_I2C
	lis3dh_i2c_read(LIS3DH_OUT_1_L, (unsigned char*)&buff[0],1);
	lis3dh_i2c_read(LIS3DH_OUT_1_H, (unsigned char*)&buff[1],1);
#elif  GSENSOR_ACCESS_TYPE	== GSENSOR_ACCESS_BY_SPI
	lis3dh_spi_read(LIS3DH_OUT_1_L|0xc0, (unsigned char*)buff,2);
#endif

	retValue = buff[1]<<8 + buff[0];

	return retValue;
}

void gsensor_reg_reset(void)
{
	LIS3DH_RESET_MEM();
	LIS3DH_WriteReg(LIS3DH_CTRL_REG1, 0);
	LIS3DH_WriteReg(LIS3DH_CTRL_REG2, 0);
	LIS3DH_WriteReg(LIS3DH_CTRL_REG3, 0);
	LIS3DH_WriteReg(LIS3DH_CTRL_REG4, 0);
	LIS3DH_WriteReg(LIS3DH_CTRL_REG5, 0);
	LIS3DH_WriteReg(LIS3DH_CTRL_REG6, 0);
	LIS3DH_FIFOModeEnable(LIS3DH_FIFO_DISABLE);
}



/******************* (C) COPYRIGHT 2012 STMicroelectronics *****END OF FILE****/


