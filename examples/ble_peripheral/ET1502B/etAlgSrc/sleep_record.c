#include "ET1502B.h"
#include "sleep_record.h"
#if SLEEP_RECORD_ARITHMETIC
#include "string.h"
#include "ke_timer.h"
#include "usr_task.h"
#include "gSensor_driver.h"
#include "et_debug.h"
#include "sys_time.h"
#include "arithmetic.h"
#include "app_arithmetic.h"
#include "arithmetic_data_manger.h"
#include "data_manger.h"
#include "common_arithmetic.h"
#include "sleep_analysis.h"
#include "gSensor.h"
#include "sleep_data.h"

extern unsigned char charger_exist_check(void);

#define XDATA_STATIC_MAX_NUM  	(20)  // 60/15(pre Min) * 5Min = 60 times
#define XYZ_STATIC_MIN_DALT		(100)//(600)//(8)
#define STATIC_MIN_SAMPLE_NUM	(XDATA_STATIC_MAX_NUM/10)


#define HIGH_CHAR(x) ((unsigned char)((x>>8)&0xff))
#define LOW_CHAR(x)	 ((unsigned char)(x&0xff))


app_timer_id_t sleep_record_timer_id;
app_timer_id_t gsensor_nomotion_timer_id;

extern gsensor_cfg_struct g_gsensro_cfg;

GSensorStaticStruct gStaticXDataArray[XDATA_STATIC_MAX_NUM] = {0}; //

sleepRawDataStruct gSleepRawCurItem = {0};

unsigned short gSleepDeepXDataSameOnceMaxNum = 0;
unsigned short gSleepDeepXDataSameNum = 0;

unsigned short gSleepWalkOnceSteps = 0;	
unsigned short gSleepSlopeOnceMaxNum = 0;

unsigned short gStaticXDataPos = 0;
unsigned int gStaticXDataPreSec = 0;


static void sleep_add_xyzData(char *xyzBuf, unsigned int curSec)
{
	unsigned int daltSec = 0;
	unsigned short newPos = 0;
	if(gStaticXDataPreSec == 0)
		gStaticXDataPreSec = FourCharGetInt((char*)gSleepRawCurItem.startSecs);
	
	daltSec = curSec - gStaticXDataPreSec;
	newPos = gStaticXDataPos + daltSec/GSENSOR_NO_MOTION_INTERVAL;

	SLP_RCD_INFO("add: daltSec = %d, newPos = %d, preSec = %d, preIndex = %d, curSec = %d,xData = %d ,yData = %d, zData = %d\r\n",
	daltSec,newPos,gStaticXDataPreSec,gStaticXDataPos,curSec,(xyzBuf[1]<<8) | xyzBuf[0],(xyzBuf[3]<<8) | xyzBuf[2],(xyzBuf[5]<<8) | xyzBuf[4]);

	if(newPos < XDATA_STATIC_MAX_NUM)
	{
		gStaticXDataArray[newPos].xData = TwoCharGetShort((char*)&xyzBuf[0]);
		gStaticXDataArray[newPos].yData = TwoCharGetShort((char*)&xyzBuf[2]);
		gStaticXDataArray[newPos].zData = TwoCharGetShort((char*)&xyzBuf[4]);
		gStaticXDataArray[newPos].flag = 1;
		gStaticXDataPos = newPos;
		gStaticXDataPreSec = curSec;
	}
	else
		newPos = XDATA_STATIC_MAX_NUM-1;	
}

static void sleep_xDataSect_SameCal(signed short sameCount, signed short avgXData)
{
	if(sameCount >= STATIC_MIN_SAMPLE_NUM)
	{
		gSleepDeepXDataSameNum += sameCount;//(sameCount - 1);

		if((gSleepRawCurItem.xAxisSameTotalNum + sameCount) < 0xff)
			gSleepRawCurItem.xAxisSameTotalNum += sameCount;
		else
			gSleepRawCurItem.xAxisSameTotalNum = 0xff;
		
		if(gSleepDeepXDataSameNum > gSleepDeepXDataSameOnceMaxNum)
		{
			gSleepDeepXDataSameOnceMaxNum = gSleepDeepXDataSameNum;		
			ShortSetTwoChar((char*)gSleepRawCurItem.xAxisSameAvgData,avgXData);
			SLP_RCD_INFO("xDataSect_SameCal: sameCount = %d, avgXData = %d\r\n",sameCount,avgXData);
		}
	}

	SLP_RCD_INFO("sameCount = %d,SameNum = %d, onceMaxNum = %d, totalNum = %d, avgXData = %d\r\n",
		sameCount,gSleepDeepXDataSameNum,gSleepDeepXDataSameOnceMaxNum,gSleepRawCurItem.xAxisSameTotalNum,avgXData);
	gSleepDeepXDataSameNum = 0;
}

