#ifndef _SLEEP_RECORD_H_
#define _SLEEP_RECORD_H_
#include "ET1502B.h"

#if SLEEP_RECORD_ARITHMETIC
#include "app_timer.h"

#define RCD_TIMER_INTERVAL				(60) // 1 min
#define GSENSOR_NO_MOTION_INTERVAL		(15) // 15 sec

#define SLEEP_CHECK_INTERVAL			APP_TIMER_TICKS(RCD_TIMER_INTERVAL*1000, 0)
#define NOMOTION_CHECK_INTERVAL			APP_TIMER_TICKS(GSENSOR_NO_MOTION_INTERVAL*1000, 0)
#define GSENSOR_CHECK_INTERVAL			APP_TIMER_TICKS(1000, 0)


typedef enum
{
	GSENSOR_CFG_DAY = 0,	// 10:00 - 20:00
	GSENSOR_CFG_MORING,		// 07:00 - 09:00
	GSENSOR_CFG_EVENING,	// 20:00 - 23:59
	GSENSOR_CFG_NIGHT,		// 00:00 - 07:00
	GSENSOR_CFG_MAX,
}GSENSOR_CFG_ENUM;



typedef struct{
	unsigned char startSecs[4];  		// cur time in sec from 1970.1.1 00:00:00	代表睡眠的记录时间（低位在前）
	unsigned char stepsTotalNum;		//5 min generated total steps 			代表5 min 产生总的步数
	unsigned char walkTimes;			//5 min boot pemdometer times        	代表5 min 启动记步的次数
	unsigned char walkOnceMaxSteps;		//5 min pemdometer once max steps  	代表5 min 1次记步的最大步数
	unsigned char slopeTotalNum;		//5 min generated slope total times		代表5 min 手环震动总的次数
	unsigned char slopeOnceMaxNum;		//5 min generated slope once max times	代表5 min generated slope once max times
	unsigned char noMotionTotalNum;		//5 min generated no montion times		代表5 min 手环静止产生的次数
	unsigned char xAxisSameTotalNum;	//5 min xAxis sampling total count		代表5 min 加速度x轴 采样总次数
	unsigned char xAxisSameOnceMaxNum;	//5 min xAxis sampling once count		代表5 min xAxis sampling once count
	unsigned char xAxisSameAvgData[2];	//5 min xAxis average value			代表5 min 加速度x轴 平均值（低位在前）
	union{
		unsigned char oneChar;
		struct{
			unsigned char chargerFlag	: 1;	//battery charge flag
			unsigned char vibrateFlag	: 1;	//motor flag
			unsigned char reserveBits	: 6;
		}bits;
	}devStatus;
	unsigned char reserveChars;
}sleepRawDataStruct;

typedef struct
{
	signed short xData;
	signed short yData;
	signed short zData;
	unsigned char flag;
}GSensorStaticStruct;

extern app_timer_id_t gsensor_nomotion_timer_id;

extern sleepRawDataStruct gSleepRawCurItem;

extern void Sleep_Add_Interval_Steps(unsigned short steps);
extern void sleep_once_interval_max_steps(void);
extern void sleep_slope_check(void);
extern void Init_SleepArithmetic(void);
extern void sleep_timer_create(void);

#endif

#endif


