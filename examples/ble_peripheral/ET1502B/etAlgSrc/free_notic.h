#ifndef _FREE_NOTIC_H_
#define _FREE_NOTIC_H_

#include "ET1502B.h"

#if FREE_NOTIC_ARITHMETIC
typedef struct
{
	unsigned char flag;
	unsigned char free_enable;    
	unsigned char time;    
	unsigned char start_time_hour;    
	unsigned char start_time_minute;    
	unsigned char end_time_hour;    
	unsigned char end_time_minute;    
	unsigned char count_time;		
	unsigned char reservedChar;
}free_notice_struct;

extern free_notice_struct  free_notic_info;
extern void free_notic_onoff(unsigned char on);

extern void free_notic_info_timer(unsigned short const msgid);


#endif

#endif