static void sleep_xDataSect_analysis(signed short beginPos, signed short endPos)
{
	signed short i = -1, sameCount = 0;
	signed short avgXData = 0, avgYData = 0, avgZData = 0;

	if(endPos < 0 || beginPos < 0 || beginPos + 1 > endPos)
		return;
	
	avgXData = gStaticXDataArray[beginPos].xData;
	avgYData = gStaticXDataArray[beginPos].yData;
	avgZData = gStaticXDataArray[beginPos].zData;
	
	if(endPos > XDATA_STATIC_MAX_NUM)
		endPos = XDATA_STATIC_MAX_NUM -1;
	
	
	SLP_RCD_INFO("xDataSect_analysis 111111\r\n");
	for(i = beginPos + 1; i <= endPos; i++)
	{
		if((avgYData >= gStaticXDataArray[i].yData ? avgYData - gStaticXDataArray[i].yData : gStaticXDataArray[i].yData - avgYData) < XYZ_STATIC_MIN_DALT)
		{
			avgXData = (avgXData + gStaticXDataArray[i].xData)/2;
			avgYData = (avgYData + gStaticXDataArray[i].yData)/2;
			avgZData = (avgZData + gStaticXDataArray[i].zData)/2;
			if(sameCount == 0)
				sameCount += 2;
			else
				sameCount += 1;
		}
		else
		{
			SLP_RCD_INFO("xDataSect_analysis 222222: beginPos = %d, endPos = %d, i = %d, sameCount = %d\r\n",
				beginPos,endPos,i,sameCount);
			sleep_xDataSect_SameCal(sameCount,avgYData);
			avgYData = gStaticXDataArray[i].yData;
			
			sameCount = 0;
		}
	}

	if(sameCount > 0)
	{
		SLP_RCD_INFO("xDataSect_analysis 33333: sameCount = %d\r\n",sameCount);
		sleep_xDataSect_SameCal(sameCount,avgYData);
		sameCount = 0;
	}
				
}

static void sleep_xDataArray_analysis(void)
{
	signed short i = -1, beginPos = -1, endPos = -1, count = 0;
	
	for(i = 0; i < XDATA_STATIC_MAX_NUM; i++)
	{
		if(gStaticXDataArray[i].flag)
		{
			if(count == 0)
			{
				beginPos = i;
			}
			else
				endPos = i;
			count++;
		}
		else
		{
			if(count > 0) // begin analysis
			{
				if(count < STATIC_MIN_SAMPLE_NUM)
				{
					count = 0;
					continue;
				}
				SLP_RCD_DUG("xDataArray_analysis 1111: count = %d, beginPos = %d, endPos = %d\r\n",count,beginPos,endPos);
				sleep_xDataSect_analysis(beginPos,endPos);
				count = 0;
			}
			else
				continue;
		}
	}
	if(count > 0)
	{
		SLP_RCD_DUG("xDataArray_analysis 2222: count = %d, beginPos = %d, endPos = %d\r\n",count,beginPos,endPos);
		sleep_xDataSect_analysis(beginPos,endPos);
	}
}

