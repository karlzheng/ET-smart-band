#include "ET1502B.h"
#include "app_arithmetic.h"
#if WALK_STEP_RECORD_ARITHMETIC
#include <stdbool.h>
#include <string.h>
#include "arithmetic.h"
#include "gSensor.h"
#include "arithmetic_data_manger.h"
#include "et_debug.h"
#include "gSensor_driver.h"
#include "sys_time.h"
#include "sleep_record.h"
#include "free_notic.h"
#include "common_arithmetic.h"

#define ARTH_ZERO_STOP_MAX	(2)

typedef struct 
{
	unsigned char bufStatus;
	unsigned short dataNum;	
	unsigned short* pDataBuf;
}arth_buf_t;

char StepArithmeticStartFalg = false;
static unsigned char g_zeroStepCount = 0;

unsigned int g_arthStartSec = 0;
unsigned int g_arthStoreSec = 0;
unsigned int g_arthCurSec = 0;
static unsigned int g_preChangeSec=0;


static unsigned char g_walkStartFlag = 0;
static signed short g_walkStartDetectParseSteps = 0;


static unsigned short gArithDataBuf[BLE_ARTH_BUF_SIZE] = {0};
static unsigned short gArithTransDataBuf[32] = {0};
static void app_arithmetic_cal_timer_handler(void);

static arth_buf_t gArithmeticBuf =
{
	0,
	0,
	gArithDataBuf
};

static arth_buf_t gArithTransBuf =
{
	0,
	0,
	gArithTransDataBuf
};

unsigned char app_get_pedometer_state(void)
{
	return g_walkStartFlag;/*ÊÇ·ñ¿ªÊ¼¼Æ²½²?8S 10²½Ìõ¼þÊÇ·ñ³ÉÁ¢*/
}

static void app_set_pedometer_state(unsigned char flag)
{
	g_walkStartFlag = flag ? 1 : 0;

#if SLEEP_RECORD_ARITHMETIC
	if(flag)
	{
		if(gSleepRawCurItem.walkTimes < 255)
			gSleepRawCurItem.walkTimes++;
	}
#endif
}

static void clear_arithmetic_buf(arth_buf_t *pArthBuf)
{
	memset(pArthBuf->pDataBuf,0,pArthBuf->dataNum*2);
	pArthBuf->bufStatus = 0;
	pArthBuf->dataNum = 0;
}

static bool arithmetic_gsensor_write_data(char* fifoData, unsigned char frameNum)
{
	signed short xVal = 0, yVal = 0, zVal = 0;
	unsigned char i = 0;
	if(gArithTransBuf.bufStatus)
		return false;

	for(i = 0; i < frameNum; i++)
	{
		xVal = TwoCharGetShort(fifoData + i*6);
		yVal = TwoCharGetShort(fifoData + i*6 + 2);
		zVal = TwoCharGetShort(fifoData + i*6 + 4);

		gArithTransBuf.pDataBuf[i] = sqrt_16(xVal*xVal + yVal*yVal + zVal*zVal);
	}
	
	gArithTransBuf.dataNum = frameNum;
	gArithTransBuf.bufStatus = 1;

	return true;
}

static bool arithmetic_transfer_data(void)
{
	if(gArithmeticBuf.bufStatus)
	{
		ARITH_ERR("bufStatus is full\r\n");
		return false;
	}
	
	memcpy(gArithmeticBuf.pDataBuf + gArithmeticBuf.dataNum,gArithTransBuf.pDataBuf,gArithTransBuf.dataNum*2);
	gArithmeticBuf.dataNum += gArithTransBuf.dataNum;

	clear_arithmetic_buf(&gArithTransBuf);
	if(gArithmeticBuf.dataNum > BLE_ARTH_BUF_SIZE - 32 ) /*32¸ö16Î»BUFFER==64BYTE*/
	{
		gArithmeticBuf.bufStatus = 1;
		app_arithmetic_cal_timer_handler();/*¶ÁÈ¡¼Æ²½Ê±¼ä²ÎÊý*/
	}

	return true;
}

void arithmetic_write_data2buf(char* fifoData, unsigned char frameNum)
{
	if(!StepArithmeticStartFalg)
		return;

	if(!arithmetic_gsensor_write_data(fifoData,frameNum))
	{
		if(arithmetic_transfer_data())
		{
			arithmetic_gsensor_write_data(fifoData,frameNum);
			if(arithmetic_transfer_data())
				ARITH_INFO("write data suc case2: mainbuf[%d,%d]\r\n",gArithmeticBuf.bufStatus,gArithmeticBuf.dataNum);
			else
				ARITH_INFO("write gsensor data, has write data to temp buf, main buf is busy 1111\r\n");
		}
		else
		{
			ARITH_INFO("write gsensor data, all buf is full\r\n");
		}
		
	}
	else
	{
		if(!arithmetic_transfer_data())
		{
			ARITH_INFO("write gsensor data, has write data to temp buf, main buf is busy 2222\r\n");
		}
		else
		{
			ARITH_INFO("write data suc case1: mainbuf[%d,%d]\r\n",gArithmeticBuf.bufStatus,gArithmeticBuf.dataNum);
		}
	}
}

