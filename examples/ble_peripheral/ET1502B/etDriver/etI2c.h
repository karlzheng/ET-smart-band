#ifndef _I2C_GPIO_H_
#define _I2C_GPIO_H_
#include "ET1502B.h"


#include "nrf_gpio.h"
//#define IIC_SDA_HIGH   nrf_gpio_pin_set(LCD_SDA_PIN)
//#define IIC_SDA_LOW    nrf_gpio_pin_clear(LCD_SDA_PIN)
//#define IIC_SDA_OUTPUT   nrf_gpio_cfg_output(LCD_SDA_PIN)
//#define IIC_SDA_INPUT    nrf_gpio_cfg_output(LCD_SDA_PIN)
//#define IIC_SDA_DATA_GET  nrf_gpio_pin_read(LCD_SDA_PIN)
#define IIC_SDA_OUTPUT   nrf_gpio_cfg_output(I2C_SDA_PIN)
#define IIC_SDA_INPUT    nrf_gpio_cfg_input(I2C_SDA_PIN,NRF_GPIO_PIN_PULLUP)
#define IIC_SDA_DATA_GET  nrf_gpio_pin_read(I2C_SDA_PIN)
//#define IIC_SCK_LOW    nrf_gpio_pin_clear(LCD_SCL_PIN)
//#define IIC_SCK_HIGH    nrf_gpio_pin_set(LCD_SCL_PIN)
#define IIC_SCK_OUTPUT  nrf_gpio_cfg_output(I2C_SCL_PIN)
#define IIC_SCK_INPUT  nrf_gpio_cfg_input(I2C_SCL_PIN,NRF_GPIO_PIN_PULLUP)



#define IIC_SDA_HIGH   NRF_GPIO->OUTSET= I2C_SDA_PIN_MASK
                          
#define IIC_SDA_LOW    NRF_GPIO->OUTCLR = I2C_SDA_PIN_MASK

#define IIC_SCK_HIGH   NRF_GPIO->OUTSET = I2C_SCL_PIN_MASK
#define IIC_SCK_LOW    NRF_GPIO->OUTCLR = I2C_SCL_PIN_MASK





//#define LEDS_OFF(leds_mask) do {  NRF_GPIO->OUTSET = (leds_mask) & (LEDS_MASK & LEDS_INV_MASK); \
//                            NRF_GPIO->OUTCLR = (leds_mask) & (LEDS_MASK & ~LEDS_INV_MASK); } while (0)

//#define LEDS_ON(leds_mask) do {  NRF_GPIO->OUTCLR = (leds_mask) & (LEDS_MASK & LEDS_INV_MASK); \
//                           NRF_GPIO->OUTSET = (leds_mask) & (LEDS_MASK & ~LEDS_INV_MASK); } while (0)


#ifdef ENABLE_NEW_BOARD
 #if 1
  #define G_IIC_SDA_OUTPUT   G_nrf_gpio_cfg_output(G_I2C_SDA_PIN)
  #define G_IIC_SDA_INPUT    nrf_gpio_cfg_input(G_I2C_SDA_PIN,NRF_GPIO_PIN_PULLUP)//nrf_gpio_cfg_input(G_I2C_SDA_PIN,NRF_GPIO_PIN_PULLUP)
  #define G_IIC_SDA_DATA_GET  nrf_gpio_pin_read(G_I2C_SDA_PIN)
  #define G_IIC_SCK_OUTPUT  G_nrf_gpio_cfg_output(G_I2C_SCL_PIN)//G_nrf_gpio_cfg_output(G_I2C_SCL_PIN)
 #else
	#define G_IIC_SDA_OUTPUT   nrf_gpio_cfg_output(G_I2C_SDA_PIN)
  #define G_IIC_SDA_INPUT    nrf_gpio_cfg_input(G_I2C_SDA_PIN,NRF_GPIO_PIN_NOPULL)//nrf_gpio_cfg_input(G_I2C_SDA_PIN,NRF_GPIO_PIN_PULLUP)
  #define G_IIC_SDA_DATA_GET  nrf_gpio_pin_read(G_I2C_SDA_PIN)
  #define G_IIC_SCK_OUTPUT  nrf_gpio_cfg_output(G_I2C_SCL_PIN)//G_nrf_gpio_cfg_output(G_I2C_SCL_PIN)
 #endif 
 // #define G_IIC_SDA_INPUT_LOW    nrf_gpio_cfg_input(G_I2C_SDA_PIN,NRF_GPIO_PIN_PULLDOWN)
//  #define G_IIC_SCK_INPUT_LOW    nrf_gpio_cfg_input(G_I2C_SCL_PIN,NRF_GPIO_PIN_PULLDOWN)
 // #define G_IIC_SDA_OUTPUT_LOW    Gensor_nrf_gpio_cfg_output(G_I2C_SDA_PIN,NRF_GPIO_PIN_NOPULL)
 // #define G_IIC_SCK_OUTPUT_LOW    Gensor_nrf_gpio_cfg_output(G_I2C_SCL_PIN,NRF_GPIO_PIN_NOPULL)
  

  #define G_IIC_SDA_HIGH   NRF_GPIO->OUTSET= G_I2C_SDA_PIN_MASK
                            
  #define G_IIC_SDA_LOW    NRF_GPIO->OUTCLR = G_I2C_SDA_PIN_MASK

  #define G_IIC_SCK_HIGH   NRF_GPIO->OUTSET = G_I2C_SCL_PIN_MASK
  #define G_IIC_SCK_LOW    NRF_GPIO->OUTCLR = G_I2C_SCL_PIN_MASK
extern void sensor_GPIO_i2c_read(unsigned char I2C_addr,unsigned char reg_addr, unsigned char *buffer, unsigned short len);
extern void sensor_GPIO_i2c_write(unsigned char I2C_addr,unsigned char reg_addr,unsigned char *buffer, unsigned short len);
extern void sensor_GPIO_i2c_init(void);
#endif
extern void et_i2c_init(void);
extern void GPIO_i2c_lowpower(void);
extern void et_i2c_write_conValue(unsigned char I2C_addr,unsigned char reg_addr,unsigned char value,unsigned short len);
extern void et_i2c_write(unsigned char I2C_addr,unsigned char reg_addr,const unsigned char *buffer, unsigned short len);
extern void et_i2c_read(unsigned char I2C_addr,unsigned char reg_addr, unsigned char *buffer, unsigned short len);
#endif
