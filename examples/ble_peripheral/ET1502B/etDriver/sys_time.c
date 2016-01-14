#include "sys_time.h"
#include "ET1502B.h"
#include "usr_task.h"
#include "app_timer.h"
#include "ke_timer.h"
#include "et_debug.h"
#define YEAR_BASE (1970)
#define DAY_SEC      (86400)		/* one day second = 24*60*60 */

#define APP_TIMER_PRESCALER         0
//#define ONE_MINUTE_INTERVAL         APP_TIMER_TICKS(1000*60, APP_TIMER_PRESCALER)
#define ONE_SECOND_INTERVAL         APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER)

static unsigned int gTime_sec = 0;
volatile unsigned int todayBeginSec = 0;
volatile unsigned int todayEndSec = 0;
static unsigned char second_ready=0;
systime_sleep_str timeSave_str;
static app_timer_id_t                 wallClockID;
static char isleap(unsigned short year)
{
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

static unsigned short get_yeardays(unsigned short year) 
{
    if (isleap(year))
        return 366;
    return 365;
}

static unsigned char CaculateWeekDay(unsigned short y,unsigned char m, unsigned char d)
{
	unsigned char week = 0;
    if(m==1||m==2) {
        m+=12;
        y--;
    }
    week=(d+2*m+3*(m+1)/5+y+y/4-y/100+y/400)%7;	
	return week;
}

void system_day_sec_init(void)
{
	unsigned int curSec = 0;
	s_tm curTime;
	system_time_get(&curTime);
	curSec = system_sec_get();  
	// todayMaxSec = curSec + (23h59m59s - curTime)sec + 1sec
	todayBeginSec = curSec - curTime.second - 60*curTime.minute - 60*60*curTime.hour;
	todayEndSec = curSec + (59 - curTime.second) + 60*(59 - curTime.minute) + 60*60*(23 - curTime.hour) + 1; 
}

void system_time_set(s_tm tm)
{
	unsigned int retDay = 0;
	unsigned short year = tm.year;
	unsigned char i,mons[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	if(tm.month>12 || tm.month==0 || tm.day>31 || tm.day == 0 || tm.hour > 23 || tm.minute >59)
		return;
	if(tm.second > 59)
		tm.second = 0;
	
	while((year--)>YEAR_BASE)
	{
		retDay += get_yeardays(year);
	}

	if (isleap(tm.year)) 
		mons[1] += 1;
	
	for (i = 1; i < tm.month; i++) 
	{
		retDay += mons[i-1];
	}

	retDay += tm.day-1;

	retDay *= DAY_SEC;

	retDay += tm.hour*3600;
	retDay += tm.minute*60;
	retDay += tm.second;
	gTime_sec = retDay;
    //DbgPrintf("systime =%d \r\n",gTime_sec);
	system_day_sec_init();
#if ENABLE_ALARM//ENABLE_PEDOMETER
	//ke_timer_set(USR_TIME_CHECK_TIMER, 2*100);//kevin delete
  ke_timer_set(USR_TIME_CHECK_TIMER, 2);//kevin add
#endif
}

unsigned int system_sec_get(void)
{
	return gTime_sec;
}

void system_time_get(s_tm *tm) 
{
	unsigned char i, mons[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	unsigned short curr_day = 0;
	unsigned int days = 0;
	days = gTime_sec/DAY_SEC;
	
	curr_day = get_yeardays(tm->year = YEAR_BASE);
    while (days >= curr_day)
	{
        days -= curr_day;
        tm->year += 1;
        curr_day = get_yeardays(tm->year);
    }
	
    if (isleap(tm->year))
		mons[1] += 1;
    for (i = 0; i < 12; i++) 
	{
        if (days < mons[i]) 
		{
            tm->month = i+1;
            tm->day = days+1;
            break;
        }
		else
        	days -= mons[i];
    }
	
	tm->hour = (gTime_sec % DAY_SEC)/(3600);
    tm->minute = (gTime_sec % DAY_SEC)%(3600)/60;
    tm->second = (gTime_sec % DAY_SEC)%60;
	tm->weekdays = CaculateWeekDay(tm->year,tm->month,tm->day);
}
void system_time_get_hourS(s_tm *tm,unsigned int sys_sencond) 
{
	unsigned char i, mons[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	unsigned short curr_day = 0;
	unsigned int days = 0;
	days = sys_sencond/DAY_SEC;
	if(sys_sencond==0)
		return;
	curr_day = get_yeardays(tm->year = YEAR_BASE);
    while (days >= curr_day)
	{
        days -= curr_day;
        tm->year += 1;
        curr_day = get_yeardays(tm->year);
    }
	
    if (isleap(tm->year))
		mons[1] += 1;
    for (i = 0; i < 12; i++) 
	{
        if (days < mons[i]) 
		{
            tm->month = i+1;
            tm->day = days+1;
            break;
        }
		else
        	days -= mons[i];
    }
	
	tm->hour = (sys_sencond % DAY_SEC)/(3600);
    tm->minute = (sys_sencond % DAY_SEC)%(3600)/60;
    tm->second = (sys_sencond % DAY_SEC)%60;
	tm->weekdays = CaculateWeekDay(tm->year,tm->month,tm->day);
}

void system_time_tick(void * p_context)
{
	gTime_sec++;
	second_ready=1;
  ke_timer_flag_set();
  //gTimerTickFlg = 1;	
//#if DEBUG_UART_EN    
		//DbgPrintf(" rtc counter = %d \r\n", gTime_sec);
//#endif
}
UINT8 system_time_get_second_ready(void)
{
  return second_ready;
}
void system_time_get_second_clr(void)
{
   second_ready=0;
}
void system_time_updated(UINT32 temp_systime)
{
  gTime_sec=temp_systime;
}
void systemTime_save_sleepmode(void)
{
 // timeSave_str.systime_save=TbcGetCnt();
}
void systemTimeUpdated_wakeup(void)
{
//  gTime_sec+=TbcGetCnt()-timeSave_str.systime_save;
}

void system_time_init(void)
{
	uint32_t err_code;
	s_tm tm;
	tm.year = 2016;
	tm.month = 1;
	tm.day = 14;
	tm.hour = 12;
	tm.minute = 0;
	tm.second= 0;
	system_time_set(tm);
	
	  err_code = app_timer_create(&wallClockID, APP_TIMER_MODE_REPEATED, system_time_tick);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_start(wallClockID, ONE_SECOND_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}

void system_time_init_from_sleep(void)
{
	systemTimeUpdated_wakeup();
	//TbcIntDis(SECINT);//kai.ter add because not disable tbc intr have boot false
	//TbcIntEn(SECINT);
}



