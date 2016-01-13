#ifndef _ALARM_H_
#define _ALARM_H_
#include "ET1502B.h"

#if ENABLE_ALARM
#define ALARM_ITEM_NUMBER_MAX       (3)//(5)//(DATA_BLOCK_LOAD_SIZE/ALARM_ITEM_MAX_SIZE)
#define ALARM_ITEM_ONE_SIZE			(4)//(3) 
#define ALARM_WEEK_CHECK_BIT (0x7f)

#define ALARM_WORKED_TODAY 0
#define ALARM_NO_WORKED_TODAY 1
//typedef struct
//{
//	unsigned char week_repeat; // week alarm repeat bit set
//	unsigned char hour;
//	unsigned char min;
//}alarm_info_struct;
/*
typedef struct
{
	unsigned				un_used_9:27;
	unsigned				alarm_work_today:1;
	unsigned 			   alarm_enable:1;	  
	unsigned				alarm_ID:3;
}alarm_ID;
*/
/*
typedef struct
{
	unsigned char                alarm_ID;
    unsigned char               alarm_week;	
	unsigned char                alarm_hour;
	unsigned char                alarm_minute;
}alarm_str;
*/
typedef struct
{
    unsigned                un_used_9:2;
    unsigned                alarm_work_today:1;
		unsigned                alarm_set:1;
    unsigned                alarm_enable:1;	
	unsigned                alarm_ID:3;
    unsigned                alarm_week:8;	
	unsigned                alarm_hour:8;
	unsigned                alarm_minute:8;	
	
}alarm_str;


typedef struct 
{
	//unsigned char flag;
	//const unsigned char dataItemSize;
	//unsigned char dataItemNums;
	////unsigned char pdata[ALARM_ITEM_MAX_SIZE*5];
	////alarm_str pdata[ALARM_ITEM_ONE_SIZE*ALARM_ITEM_NUMBER_MAX];
	alarm_str pdata[ALARM_ITEM_NUMBER_MAX];
}alarm_data_block_t;
extern alarm_data_block_t g_RamAlarmBlock;
extern void alarm_init(void);
extern void usr_clock_time_unit_handler(unsigned short const msgid);

#endif


#endif


