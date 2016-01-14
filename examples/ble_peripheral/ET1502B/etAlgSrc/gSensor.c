#include "ET1502B.h"
#include <string.h>
#include "gSensor.h"
#include "gSensor_driver.h"
#include "app_arithmetic.h"
#include "sleep_record.h"
#include "et_debug.h"
#include "common_arithmetic.h"
#include "data_manger.h"
#include "free_notic.h"
#include "app_timer.h"

app_timer_id_t gsensro_check_timer_id;

#if LIS3DH_FIFO_ENABLE

void gSensor_Clear_Fifo_Data(void)
{
	LIS3DH_FIFOModeEnable(LIS3DH_FIFO_BYPASS_MODE);
	LIS3DH_FIFOModeEnable(LIS3DH_FIFO_MODE);
}

static unsigned char gSensor_Read_Fifo_Data(void)
{
	unsigned char frames = 0,ovrun=0;
	char gBma250eFifoBuf[BMA250E_FIFO_FRAMES_NUM * BMA250E_FIFO_DATA_AXIS * 2] = {0};

	LIS3DH_GetFifoSourceBit(LIS3DH_FIFO_SRC_OVRUN, &ovrun);
    /* Read transaction */
    LIS3DH_GetFifoSourceFSS(&frames);

    if(MEMS_SET == ovrun){
        frames = 32;
    }
    frames = (frames >> 2) << 2;

	
	memset(gBma250eFifoBuf,0,BMA250E_FIFO_FRAMES_NUM*BMA250E_FIFO_DATA_AXIS*2);

	for(unsigned char i=0;i<frames;i++)
	{
		LIS3DH_Get_XYZ_Raw(&gBma250eFifoBuf[i*BMA250E_FIFO_DATA_AXIS*2]);
	}

	gSensor_Clear_Fifo_Data();
	
#if WALK_STEP_RECORD_ARITHMETIC
	arithmetic_write_data2buf((char *)gBma250eFifoBuf,frames);
#endif

	return frames;
}
#if LIS3DH_FIFO_READ_MODE == LIS3DH_FIFO_READ_BY_TIMER
#define SENSOR_INTERVAL	APP_TIMER_TICKS(180, 0)
app_timer_id_t gsensro_fifo_timer_id;
unsigned int test_fifo = 0;
void gsensor_fifo_timer_handle(void * val)
{
	static unsigned char fifoReadFailCount = 0;
	unsigned short fifo_count =0;

	if(StepArithmeticStartFalg)
	{
		app_timer_start(gsensro_fifo_timer_id, SENSOR_INTERVAL, NULL);
		fifo_count = gSensor_Read_Fifo_Data();
		test_fifo = fifo_count;
		if(fifo_count > 0)
		{
			fifoReadFailCount = 0;
		}
		else
		{
			fifoReadFailCount++;
			if(fifoReadFailCount >= 10)
			{
				BOSCH_INFO("gsensor fifo read failed count > 10, stop arithmetic \r\n");
				app_stop_step_arithmetic_cal();
			}
		}
	}
	else
	{
		app_timer_stop(gsensro_fifo_timer_id);
	}

}

void operate_usr_gsensor_fifo_read_timer(unsigned char flag)
{
	if(flag)
	{
		app_timer_start(gsensro_fifo_timer_id, SENSOR_INTERVAL, NULL);
		BOSCH_INFO("start gsensor fifo read timer, delay = %d \r\n",FIFO_READ_TIME);
	}
	else
	{
		app_timer_stop(gsensro_fifo_timer_id);
		// flush fifo buf;
		gSensor_Read_Fifo_Data();
		BOSCH_INFO("stop gsensor fifo read timer\r\n");
	}
}

#endif

#endif

#if TAP_DOUBLE_CLCIK_ENABLE
extern unsigned int test_click_count;
#endif
static void gSensor_Intr_Work_Func(unsigned char pin)
{
	unsigned char intr_status = 0;

	switch(pin)
	{
	#if GSENSOR_INER_PIN1_ENABLE
		case GSENSOR_INTR_PIN_1:
			LIS3DH_GetInt1Src(&intr_status);
			if(intr_status > 0)
			{
				gSensor_Slope_Intr_Enable(0);//kevin delete
				
				#if WALK_STEP_RECORD_ARITHMETIC
				app_start_step_arithmetic_cal();
				#endif
				
       			app_timer_start(gsensro_check_timer_id, GSENSOR_CHECK_INTERVAL, NULL);
				
				#if SLEEP_RECORD_ARITHMETIC
				app_timer_stop(gsensor_nomotion_timer_id);
				app_timer_start(gsensor_nomotion_timer_id, NOMOTION_CHECK_INTERVAL, NULL);
				sleep_slope_check();
				#endif
			}

		#if LIS3DH_FIFO_ENABLE
			#if LIS3DH_FIFO_READ_MODE == LIS3DH_FIFO_READ_BY_INTR
			intr_status = 0;
			LIS3DH_GetFifoSourceReg(&intr_status);
			if(intr_status & 0x40)//fifo overrun
			{
				BOSCH_INFO("fifo intr intrStatus[0]=%d\r\n",intr_status);
				gSensor_Read_Fifo_Data();
			}	
			#endif
		#endif

		#if TAP_DOUBLE_CLCIK_ENABLE
		unsigned char click_status = 0;
		if(LIS3DH_GetClickResponse(&click_status) == MEMS_SUCCESS)
		{
			if(click_status)
			{
				test_click_count++;
				QPRINTF("click_status=0x%02x,\r\n",click_status);
			}
		}
		
		#endif
			break;
	#endif

	#if GSENSOR_INER_PIN2_ENABLE
		case GSENSOR_INTR_PIN_2:
			BOSCH_INFO("gSensor fifo intr 2 function\r\n");
			break;
	#endif
		default:break;
	}
}

