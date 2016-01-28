#ifndef _APP_ARITHMETIC_H_
#define _APP_ARITHMETIC_H_
#include "ET1502B.h"

#if WALK_STEP_RECORD_ARITHMETIC

#include "usr_task.h"

#define ARTH_WORK_START_TIME_THRESHOLD		(8) //8s
#define ARTH_WORK_START_STEPS_THRESHOLD		(10) // 10 steps

extern char StepArithmeticStartFalg;
extern unsigned int g_arthStartSec;
extern unsigned int g_arthStoreSec;
extern unsigned int g_arthCurSec;

extern void arithmetic_write_data2buf(char* fifoData, unsigned char frameNum);
extern void app_start_step_arithmetic_cal(void);
extern void app_stop_step_arithmetic_cal(void);
extern unsigned char app_get_pedometer_state(void);

void arithmetic_calibrate_before_rtc_change(void);
void arithmetic_calibrate_after_rtc_change(void);

#endif


#endif

