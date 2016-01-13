/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#ifndef _ET1502_H
#define _ET1502_H

// LEDs definitions for PCA10028
//====================================================
#ifdef ENABLE_NEW_BOARD
  #define LEDS_NUMBER    0
  #define LED_1          7//21
  #define LEDS_LIST { LED_1}
#else
  #define LEDS_NUMBER    0

  //#define LED_START      30//21
  #define LED_1          30//21
  #define LED_2          30//22
  #define LED_3          30//23
  #define LED_4          30//24
  //#define LED_STOP       30//24
  #define LEDS_LIST { LED_1, LED_2, LED_3, LED_4 }

#endif

  #define BSP_LED_0      LED_1
  #define BSP_LED_1      LED_2
  #define BSP_LED_2      LED_3
  #define BSP_LED_3      LED_4

#define BSP_LED_0_MASK (1<<BSP_LED_0)
#define BSP_LED_1_MASK (1<<BSP_LED_1)
#define BSP_LED_2_MASK (1<<BSP_LED_2)
#define BSP_LED_3_MASK (1<<BSP_LED_3)

//#define LEDS_MASK      (BSP_LED_0_MASK | BSP_LED_1_MASK | BSP_LED_2_MASK | BSP_LED_3_MASK)
#define LEDS_MASK      (BSP_LED_0_MASK)

/* all LEDs are lit when GPIO is low */
#define LEDS_INV_MASK  LEDS_MASK
//====================================================
#ifdef ENABLE_NEW_BOARD
  #define BUTTONS_NUMBER 5//5
  //#define BUTTON_START   17
  #define BUTTON_1       17
  #define BUTTON_2       18
  #define BUTTON_3       19
  #define BUTTON_4       20
  #define BUTTON_5       30
  //#define BUTTON_STOP    30
#else
  
  #define BUTTONS_NUMBER 5
  //#define BUTTON_START   0//16//17
  #define BUTTON_1       0//16//17
  #define BUTTON_2       1//17//18
  #define BUTTON_3       2//18//19
  #define BUTTON_4       3//19//20
  #define BUTTON_5       25
  //#define BUTTON_STOP    25//17//20
#endif

#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP//NRF_GPIO_PIN_NOPULL//NRF_GPIO_PIN_NOPULL////NRF_GPIO_PIN_PULLUP
#define BUTTON_PULL_SENSOR_NOPULL  NRF_GPIO_PIN_PULLUP//NRF_GPIO_PIN_NOPULL
#define BUTTONS_LIST { BUTTON_1, BUTTON_2, BUTTON_3, BUTTON_4,BUTTON_5}
//#define BUTTONS_LIST { BUTTON_1, BUTTON_2, BUTTON_3, BUTTON_4}

#define BSP_BUTTON_0   BUTTON_1
#define BSP_BUTTON_1   BUTTON_2
#define BSP_BUTTON_2   BUTTON_3
#define BSP_BUTTON_3   BUTTON_4
#define BSP_BUTTON_4   BUTTON_5


#define BSP_BUTTON_0_MASK (1<<BSP_BUTTON_0)
#define BSP_BUTTON_1_MASK (1<<BSP_BUTTON_1)
#define BSP_BUTTON_2_MASK (1<<BSP_BUTTON_2)
#define BSP_BUTTON_3_MASK (1<<BSP_BUTTON_3)
#define BSP_BUTTON_4_MASK (1<<BSP_BUTTON_4)
//#define BUTTONS_MASK   0x001E0000
//====================================================
#ifdef ENABLE_NEW_BOARD
  #define RX_PIN_NUMBER  21//9
  #define TX_PIN_NUMBER  23//8
  #define CTS_PIN_NUMBER 11
  #define RTS_PIN_NUMBER 10
  #define HWFC           0//true
//  #define RX_PIN_NUMBER  9//11
//  #define TX_PIN_NUMBER  8//9
//  #define CTS_PIN_NUMBER 11//11//10
//  #define RTS_PIN_NUMBER 10//8
//  #define HWFC           0//true 
#else
  #define RX_PIN_NUMBER  9//11
  #define TX_PIN_NUMBER  8//9
  #define CTS_PIN_NUMBER 11//11//10
  #define RTS_PIN_NUMBER 10//8
  #define HWFC           0//true
#endif

#define RX_PIN_MASK (1<<RX_PIN_NUMBER )
#define TX_PIN_MASK (1<<TX_PIN_NUMBER)
#define CTS_PIN_MASK (1<<CTS_PIN_NUMBER)
#define RTS_PIN_MASK (1<<RTS_PIN_NUMBER)