#if GSENSOR_INER_PIN1_ENABLE
void app_event_g_sensor_intr1_handler(unsigned short const msgid)
{
	gSensor_Intr_Work_Func(GSENSOR_INTR_PIN_1);
}
#endif

#if GSENSOR_INER_PIN2_ENABLE
void app_event_g_sensor_intr2_handler(unsigned short const msgid)
{
	gSensor_Intr_Work_Func(GSENSOR_INTR_PIN_2);
}
#endif

void gsensor_check_handler(void * val)
{
	app_timer_stop(gsensro_check_timer_id);
	gSensor_Slope_Intr_Enable(1);
}

static void gSensor_driver_init(void)
{	
	gsensor_reg_reset();

	LIS3DH_SetAxis(LIS3DH_X_ENABLE|LIS3DH_Y_ENABLE|LIS3DH_Z_ENABLE);
	
	/*3.set gsensor normal mode*/
	LIS3DH_SetMode(LIS3DH_NORMAL);
	
	/*4.set gsensor samping HZ*/
	LIS3DH_SetODR(LIS3DH_ODR_200Hz);// 1秒200组XYZ数据
	
	/*5.set gsensor sampling rang*/
	LIS3DH_SetFullScale(LIS3DH_FULLSCALE_2);

	/*6.enable fifo mode*/
	#if LIS3DH_FIFO_ENABLE
	LIS3DH_SetTriggerInt(LIS3DH_TRIG_INT1);
	LIS3DH_SetWaterMark(31);//buff 满才来一次中断
	#endif
 
	/*7.enable intr pin*/
	LIS3DH_HPFAOI1Enable(MEMS_ENABLE);// enable INT

	/*8. map intr pin to intr1 */
	gSensor_Slope_Intr_config();// SET 哪个方向中断
    //norflash_test();
	/*9.set intr pin wake up by low */
	LIS3DH_SetInt2Pin(LIS3DH_INT_ACTIVE_LOW);

	/*10.set gSensor duration and threshold*/
	gSensor_set_duration_and_threshold(0,2);

	/*11.set intr mode */
	LIS3DH_SetIntMode(LIS3DH_INT_MODE_6D_MOVEMENT);// XYZ 6 个方向有效

	/*12.set fifo mode */
#if LIS3DH_FIFO_ENABLE
	LIS3DH_FIFOModeEnable(LIS3DH_FIFO_STREAM_MODE);//(LIS3DH_FIFO_MODE);

	#if LIS3DH_FIFO_READ_MODE == LIS3DH_FIFO_READ_BY_INTR
	gSensor_FWM_Interrupt_Enable(0);
	#endif
#endif

#if TAP_DOUBLE_CLCIK_ENABLE

	LIS3DH_SetClickCFG( LIS3DH_ZD_ENABLE | LIS3DH_ZS_DISABLE | LIS3DH_YD_DISABLE | 
						LIS3DH_YS_DISABLE | LIS3DH_XD_DISABLE | LIS3DH_XS_DISABLE);
				 
	LIS3DH_SetClickLIMIT(0x10);//(6);//127ms 127*0.05
	LIS3DH_SetClickLATENCY(0x20);//637ms  637*0.05 (0x33);//
	LIS3DH_SetClickWINDOW(0x40); // 637ms 637*0.05 (0xff);//


	LIS3DH_SetClickTHS(35);

	LIS3DH_SetInt1Pin(LIS3DH_CLICK_ON_PIN_INT1_ENABLE,1);
#endif

	/*13.enable slop intr */
 	gSensor_Slope_Intr_Enable(1);
}

static void gsensor_timer_create(void)
{
	app_timer_create(&gsensro_check_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                gsensor_check_handler);
#if LIS3DH_FIFO_READ_MODE == LIS3DH_FIFO_READ_BY_TIMER
	app_timer_create(&gsensro_fifo_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                gsensor_fifo_timer_handle);
#endif

}

void gSensor_init(void)
{
	gSensor_driver_init();

	#if SLEEP_RECORD_ARITHMETIC
	sleep_timer_create();
	Init_SleepArithmetic();
	#endif

	gsensor_timer_create();
	
	#if DATA_MANGER_FEATURE_ENABLE
	init_data_manger();
	#endif

	#if FREE_NOTIC_ARITHMETIC
	free_notic_onoff(1);
	#endif
	
}

void gsensor_reset(void)
{
	gSensor_driver_init();
}


