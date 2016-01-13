#ifndef _ETKEYSLEEP_H_
#define _ETKEYSLEEP_H_

//#define KEY_SLEEP 1
//#define KEY_NORMAL 0
#define KEY_DELAY_TO_SLEEP_COUNT 10

//typedef struct
//{
//    unsigned short id;
//    ke_msg_func_t func;
//}ke_msg_handler;


typedef struct
{
 unsigned sleep_count:16;
// unsigned sleep_status:8;
 unsigned unused111:16;
}Sleep_stru;
extern Sleep_stru key_sleep;

 void Key_clr_sleep_count(void);
void Key_to_sleepDelay(unsigned short const msgid);
void Key_to_sleepMode(void);


#endif