void app_start_step_arithmetic_cal(void)
{
	if(!StepArithmeticStartFalg)
	{
		StepArithmeticStartFalg = true;
		
		#if LIS3DH_LOWPOWER_MODE_EN
		ARITH_INFO("gSensor in normal mode\r\n");
		LIS3DH_SetMode(LIS3DH_NORMAL);
		#endif

		
	#if DATA_MANGER_FEATURE_ENABLE
		//app_arthmetic_clear_steps(0);
	#endif
	
		clear_arithmetic_buf(&gArithmeticBuf);
		clear_arithmetic_buf(&gArithTransBuf);
	
	#if LIS3DH_FIFO_ENABLE
		gSensor_Clear_Fifo_Data();
		#if LIS3DH_FIFO_READ_MODE == LIS3DH_FIFO_READ_BY_INTR
		gSensor_FWM_Interrupt_Enable(1);
		#elif LIS3DH_FIFO_READ_MODE == LIS3DH_FIFO_READ_BY_TIMER
		operate_usr_gsensor_fifo_read_timer(1);
		#endif
	#endif	
		g_arthStartSec = system_sec_get();
		g_arthStoreSec = g_arthStartSec;
		
		g_zeroStepCount = 0;
		ARITH_INFO("start arithmetic g_arthStartSec = %d\r\n",g_arthStartSec);
	}
}

void app_stop_step_arithmetic_cal(void)
{
	if(StepArithmeticStartFalg)
	{
		unsigned char sportMode = 0, steps = 0;
		StepArithmeticStartFalg = false;

	#if LIS3DH_LOWPOWER_MODE_EN
		ARITH_INFO("gSensor in low power mode\r\n");
		LIS3DH_SetMode(LIS3DH_LOW_POWER);
	#endif
		
	#if LIS3DH_FIFO_READ_MODE == LIS3DH_FIFO_READ_BY_INTR
		gSensor_FWM_Interrupt_Enable(0);
	#elif LIS3DH_FIFO_READ_MODE == LIS3DH_FIFO_READ_BY_TIMER
		operate_usr_gsensor_fifo_read_timer(0);
	#endif
		
		steps = deal_raw_data((unsigned short *)gArithmeticBuf.pDataBuf, gArithmeticBuf.dataNum, &sportMode,app_arithmetic_get_left_right_hand());
		clear_arithmetic_buf(&gArithmeticBuf);
		app_arithmetic_store_steps(steps,sportMode, 1,app_get_pedometer_state());	
		
		QPRINTF("stop arithmetic: total steps %d, cal = %d, distance = %d----------- \r\n",
			app_arthmetic_get_total_steps(),
			app_arthmetic_get_total_calorie(),
			app_arthmetic_get_total_distance());
				
	#if LIS3DH_FIFO_ENABLE
		gSensor_Clear_Fifo_Data();
	#endif
		app_set_pedometer_state(0);

		g_zeroStepCount = 0;
		g_walkStartDetectParseSteps = 0;

		#if FREE_NOTIC_ARITHMETIC
		free_notic_onoff(1);
		#endif
		
		#if SLEEP_RECORD_ARITHMETIC
		sleep_once_interval_max_steps();
		#endif

	}
}