//======================================
//====================================================
#ifdef ENABLE_NEW_BOARD
  #define I2C_SCL_PIN   15
  #define I2C_SDA_PIN   16


  
  #define I2C_SCL_PIN_MASK (1UL<<I2C_SCL_PIN)
  #define I2C_SDA_PIN_MASK (1UL<<I2C_SDA_PIN)
  #define I2C_PIN_MASK (I2C_SDA_PIN_MASK |I2C_SCL_PIN_MASK)  

  #define G_I2C_SCL_PIN   29
  #define G_I2C_SDA_PIN   28
  #define G_I2C_SCL_PIN_MASK (1UL<<G_I2C_SCL_PIN)
  #define G_I2C_SDA_PIN_MASK (1UL<<G_I2C_SDA_PIN)
  #define G_I2C_PIN_MASK (G_I2C_SDA_PIN_MASK |G_I2C_SCL_PIN_MASK)  
#else

  #define I2C_SCL_PIN   15
  #define I2C_SDA_PIN   16
  #define I2C_SCL_PIN_MASK (1UL<<I2C_SCL_PIN)
  #define I2C_SDA_PIN_MASK (1UL<<I2C_SDA_PIN)
  #define I2C_PIN_MASK (I2C_SDA_PIN_MASK |I2C_SCL_PIN_MASK)  
#endif

//====================================================
#ifdef ENABLE_NEW_BOARD
  #define LCD_RES_PIN   14
  #define LCD_POWER_PIN 13
#else
  #define LCD_RES_PIN   13
  #define LCD_POWER_PIN 20
#endif
#define LCD_RES_PIN_MASK (1<<LCD_RES_PIN)
#define LCD_POWER_PIN_MASK (1<<LCD_POWER_PIN)
//====================================================
#ifdef ENABLE_NEW_BOARD
  #define USB_DETECT_PIN 5
  #define CHARGE_PIN  6
#else
  #define USB_DETECT_PIN 19
	#define CHARGE_PIN  14
#endif
#define USB_DETECT_PIN_MASK (1<<USB_DETECT_PIN)

//======================================
//#define SPIS_MISO_PIN  28    // SPI MISO signal. 
//#define SPIS_CSN_PIN   12    // SPI CSN signal. 
//#define SPIS_MOSI_PIN  25    // SPI MOSI signal. 
//#define SPIS_SCK_PIN   29    // SPI SCK signal. 
//====================================================
#ifdef ENABLE_NEW_BOARD
  #define SPIM0_SCK_PIN       1    /**< SPI clock GPIO pin number. */
  #define SPIM0_MOSI_PIN      31     /**< SPI Master Out Slave In GPIO pin number. */
  #define SPIM0_MISO_PIN      0     /**< SPI Master In Slave Out GPIO pin number. */
  //#define GSENSOR_SPIM0_SS_PIN        7 //GPIO CS    /**< SPI Slave Select GPIO pin number. */
  #define FLASH_SPIM0_SS_PIN        2//29     /**< SPI Slave Select GPIO pin number. */
  //#define SPIM0_SS_PIN   7  //hardword CS :kevin use for debug
#else
  #define SPIM0_SCK_PIN       24//4     /**< SPI clock GPIO pin number. */
  #define SPIM0_MOSI_PIN      22//22//1     /**< SPI Master Out Slave In GPIO pin number. */
  #define SPIM0_MISO_PIN      23//23//3     /**< SPI Master In Slave Out GPIO pin number. */
  #define GSENSOR_SPIM0_SS_PIN        21//29//2 //GPIO CS    /**< SPI Slave Select GPIO pin number. */
  #define FLASH_SPIM0_SS_PIN        29//2     /**< SPI Slave Select GPIO pin number. */

  #define GSENSOR_SPIM0_SS_PIN_MASK (1<<GSENSOR_SPIM0_SS_PIN)  
#endif

#define SPIM0_SCK_PIN_MASK (1<<SPIM0_SCK_PIN )
#define SPIM0_MOSI_PIN_MASK (1UL<<SPIM0_MOSI_PIN)
#define SPIM0_MISO_MASK (1<<SPIM0_MISO_PIN )
#define SFLASH_SPIM0_SS_PIN_MASK (1<<FLASH_SPIM0_SS_PIN)



//=====================================
//====================================================
#ifdef ENABLE_NEW_BOARD
  #define Z8_PW_PIN   24
  #define MOTOR_PW_PIN   3
  #define GSENSOR_INTR_PIN_1		30
  #define GSENSOR_INTR_PIN_2		8  
