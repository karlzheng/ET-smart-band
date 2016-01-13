#ifndef _DATA_TRANCSFER_H_
#define _DATA_TRANCSFER_H_
#include "ET1502B.h"
#if DATA_TRANSFER_FEATURE_ENABLE


extern unsigned int ReciveTime;
extern unsigned char DateNumer;
extern unsigned char Read_SportBlock;/*多少个256BYTE*/
extern unsigned char Read_SleepBlock;/*多少个256BYTE*/


extern unsigned char auto_Startsend_sport_record_to_app_timer(void);


extern unsigned char get_ble_connect_status(void);
extern void send_sport_get_app_ack(unsigned char * data,unsigned short length);
extern void send_sleep_get_app_ack(unsigned char * data,unsigned short length);
extern void send_histy_sport_record_get_app_ack(unsigned char * data,unsigned short length);
extern void send_data_timeout_timer(unsigned short const msgid);
extern unsigned char auto_send_sport_record_to_app_timer(void);
extern unsigned char auto_send_histy_sport_record_to_app_timer(void);
extern unsigned char auto_send_sleep_record_to_app_timer(void);
extern void auto_send_history_TotalSleepRecord(void);
extern void return_app_require_data(unsigned char * data,unsigned short length,unsigned char ser_flag);
extern void receive_app_usr_info(unsigned char * data,unsigned short length);
extern void Get_app_usr_info(unsigned char ser_flag);
#if FREE_NOTIC_ARITHMETIC
extern void receive_app_free_notic_info(unsigned char * data,unsigned short length);
#endif
#endif

#endif
