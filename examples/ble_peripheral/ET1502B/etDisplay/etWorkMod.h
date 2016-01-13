#ifndef _ET_WORK_MOD_H_
#define _ET_WORK_MOD_H_
#include <stdint.h>
#if 0

typedef enum
{
  KEY_MODE_NULL=0,
  KEY_MODE_SLEEP,
  KEY_MODE_lIMBO,
  KEY_MODE_STEP_TO_LIMBO,
  KEY_MODE_READY_TO_SLEEP,    
  KEY_MODE_IDLE,
  //KEY_MODE_PAIR,
  KEY_MODE_BTCONNECTED,  
  KEY_MODE_USB,
  KEY_MODE_MAX
}KEY_MODE;
typedef enum
{
  KEY_WP_NULL=0,
  KEY_WP_GPIO1,
  KEY_WP_SPISFRM,
  KEY_WP_TIMER4,
  KEY_WP_MAX
}KEY_WP_TRIM;

typedef struct
{
	KEY_MODE KeyMode;	
	unsigned  KEY_WU_FLAG:8;
	unsigned  no_used11:24;
}keyMode_Str;

extern keyMode_Str keyMD;


//extern void set_work_mod(unsigned char work_mod);
extern unsigned char get_work_mod(void);
 /*
extern void set_reply_connecting_flg(BOOL reply_flg);
extern unsigned char get_reply_connecting_flg(void);
 */

//extern unsigned char get_reply_run_mod_flg(unsigned char *pre_work_mod, unsigned char *cur_work_mod);


//extern void run_idle_mod(BOOL flg);
//extern void run_usb_mod(BOOL flg);
//extern void run_pairing_mod(BOOL flg);
//extern void run_connecting_mod(BOOL flg);
//extern void run_connected_mod(unsigned char flg);
//extern void run_sleep(BOOL flg);
//extern void run_power_on(void);

//extern void ParseWorkMod(void);

extern void clean_auto_poweroff_cnt(void);
extern void auto_poweroff_handle(void);
/************************************************************************************/
uint8_t key_Mode_Set(KEY_MODE temp_mode);

KEY_MODE key_Mode_Get(void);
 
#endif

#endif

