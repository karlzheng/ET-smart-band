#ifndef _ETKEYBOARD_H
#define _ETKEYBOARD_H
#include <stdint.h>
#include "et_define.h"
//#include "ET1502B.h"



#define NO_KEY          0xFF   //没有按键
#define OK_KEY          0xF0   //OK键
#define CANCEL_KEY      0x0F   //取消键
#define UP_KEY          0xAA   //UP键
#define DOWN_KEY        0x55   //DOWN键

#define MAX_TOUCH_KEY_BUF_LEN    4//8

extern void Key_value_reset(void);
extern void write_switch_val(uint8_t key_val);
extern uint8_t read_switch_val(void);


#endif