static void app_sleep_store_item_end_info(void)
{
	if(gSleepDeepXDataSameNum > gSleepDeepXDataSameOnceMaxNum)
		gSleepDeepXDataSameOnceMaxNum = gSleepDeepXDataSameNum;

	sleep_xDataArray_analysis();

	if(gSleepRawCurItem.walkOnceMaxSteps < gSleepWalkOnceSteps)
		gSleepRawCurItem.walkOnceMaxSteps = gSleepWalkOnceSteps >= 0xff ? 0xff : gSleepWalkOnceSteps;
	gSleepRawCurItem.xAxisSameOnceMaxNum = gSleepDeepXDataSameOnceMaxNum >= 0xff ? 0xff : gSleepDeepXDataSameOnceMaxNum;

	SLP_RCD_DUG("-------------------------------------\r\n");
	QPRINTF("Sleep End Store: startSec = %d, stepsTotalNum = %d, walkTimes = %d, walkOnceMaxSteps = %d, slopeTotalNum = %d\r\n",
		FourCharGetInt((char*)gSleepRawCurItem.startSecs),gSleepRawCurItem.stepsTotalNum,gSleepRawCurItem.walkTimes,gSleepRawCurItem.walkOnceMaxSteps,gSleepRawCurItem.slopeTotalNum);
	SLP_RCD_DUG("slopeOnceMaxNum = %d, noMotionTotalNum = %d, xAxisSameTotalNum = %d, xAxisSameOnceMaxNum = %d, xAxisSameAvgData = %d, devStatus = %d\r\n\r\n",
		gSleepRawCurItem.slopeOnceMaxNum,gSleepRawCurItem.noMotionTotalNum,gSleepRawCurItem.xAxisSameTotalNum,gSleepRawCurItem.xAxisSameOnceMaxNum,TwoCharGetShort((char*)gSleepRawCurItem.xAxisSameAvgData),gSleepRawCurItem.devStatus.oneChar);
#if DATA_MANGER_FEATURE_ENABLE
	//write_data_item_2_mem_pool(DATA_TYPE_SLEEP,(unsigned char*)(&gSleepRawCurItem));
	write_sleep_rocrd_to_raw((unsigned char*)(&gSleepRawCurItem));
#endif
}

static void app_sleep_store_item_begin_info(void)
{
	unsigned int curSec = system_sec_get();
	unsigned char pedometerFlg = 0;

	gSleepDeepXDataSameNum = 0;
	gSleepDeepXDataSameOnceMaxNum = 0;
	gSleepWalkOnceSteps = 0;
	gSleepSlopeOnceMaxNum = 0;
	memset((unsigned char*)(&gSleepRawCurItem),0,sizeof(sleepRawDataStruct));
	IntSetFourChar((char*)gSleepRawCurItem.startSecs,curSec);
	memset((unsigned char*)gStaticXDataArray,0,sizeof(GSensorStaticStruct)*XDATA_STATIC_MAX_NUM);
	gStaticXDataPos = 0;
	gStaticXDataPreSec = 0;
	ShortSetTwoChar((char*)gSleepRawCurItem.xAxisSameAvgData,0);
	
#if WALK_STEP_RECORD_ARITHMETIC
	pedometerFlg = app_get_pedometer_state();
#endif

	if(pedometerFlg)
		gSleepRawCurItem.walkTimes = 1;
	SLP_RCD_INFO("Sleep store begin: startSecs = %d\r\n",FourCharGetInt((char*)gSleepRawCurItem.startSecs));
}

static void app_sleep_gsensor_cfg_judge(void)
{

	s_tm ltime;
    system_time_get(&ltime);
	sleep_gsensor_cfg_judge(ltime.hour);
	
	gSensor_set_duration_and_threshold(g_gsensro_cfg.slopeDuration,g_gsensro_cfg.slopeThreshold);

	#if WALK_STEP_RECORD_ARITHMETIC 
	Arithmetic_Set_YAxis_Min_Threshold(g_gsensro_cfg.stepArithmeticYMin); 
	#endif


	SLP_RCD_INFO("Judge GSensor CFG: index = %d\r\n",preParamIndex);
}


void Sleep_Set_Charger_Flag(unsigned char flag)
{
	SLP_RCD_INFO("Set sleep charger flag %d\r\n",flag);
	gSleepRawCurItem.devStatus.bits.chargerFlag = flag ? 1: 0;
}

void app_sleep_record_timer_handler(void * val)
{ 
	static unsigned int preStoreMinCount = 0;
	static unsigned int curMinCount = 0;	
	SLP_RCD_INFO("Sleep_RCD_Timer: curMin = %d,preMin = %d, rcdInterval = %d\r\n",curMinCount,preStoreMinCount,g_gsensro_cfg.rcdMinInterval);

	curMinCount++;
	if(curMinCount >= preStoreMinCount + g_gsensro_cfg.rcdMinInterval)
	{
		preStoreMinCount = curMinCount;
		#if BATTERY_MANAGER_ENABLE
		#if TEST_SLEEP_ENABLE
		Sleep_Set_Charger_Flag(0);
		#else
		Sleep_Set_Charger_Flag(charger_exist_check());
		#endif
		#endif
		app_sleep_store_item_end_info();
		app_sleep_store_item_begin_info();
	}
	gsensor_reset();
	app_sleep_gsensor_cfg_judge();
	app_timer_start(sleep_record_timer_id, SLEEP_CHECK_INTERVAL, NULL);
}



