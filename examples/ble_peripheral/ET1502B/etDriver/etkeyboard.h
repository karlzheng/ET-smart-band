#ifndef _ETKEYBOARD_H
#define _ETKEYBOARD_H
#include <stdint.h>
#include "et_define.h"
//#include "ET1502B.h"



#define NO_KEY          0xFF   //û�а���
#define OK_KEY          0xF0   //OK��
#define CANCEL_KEY      0x0F   //ȡ����
#define UP_KEY          0xAA   //UP��
#define DOWN_KEY        0x55   //DOWN��

#define MAX_TOUCH_KEY_BUF_LEN    4//8

extern void Key_value_reset(void);
extern void write_switch_val(uint8_t key_val);
extern uint8_t read_switch_val(void);


#endif

