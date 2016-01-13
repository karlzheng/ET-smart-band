#include <stdio.h>
//#include "Arch.h"
//#include "SccDrv.h"
//#include "SpiDrv.h"
//#include "UartDrv.h"
//#include "IomDrv.h"
//#include "ScuDrv.h"
//#include "MpuDrv.h"
//#include "WdtDrv.h"
//#include "EfcDrv.h"
//#include "intdrv.h"
//#include "keyboard.h"
//#include "protocol.h"
#include "etWorkMod.h"
//#include "protocol.h"
//#include "../cos/Includes.h"
//#include "../display/etWorkMod.h"
//#include "../cos/macro.h"
//#include "../cos/Global.h"
//#include "global.h"
//#include "etConfig.h"
//#include "debug.h"
#include <stdint.h>
#include "etkeyboard.h"


struct touch_key_event
{
	uint8_t key_rp:4;
	uint8_t key_wp:4;
	uint8_t key_val[MAX_TOUCH_KEY_BUF_LEN];
};
/*
struct key_scan_timer
{
	uint8_t up_key_timer:2;
	uint8_t down_key_timer:2;
	uint8_t ok_key_timer:2;
	uint8_t cancel_key_timer:2;
};*/
static struct touch_key_event  TouchKeyInf = {0,0,{0}};
//static struct key_scan_timer  gKeyScanTimer = {0, 0, 0, 0};

void write_switch_val(uint8_t key_val)
{
	TouchKeyInf.key_val[TouchKeyInf.key_wp++] = key_val;
	if (TouchKeyInf.key_wp >= MAX_TOUCH_KEY_BUF_LEN)
		TouchKeyInf.key_wp = 0;
}

uint8_t read_switch_val(void)
{
	uint8_t key_val = NO_KEY;

	if (TouchKeyInf.key_rp != TouchKeyInf.key_wp)
	{
		key_val = TouchKeyInf.key_val[TouchKeyInf.key_rp];
		TouchKeyInf.key_val[TouchKeyInf.key_rp]=NO_KEY;
		TouchKeyInf.key_rp++;		
		if (TouchKeyInf.key_rp >= MAX_TOUCH_KEY_BUF_LEN)
			TouchKeyInf.key_rp = 0;
	}
	return (key_val);
}

void Key_value_reset(void)
{
    uint8_t len=0;
	for(len=0;len<MAX_TOUCH_KEY_BUF_LEN;len++)
    TouchKeyInf.key_val[len] = NO_KEY;
	
	TouchKeyInf.key_rp=0;
	TouchKeyInf.key_wp=0;
	
}
/*
void key_board_scan(void)
{
	if (!GET_UP_PIN_VAL())
	{
		if (gKeyScanTimer. up_key_timer == 1)
		{
			write_switch_val(CANCEL_KEY);
			
		}
		if (gKeyScanTimer. up_key_timer < 2)
			gKeyScanTimer. up_key_timer++;
	}
	else if(gKeyScanTimer. up_key_timer)
	{
		gKeyScanTimer. up_key_timer = 0;

	}
	
	if (!GET_DOWN_PIN_VAL())
	{
		
		if (gKeyScanTimer. down_key_timer == 1)
		{
			write_switch_val(DOWN_KEY);
		}
		if (gKeyScanTimer. down_key_timer < 2)
			gKeyScanTimer. down_key_timer++;
	}
	else if(gKeyScanTimer. down_key_timer)
	{
		gKeyScanTimer. down_key_timer = 0;		
	}

	if (!GET_OK_PIN_VAL())
	{
		if (gKeyScanTimer. ok_key_timer == 1)
		{
			write_switch_val(OK_KEY);
		}
		if (gKeyScanTimer. ok_key_timer < 2)
			gKeyScanTimer. ok_key_timer++;
	}
	else if(gKeyScanTimer. ok_key_timer)
	{
		gKeyScanTimer. ok_key_timer = 0;

	}

	if (!GET_CANCEL_PIN_VAL())
	{
		if (gKeyScanTimer. cancel_key_timer == 1)
		{
			write_switch_val(UP_KEY);

		}
		if (gKeyScanTimer. cancel_key_timer < 2)
     gKeyScanTimer. cancel_key_timer++;
	}
	else if(gKeyScanTimer. cancel_key_timer)
	{
		gKeyScanTimer. cancel_key_timer = 0;

	}

}
*/