static void app_arithmetic_cal_timer_handler(void)
{
	unsigned char sportMode = 0, steps = 0;
	signed short daltStartTime = 0;

	steps = deal_raw_data((unsigned short *)gArithmeticBuf.pDataBuf, gArithmeticBuf.dataNum, &sportMode,app_arithmetic_get_left_right_hand());
	clear_arithmetic_buf(&gArithmeticBuf);
	
	g_arthCurSec = system_sec_get();
	daltStartTime = g_arthCurSec - g_arthStartSec;/* ¼ÆËãÔË¶¯¼ä¸ôÊ±¼ä*/

	ARITH_INFO("g_arthCurSec=%x,g_arthStartSec=%x\r\n",g_arthCurSec,g_arthStartSec);
	ARITH_INFO("cur = %x, startFlag = %d, sportMode=%d,steps = %d,total steps=%d\r\n",g_arthCurSec,app_get_pedometer_state(),sportMode,steps,app_arthmetic_get_total_steps());
	
	if(steps > 0)
	{		
		g_zeroStepCount = 0;
			
		if(app_get_pedometer_state())/*ÊÇ·ñ¿ªÊ¼¼Æ²½?8S 10²½Ìõ¼þÊÇ·ñ³ÉÁ¢*/
		{
			//app_arithmetic_store_steps(steps, sportMode,0,1);//NICK Remove
			app_arithmetic_store_steps(steps, sportMode,1,1);
			#if FREE_NOTIC_ARITHMETIC
			free_notic_onoff(0);
			#endif
			ARITH_INFO("arith: detect, walk start. \r\n");
		}
		else
		{
			g_walkStartDetectParseSteps += steps > 3 ? 3 : steps;
			ARITH_INFO("arith: detect, pedometerFlag = %d, daltStartTime = %d, detectSteps = %d \r\n",app_get_pedometer_state(),daltStartTime,g_walkStartDetectParseSteps);		

			if(((daltStartTime <= ARTH_WORK_START_TIME_THRESHOLD) &&
				(g_walkStartDetectParseSteps >= ARTH_WORK_START_STEPS_THRESHOLD))||
				(g_walkStartDetectParseSteps >= ARTH_WORK_START_STEPS_THRESHOLD*2))//8 ÃëÖ®ÄÚ±ØÐë×ß10²½£¬ÉÙÓÚ10²½ÎÞÐ§
			{
				ARITH_INFO("arith: detect, 111111. walkStart = %d\r\n",g_walkStartDetectParseSteps);
						
				app_set_pedometer_state(1);
			
				g_walkStartDetectParseSteps = g_walkStartDetectParseSteps <= daltStartTime*2 ? g_walkStartDetectParseSteps : daltStartTime*2;

				app_arithmetic_store_steps(g_walkStartDetectParseSteps,sportMode,1,1);
				g_walkStartDetectParseSteps = 0;
			}
			else if(daltStartTime > ARTH_WORK_START_TIME_THRESHOLD)
			{
				g_walkStartDetectParseSteps = 0;
				g_arthStartSec = g_arthCurSec;
			}
		}
	}	
	else
	{
		if(app_get_pedometer_state() == 0)
		{
			ARITH_INFO("arith: rebegin the start walk detect. \r\n");
			g_walkStartDetectParseSteps = 0;
			g_arthStartSec = g_arthCurSec;
		}
		g_zeroStepCount++;
		ARITH_INFO("arith: zore step, count = %d\r\n",g_zeroStepCount);		
	}

	if(g_zeroStepCount >= ARTH_ZERO_STOP_MAX) /*Á¬Ðø ARTH_ZERO_STOP_MAX ´ÎÃ»ÓÐ¼Æ²½£¬stop ¼Æ²½*/
		app_stop_step_arithmetic_cal();
}
s_tm g_preChangeTime;

void arithmetic_calibrate_before_rtc_change(void)
{
	g_arthCurSec = system_sec_get();
	g_preChangeSec=g_arthCurSec;
	system_time_get(&g_preChangeTime);
}

void arithmetic_calibrate_after_rtc_change(void)
{
	s_tm curTime;
    unsigned int dlatSec = 0, daltStoreSec = 0;
    dlatSec = g_arthCurSec >= g_arthStartSec ? g_arthCurSec - g_arthStartSec : g_arthStartSec -g_arthCurSec;
	daltStoreSec = g_arthCurSec >= g_arthStoreSec ? g_arthCurSec - g_arthStoreSec : g_arthStoreSec - g_arthCurSec;
    g_arthCurSec = system_sec_get();
    g_arthStartSec = g_arthCurSec - dlatSec;
	g_arthStoreSec = g_arthCurSec - daltStoreSec;
	
    ARITH_INFO("new arithmetic curtime = %d, start time %d secs, dlatSec = %d.\r\n",g_arthCurSec,g_arthStartSec,dlatSec);

	system_time_get(&curTime);
	if(((int)(g_arthCurSec-g_preChangeSec) >= 24*60*60) || 
	   ((int)(g_preChangeSec - g_arthCurSec) >= 24*60*60) ||
	   (curTime.day != g_preChangeTime.day))
	{
		g_arthStartSec = g_arthCurSec;
		g_arthStoreSec = g_arthCurSec;
		app_arthmetic_clear_steps(1);
	}
	if(((int)(g_arthCurSec-g_preChangeSec)>1800)||((int)(g_preChangeSec-g_arthCurSec)>1800))
	{
		ARITH_INFO("g_Cursec=%d, g_preChangeSec=%d\r\n",g_arthCurSec,g_preChangeSec);
		ARITH_INFO("g_arthStartSec=%d, g_arthStoreSec=%d\r\n",g_arthStartSec,g_arthStoreSec);
	#if SLEEP_RECORD_ARITHMETIC
		Init_SleepArithmetic();
	#endif
	}
}


#endif

