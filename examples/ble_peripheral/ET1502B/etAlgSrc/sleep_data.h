#ifndef _SLEEP_DATA_H_
#define _SLEEP_DATA_H_
#include "ET1502B.h"

#if SLEEP_RECORD_DATA_ENABLE
void sleep_record_flash_init(void);
void write_sleep_record_to_flash(unsigned char *buf,unsigned char size);
void read_sleep_record_from_flash(unsigned char *buf,unsigned short len);
unsigned char get_sleep_state_from_ram(unsigned char *stateBuf,
									unsigned short bufLength,
									unsigned char *transferFg,
									unsigned char curPageIndex,
									unsigned char *totalPage);
#endif


#endif
