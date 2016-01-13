#include "ET1502B.h"
#include "free_notic.h"
#include "sys_time.h"
#if FREE_NOTIC_ARITHMETIC
free_notice_struct  free_notic_info = {0xAA,1,1,10,0,20,0,0,0};

void free_notic_onoff(unsigned char on)
{
	static unsigned char on_flag = 0;

	if(on == 0)
	{
	//	QPRINTF("close free notice\r\n");
		on_flag = 0;
		ke_timer_clear(USR_FREE_NOTICE_INFO_TIMER);
	}
	else{
		if(on_flag == 0)
		{
		//	QPRINTF("open free notice\r\n");
			on_flag = 1;
			free_notic_info.count_time = 0; //clear count when open it!
			ke_timer_set(USR_FREE_NOTICE_INFO_TIMER, 60*100);
		}
	}
}

void free_notic_info_timer(unsigned short const msgid)
{
	unsigned int start_time=0,end_time=0;
	unsigned int current_time=0;

	QPRINTF("==free notice count\r\n");
	if((++free_notic_info.count_time) >= free_notic_info.time)
	{
		/*1.clear count time */
		free_notic_info.count_time = 0;
		/*2.motor open */
		if(free_notic_info.free_enable)
		{
			s_tm ltime = {0};
			system_time_get(&ltime);
			current_time = 60*ltime.hour + ltime.minute;
			start_time = free_notic_info.start_time_hour*60 +
						 free_notic_info.start_time_minute;
			end_time   = free_notic_info.end_time_hour*60 + 
						 free_notic_info.end_time_minute;
			
			if((start_time <= current_time) && (current_time < end_time))
			{
				QPRINTF(" free notic : motor turn on \r\n");
			}
		}
	}
	ke_timer_set(USR_FREE_NOTICE_INFO_TIMER, 60*100);

}


#endif