#else
  #define Z8_PW_PIN   12
  #define MOTOR_PW_PIN   6
  #define GSENSOR_INTR_PIN_1		25
  #define GSENSOR_INTR_PIN_2		28 
#endif
#define Z8_PW_PIN_MASK (1<<Z8_PW_PIN)
#define MOTOR_PW_PIN_MASK (1<<MOTOR_PW_PIN)
//=====================================




//===================enable module=============================
#define ENABLE_PROTOCOL 1
#define ENABLE_PEDOMETER 1

#define ENABLE_DISPLAY 1
#define ENABLE_ALARM 1
#define ENABLE_COS 1
#define ENABLE_GT24L16M1Y_UT 1
//#define ENABLE_UARTx 0
#define ENABLE_WDT 1
#define ENABLE_SPI 1
#define ENABLE_GET_FLASH_ID 1
//=============================

/***************************************************************************************/
#define GSENSOR_TYPE_NULL				0
#define GSENSOR_TYPE_LIS3DH				1


#define I2C_BUS_TYPE_HARDWARE			1
#define I2C_BUS_TYPE_GPIO				2

#define SPI_BUS_TYPE_HARDWARE			1
#define SPI_BUS_TYPE_GPIO				2

#define GSENSOR_ACCESS_BY_I2C	0
#define GSENSOR_ACCESS_BY_SPI	1

#define LIS3DH_FIFO_READ_BY_INTR		1
#define LIS3DH_FIFO_READ_BY_TIMER		2

#define ARITHMETIC_LEFT_HAND			0
#define ARITHMETIC_RIGHT_HAND			1

#define XIAOMI_ARITHMETIC				0
#define WUFAN_ARITHMETIC				1
#define MODIFY_WUFAN_ARITHMETIC			2

//===================================================
//#define GSENSOR_DEV_TYPE	GSENSOR_TYPE_LIS3DH

//#if GSENSOR_DEV_TYPE == GSENSOR_TYPE_LIS3DH
#ifdef ENABLE_NEW_BOARD
#define GSENSOR_ACCESS_TYPE		GSENSOR_ACCESS_BY_I2C
#else
#define GSENSOR_ACCESS_TYPE		GSENSOR_ACCESS_BY_SPI
#endif

#if GSENSOR_ACCESS_TYPE == GSENSOR_ACCESS_BY_I2C
#define I2C_BUS_TYPE 	I2C_BUS_TYPE_GPIO
#endif

#define GSENSOR_INER_PIN1_ENABLE	(1)	// intr1 pin used
#define GSENSOR_INER_PIN2_ENABLE	(0) // intr2 pin used

#define LIS3DH_FIFO_ENABLE			1	// bosch 250e data fifo function switcher
#define LIS3DH_SLOPE_ENENABLE		1	//
#define LIS3DH_LOWPOWER_MODE_EN		0

#if LIS3DH_FIFO_ENABLE
#define LIS3DH_FIFO_READ_MODE			LIS3DH_FIFO_READ_BY_TIMER

#if LIS3DH_FIFO_READ_MODE == LIS3DH_FIFO_READ_BY_TIMER
#define FIFO_USER_TIMER_COUNT	(1)
#else
#define FIFO_USER_TIMER_COUNT	(0)
#endif

#endif

#define ARITHMETIC_STEPS_HAND			ARITHMETIC_RIGHT_HAND	

#define WALK_STEP_RECORD_ARITHMETIC			(1)
#define FREE_NOTIC_ARITHMETIC				(0)
#define SLEEP_RECORD_ARITHMETIC				(1)
#define ALARM_NOTIC_ENABLE					(1)
#define DATA_MANGER_FEATURE_ENABLE			(1)
#define DATA_TRANSFER_FEATURE_ENABLE		(1)
#define BATTERY_MANAGER_ENABLE				(1)
#define TAP_DOUBLE_CLCIK_ENABLE				(0)

#if SLEEP_RECORD_ARITHMETIC
#define SLEEP_RECORD_DATA_ENABLE			(1)
#else
#define SLEEP_RECORD_DATA_ENABLE			(0)
#endif

#if WALK_STEP_RECORD_ARITHMETIC
#define USE_ARITHMETIC_FROM					WUFAN_ARITHMETIC
#endif

#define USER_TIMER_COUNT					(3 + FIFO_USER_TIMER_COUNT)

#define DEBUG_UART_EN 						(0)


/* version */
#define COS_VERSION		"1.00.00"
#define HW_VERSION		"2.0-1.6"
#define BT_VERSION		"1.00.05"


#endif // PCA10028_H
