#ifndef _GSENSOR_ARITHMETIC_
#define _GSENSOR_ARITHMETIC_
#include "ET1502B.h"
#if WALK_STEP_RECORD_ARITHMETIC

#if USE_ARITHMETIC_FROM == XIAOMI_ARITHMETIC

#define FIR_SMOOTH_INTERVAL		(10)
#define SMOOTH_WIDTH			(10)
#define BLE_ARTH_BUF_SIZE		(96)

#define X_AXIS_WALK_MIN_THRESHOLD			(43)//(36)//(27)
#define X_AXIS_RUN_MIN_THRESHOLD			(20)//(21)//(19)
#define Y_AXIS_DALT_WALK_MIN_THRESHOLD		(6000)//(12000)
#define Y_AXIS_DALT_RUN_MIN_THRESHOLD		(15000)//(25000)

#define Y_AXIS_DALT_DATA					(800)


typedef struct{
	unsigned short* x_array;
	unsigned short* y_array;
	unsigned short num;
}xy_data_array;

extern unsigned char deal_raw_data(unsigned short *y_array, unsigned short length, unsigned char* sport_mode,unsigned char hand);
extern void Arithmetic_Set_YAxis_Min_Threshold(unsigned short data);

#elif USE_ARITHMETIC_FROM == WUFAN_ARITHMETIC
#define FIR_SMOOTH_INTERVAL		(10)
#define SMOOTH_WIDTH			(10)
#define BLE_ARTH_BUF_SIZE		(192)

#define X_AXIS_WALK_MIN_THRESHOLD			(36)
#define X_AXIS_RUN_MIN_THRESHOLD			(22)

#define Y_AXIS_DALT_DAY_MIN_THRESHOLD		(2000)
#define Y_AXIS_DALT_EVENING_MIN_THRESHOLD	(2000)
#define Y_AXIS_DALT_MORING_MIN_THRESHOLD	(2000)
#define Y_AXIS_DALT_NIGHT_MIN_THRESHOLD		(2000)

typedef struct{
	unsigned short* xArray;
	unsigned short* yArray;
	unsigned short num;
}XYDataArray;

extern void Arithmetic_Set_YAxis_Min_Threshold(unsigned short data);
extern unsigned char deal_raw_data(unsigned short *yArray, unsigned short length, unsigned char* pSportMode,unsigned char hand);

#elif USE_ARITHMETIC_FROM == MODIFY_WUFAN_ARITHMETIC

#define FIR_SMOOTH_INTERVAL		(10)
#define SMOOTH_WIDTH			(10)
#define BLE_ARTH_BUF_SIZE		(192)

/* left hand */
#define X_AXIS_WALK_MIN_THRESHOLD_L			(28)
#define X_AXIS_RUN_MIN_THRESHOLD_L			(15)
#define Y_AXIS_DALT_DAY_MIN_THRESHOLD_L		(1150)

#define X_AXIS_WALK_MIN_THRESHOLD_MIN_L		(25)
#define X_AXIS_RUN_MIN_THRESHOLD_MIN_L		(10)
#define Y_AXIS_DALT_DAY_MIN_THRESHOLD_MIN_L	(650)

/*****************************************************/

/* right hand */
#define X_AXIS_WALK_MIN_THRESHOLD_R			(32)
#define X_AXIS_RUN_MIN_THRESHOLD_R			(18)
#define Y_AXIS_DALT_DAY_MIN_THRESHOLD_R		(1400)

#define X_AXIS_WALK_MIN_THRESHOLD_MIN_R		(28)
#define X_AXIS_RUN_MIN_THRESHOLD_MIN_R		(12)
#define Y_AXIS_DALT_DAY_MIN_THRESHOLD_MIN_R	(800)

/*****************************************************/


typedef struct{
	unsigned short* xArray;
	unsigned short* yArray;
	unsigned short num;
}XYDataArray;

extern void Arithmetic_Set_YAxis_Min_Threshold(unsigned short data);
extern unsigned char deal_raw_data(unsigned short *yArray, unsigned short length, unsigned char* pSportMode,unsigned char hand);
#endif
#endif
#endif


