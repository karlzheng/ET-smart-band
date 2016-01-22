#include "etalarm.h"
#include "sys_time.h"
//#include "menu.h"
#include "arithmetic_data_manger.h"
#include "ET1502B.h"
#include "etLcdDisplayApp.h"
#include "et_debug.h"
#include "ke_timer.h"
#include "usr_task.h"
#include "et_flash_offset.h"
#include "etSpi.h"
#include "btprotocol.h"
#include "data_manger.h"
#include "sleep_analysis.h"

#if ENABLE_ALARM

//alarm_data_block_t g_RamAlarmBlock={0xAA,ALARM_ITEM_ONE_SIZE,1,{{0x08,0x7f,7,30},}};//{0x09,0x7f,11,58},{0x09,0x7f,12,01}{0xAA,ALARM_ITEM_MAX_SIZE,1,{0x7f,11,12},};
alarm_data_block_t g_RamAlarmBlock;//={{0x08,0x7f,7,30},}};//{0x09,0x7f,11,58},{0x09,0x7f,12,01}{0xAA,ALARM_ITEM_MAX_SIZE,1,{0x7f,11,12},};
void alarm_init(void)
{
	 unsigned char keyid[FLASH_GET_LENGTH];
	 UINT8 count=0;
	 UINT8 temp=0;
	 etSpim1ReadNorFlashStdMode(KEYID_START_ADDR,keyid,FLASH_GET_LENGTH);
	 if(keyid[ALARM_FLAG_OFFSET]==MAC_VALID_TAG1)
	 {
	   count=keyid[ALARM_LENGTH_OFFSET]/ALARM_ITEM_ONE_SIZE;
		 for(temp=0;temp<count;temp++)
		 { 
				 if(temp<ALARM_ITEM_NUMBER_MAX)
				 {
				 g_RamAlarmBlock.pdata[temp].alarm_enable=((keyid[ALARM_DATA_OFFSET+temp*ALARM_ITEM_ONE_SIZE] & ALARM_ENABLE_BIT)==0)? 0:1;
          if(g_RamAlarmBlock.pdata[temp].alarm_enable==1)
          {
          g_RamAlarmBlock.pdata[temp].alarm_work_today=ALARM_NO_WORKED_TODAY;
          }
          else
          {
          g_RamAlarmBlock.pdata[temp].alarm_work_today=ALARM_WORKED_TODAY;
          }
				 g_RamAlarmBlock.pdata[temp].alarm_ID=temp;
				 g_RamAlarmBlock.pdata[temp].alarm_week=keyid[ALARM_DATA_OFFSET+1+temp*ALARM_ITEM_ONE_SIZE]; 			 
				 g_RamAlarmBlock.pdata[temp].alarm_hour=keyid[ALARM_DATA_OFFSET+2+temp*ALARM_ITEM_ONE_SIZE];
				 g_RamAlarmBlock.pdata[temp].alarm_minute=keyid[ALARM_DATA_OFFSET+3+temp*ALARM_ITEM_ONE_SIZE];
				 g_RamAlarmBlock.pdata[temp].alarm_set=1;
				 }
		 	}

	 }
	 else
	 {
	   g_RamAlarmBlock.pdata[0].alarm_enable=0;
     g_RamAlarmBlock.pdata[0].alarm_work_today=0;
	   g_RamAlarmBlock.pdata[0].alarm_ID=0;
	   g_RamAlarmBlock.pdata[0].alarm_week=0x7f;
	   g_RamAlarmBlock.pdata[0].alarm_hour=17;
	   g_RamAlarmBlock.pdata[0].alarm_minute=49;

	   g_RamAlarmBlock.pdata[1].alarm_enable=0;
     g_RamAlarmBlock.pdata[1].alarm_work_today=0;
	   g_RamAlarmBlock.pdata[1].alarm_ID=1;
	   g_RamAlarmBlock.pdata[1].alarm_week=0x7f;
	   g_RamAlarmBlock.pdata[1].alarm_hour=17;
	   g_RamAlarmBlock.pdata[1].alarm_minute=51; 

	   g_RamAlarmBlock.pdata[2].alarm_enable=0;
     g_RamAlarmBlock.pdata[2].alarm_work_today=0;
	   g_RamAlarmBlock.pdata[2].alarm_ID=2;
	   g_RamAlarmBlock.pdata[2].alarm_week=0x7f;
	   g_RamAlarmBlock.pdata[2].alarm_hour=16;
	   g_RamAlarmBlock.pdata[2].alarm_minute=53;
	 }
}
unsigned char clock_alarm_check(void)
{
	unsigned int curSec = 0;
//	unsigned int dalSec = 0,minSec = 30;
  unsigned int minSec = 30;
	unsigned char i = 0, alarm_open_flag = 0;
	//alarm_info_struct *pClockItem;
	//alarm_str *pClockItem;
	s_tm curTime = {0};
  //  unsigned int alarmSecond  = 0;
	system_time_get(&curTime);
	
	//QPRINTF("aa\r\n");
    curSec = system_sec_get();
	if(curSec >= todayEndSec)
	{	
		//DateChangeFlg=1;
		system_day_sec_init();
		for (i = 0; i < ALARM_ITEM_NUMBER_MAX; ++i)
		{
			g_RamAlarmBlock.pdata[i].alarm_work_today=ALARM_NO_WORKED_TODAY;
		}		
		#if WALK_STEP_RECORD_ARITHMETIC
		SaveEveryDayHistySport_Data_info2_datamanger();
		//app_arthmetic_clear_steps(0);
		#endif

		#if SLEEP_RECORD_ARITHMETIC
		sleep_data_clear();
		#endif
	}
	
  for (i = 0; i < ALARM_ITEM_NUMBER_MAX; ++i)
  {
    	//pClockItem = (alarm_info_struct *)(g_RamAlarmBlock.pdata + i*ALARM_ITEM_MAX_SIZE) ;
		//pClockItem = (alarm_str *)(g_RamAlarmBlock.pdata + i*ALARM_ITEM_MAX_SIZE) ;
		//pClockItem = (alarm_str *)&(g_RamAlarmBlock.pdata[i]) ;
		//QPRINTF("alarm info %d: week=%d, hour=%d,min=%d\r\n", i,pClockItem->week_repeat, pClockItem->hour,pClockItem->min);
		//if(pClockItem->week_repeat & (1 << curTime.weekdays))
		//if((pClockItem->alarm_week & (1 << curTime.weekdays))&&(pClockItem->alarm_enable))
		if((g_RamAlarmBlock.pdata[i].alarm_week & (1 << curTime.weekdays))&&(g_RamAlarmBlock.pdata[i].alarm_enable))
		{
	     if((g_RamAlarmBlock.pdata[i].alarm_work_today==ALARM_NO_WORKED_TODAY)&&(g_RamAlarmBlock.pdata[i].alarm_hour==curTime.hour)&&(g_RamAlarmBlock.pdata[i].alarm_minute==curTime.minute))
	     {
					alarm_open_flag = 1;
				  g_RamAlarmBlock.pdata[i].alarm_work_today=ALARM_WORKED_TODAY;
					if(0 == (g_RamAlarmBlock.pdata[i].alarm_week & 0x80))
					{
						g_RamAlarmBlock.pdata[i].alarm_week &= ~(1 << curTime.weekdays);						
					}					
	    }	
		}
		if(alarm_open_flag)
		{
						alarm_open_flag=0;
						display_alarm(curTime.hour,curTime.minute);
						break;
		}    
  }
	
	//if(alarm_open_flag)
	//{
	//	display_alarm(curTime.hour,curTime.minute);
	//}
	return minSec;
}

void usr_clock_time_unit_handler(unsigned short const msgid)
{
//	unsigned char time = 0;

	//time = clock_alarm_check();
  clock_alarm_check();
	//ke_timer_set(USR_TIME_CHECK_TIMER, time*100);
  ke_timer_set(USR_TIME_CHECK_TIMER, 30);
}


#endif