void app_gSensor_nomotion_timer_handler(void * val)
{
	unsigned int curSec = system_sec_get();
	char xyzBuf[6];

	LIS3DH_Get_XYZ_Raw(xyzBuf);
	sleep_add_xyzData(xyzBuf,curSec);
	
	gSleepRawCurItem.noMotionTotalNum = gSleepRawCurItem.noMotionTotalNum < 0xff ? gSleepRawCurItem.noMotionTotalNum + 1 : 0xff;
	SLP_RCD_INFO("No motion count: %d; %d sec\r\n",gSleepRawCurItem.noMotionTotalNum,curSec);

	app_timer_start(gsensor_nomotion_timer_id, NOMOTION_CHECK_INTERVAL, NULL);
}

void Sleep_Add_Interval_Steps(unsigned short steps)
{
	unsigned short addSteps = gSleepRawCurItem.stepsTotalNum;
	addSteps += steps;
	gSleepRawCurItem.stepsTotalNum = addSteps >= 0xff ? 0xff : addSteps;
	gSleepWalkOnceSteps += steps;
	SLP_RCD_INFO("Add sleep interval %d , total %d steps\r\n",steps,addSteps);
}

void sleep_once_interval_max_steps(void)
{
	if(gSleepWalkOnceSteps > gSleepRawCurItem.walkOnceMaxSteps && gSleepRawCurItem.walkOnceMaxSteps < 0xff)
	{
		gSleepRawCurItem.walkOnceMaxSteps = gSleepWalkOnceSteps > 0xff ? 0xff:gSleepWalkOnceSteps;
	}
	gSleepWalkOnceSteps = 0;
}

void Sleep_Set_Motor_Vibrate_Flag(unsigned char flag)
{
	SLP_RCD_INFO("Set sleep charger flag %d\r\n",flag);
	if((flag != 0 && !gSleepRawCurItem.devStatus.bits.vibrateFlag) ||(flag == 0 && gSleepRawCurItem.devStatus.bits.vibrateFlag))
		gSleepRawCurItem.devStatus.bits.vibrateFlag = (flag != 0) ? 1: 0;
}

void Init_SleepArithmetic(void)
{
#if SLEEP_RECORD_DATA_ENABLE
	sleep_record_flash_init();
#endif

	app_sleep_clear();

	app_sleep_store_item_begin_info();
	app_sleep_gsensor_cfg_judge();
	
	app_timer_start(gsensor_nomotion_timer_id, NOMOTION_CHECK_INTERVAL, NULL);
	app_timer_start(sleep_record_timer_id, SLEEP_CHECK_INTERVAL, NULL);
}

void sleep_timer_create(void)
{
	app_timer_create(&sleep_record_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                app_sleep_record_timer_handler);
	

	app_timer_create(&gsensor_nomotion_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                app_gSensor_nomotion_timer_handler);

}


void sleep_slope_check(void)
{
	static unsigned int preSec= 0;
	unsigned int curSec = system_sec_get();

	if(curSec >= preSec + g_gsensro_cfg.slopeDebounceSec)
	{
		gSleepRawCurItem.slopeTotalNum = gSleepRawCurItem.slopeTotalNum < 0xff ?  gSleepRawCurItem.slopeTotalNum + 1: 0xff;
		if(curSec <= preSec + g_gsensro_cfg.slopeDebounceSec + 1)
		{
			gSleepSlopeOnceMaxNum++;
			if(gSleepSlopeOnceMaxNum > gSleepRawCurItem.slopeOnceMaxNum)
				gSleepRawCurItem.slopeOnceMaxNum = gSleepSlopeOnceMaxNum;
		}
		else
		{
			gSleepSlopeOnceMaxNum = 0;
		}
		
		SLP_RCD_INFO("Slope: count = %d curSec=%d,preSec=%d,gSleepSlopeOnceMaxNum=%d\r\n",gSleepRawCurItem.slopeTotalNum,curSec,preSec,gSleepSlopeOnceMaxNum);
		preSec = curSec;
	}
}


#endif


