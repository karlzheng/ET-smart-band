#ifndef _GSENSOR_FIFO_H_
#define _GSENSOR_FIFO_H_
#include "ET1502B.h"
#include "ke_timer.h"


#define BMA250E_FIFO_FRAMES_NUM		(32)
#define BMA250E_FIFO_DATA_AXIS		(3)


#define GPIO_DIR_OUT	0
#define GPIO_DIR_IN		1
#define GPIO_LOW		0
#define GPIO_HIGH		1

//extern void gSensor_io_config(void);
extern void gSensor_init(void);
#if GSENSOR_INER_PIN1_ENABLE
extern void app_event_g_sensor_intr1_handler(unsigned short const msgid);
#endif

#if GSENSOR_INER_PIN2_ENABLE
extern void app_event_g_sensor_intr2_handler(unsigned short const msgid);
#endif


#if LIS3DH_FIFO_ENABLE
extern void gSensor_Clear_Fifo_Data(void);
#if LIS3DH_FIFO_READ_MODE == LIS3DH_FIFO_READ_BY_INTR
#elif LIS3DH_FIFO_READ_MODE == LIS3DH_FIFO_READ_BY_TIMER
#define FIFO_READ_TIME	(16)	//16--ODR:200HZ		nees div 10
extern void usr_gsensor_fifo_read_timer_handler(unsigned short const msgid);
extern void operate_usr_gsensor_fifo_read_timer(unsigned char flag);
#endif

extern void gsensor_reset(void);

#endif

#endif


