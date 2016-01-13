#ifndef _SYS_TIME_H_
#define _SYS_TIME_H_
#include <stdint.h>
//#include "products_feature.h"
typedef unsigned          char UINT8;
typedef unsigned short     int UINT16;
typedef unsigned           int UINT32;

typedef struct{
    unsigned short year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
    unsigned char weekdays;
}s_tm;
typedef struct{
    UINT32 systime_save;
}systime_sleep_str;
extern systime_sleep_str timeSave_str;
//unsigned int gTime_sec;
extern volatile unsigned int todayBeginSec;
extern volatile unsigned int todayEndSec;
extern void system_day_sec_init(void);
//extern void system_time_tick(void);
extern unsigned int system_sec_get(void);
extern void system_time_set(s_tm tm);
extern void system_time_get(s_tm *tm) ;
extern void system_time_get_hourS(s_tm *tm,unsigned int sys_sencond);
extern UINT8 system_time_get_second_ready(void);
extern void system_time_get_second_clr(void);
extern void system_time_updated(UINT32 temp_systime);
extern void systemTime_save_sleepmode(void);
extern void systemTimeUpdated_wakeup(void);
extern void system_time_init(void);
extern void system_time_init_from_sleep(void);

#endif


