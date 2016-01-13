#include <string.h>
#include "etWorkMod.h"
#include "etLcdDisplayApp.h"
//#include "etTimer.h"
#include "et_inc.h"
#if 0
//#define AUTO_POWEROFF_COUNTER   30//s//180//180000//
//#define Timer_AUTO_POWEROFF_COUNTER  300//ms//180//180000//
//#define Step_AUTO_POWEROFF_COUNTER  1//s//180//180000//



//static unsigned char  workMod = DEVICE_POWER_ON;//IDLE_WORK_MOD;//
//static BOOL reply_connect_flg = FIRST_CONNECTING;
//static long long  AutoPowerOffCounter = 0;
//static unsigned int AutoPowerOffCounter = 0;
//static unsigned int time_tick_save = 0;
keyMode_Str keyMD;


#if 1
/*static void WORK_MOD_DELAY_MS(long ms)
{
	//long  time = ETGetTimeTick();
	//while ((ETGetTimeTick()-time) <= ms);
}

void set_work_mod(unsigned char work_mod)
{
	workMod = work_mod;
}
unsigned char get_work_mod(void)
{
	return(workMod);
}

*/
/*
void set_reply_connecting_flg(BOOL reply_flg)
{
	//reply_connect_flg = reply_flg;
}

BOOL get_reply_connecting_flg(void)
{
	return(reply_connect_flg);
}
*/
#if 0
unsigned char get_reply_run_mod_flg(unsigned char *pre_work_mod, unsigned char *cur_work_mod)
{
	unsigned char  reply_flg = 0;

	if (*pre_work_mod == *cur_work_mod)
		reply_flg = 1;
	
	//*pre_work_mod = *cur_work_mod;

	return (reply_flg);
}
#endif
/*
void run_idle_mod(BOOL flg)
{
	if (!flg)
	{
		//Toshiba_EnableToshibaModule();
		//lcd_power_control(LCD_OPEN);
		ke_timer_set(USR_DISP_BT_LOGO,50);
	}
	return;
}


void run_usb_mod(BOOL flg)
{
	if (!flg)
	{
		//clear_AllScreen();
		//display_Keymode(keyMD.KeyMode);	
		//g_currWorkingMode = USB_WORK_STA;
		//open_lcd();
		//clear_screen_display();
		//display_usb_logo_16x24(USB_LOGO_DISP_ENABLE);
	}
	if ((USB_detect()==0))
	{									       
      if(key_Mode_Get()==KEY_MODE_USB)
      {
        //clear_secterScreen(0,4,0,LCD_LOG_COLUME_START);////clear_AllScreen();
        display_check_to_clear_Screen();
		if(Protocol_check_BT_connected()==FALSE)
		{
            key_Mode_Set(KEY_MODE_IDLE);
		}
		else
		{
			key_Mode_Set(KEY_MODE_BTCONNECTED);

		}
	  }
	}
	return;
}*/
/*
void run_pairing_mod(BOOL flg)
{
	if (!flg)
	{
		//static  unsigned char  x = 0;
		clear_secterScreen(0,4,0,LCD_LOG_COLUME_START);//clear_AllScreen();
		//display_Keymode(keyMD.KeyMode);
		key_Mode_Set(KEY_MODE_IDLE);
		//clear_screen_display();
		//display_bt_logo_16x24(BT_LOGO_FLASH_ENABLE);
		//set_bt_logo_flash_state(BT_LOGO_FLASH_ENABLE);
		//display_gb18030_string((unsigned char *)"\n     开机\n", sizeof("\n     开机\n"), 0, 0);
					
	}
	//set_pwr_hold(POWER_ON);
	return;
}
*/
/*
static void bt_module_init(void)
{
	TBYTE btDeviceAdrress[6];
	UINT8 btDeviceName[20];
	UINT8 btSn[20];


	GetBtDeviceAdrress(btDeviceAdrress);				//get device address

	memset(btDeviceName, 0x00, sizeof(btDeviceName));	//get device name
	memset(btSn, 0x00, sizeof(btSn));
	GetDeviceNameAndSN(btDeviceName, btSn);			
			
	Toshiba_InitModule(btDeviceName, btSn, btDeviceAdrress);
	//Toshiba_InitModule(BD_NAME, SERIAL_NUM, BD_ADDR);
	vUartIntRcvEnable(___UartBuf);	//must reinit after module init to rcv cmd again
		
}*/
/*
void run_connecting_mod(BOOL flg)
{
	if (!flg)
	{
		//static  unsigned char  x = 0;
		//g_currWorkingMode = BT_PAIR_STA;
		//key_ver_disp_flg = 0;
		//key_ver_disp_start = 0;
		//RemoveUSB();
		open_lcd();
		clear_all_screen();
		//DisplayEntongLogo(2);
		//DelayMs(2000);
		clear_all_screen();
		
		clear_screen_display();
		display_bt_logo_16x24(BT_LOGO_FLASH_ENABLE);
		set_bt_logo_flash_state(BT_LOGO_FLASH_ENABLE);
		display_pairing_connecting_menu();
		clean_auto_poweroff_cnt();
		//bt_module_init();
		Toshiba_EnableToshibaModule();
		//display_gb18030_string((unsigned char *)"\n     开机\n", sizeof("\n     开机\n"), 0, 0);
		//set_bt_logo_flash_state(BT_LOGO_FLASH_ENABLE);

	}
	display_key_ver_counter();
	display_key_version();
	//set_pwr_hold(POWER_ON);
	return;
	
}*/
/*
void run_connected_mod(BOOL flg)
{
   if (!flg)
	{
		//clear_AllScreen();
		//display_Keymode(keyMD.KeyMode);	
		//g_currWorkingMode = BT_WORK_STA;
		//clear_screen_display();
		//set_reply_connecting_flg(REPLY_CONNECTING);
		//set_bt_logo_flash_state(BT_LOGO_FLASH_DISABLE);
		//display_bt_logo_16x24(BT_LOGO_FLASH_ENABLE);//显示蓝牙logo
	}
	return;

}
*/


//unsigned char  PowerOffDispInf[] = "\n     关机\n";
/*
void run_sleep(BOOL flg)
{
	if((disp_str.LCD_display_keymode_delay==0)&&(Motor_str.Motor_enalbe==0))
	{

		//lcd_power_control(LCD_CLOSE);
		//ET_LCD_PW_close();
		//Key_to_sleepMode();
	}
	
}*/
/*
void run_power_on(void)
{
	#if 0
	//if (!check_power_switch_release())//检测开关键引脚是否为释放
	//{
		//未释放
	//	return;
	//}
	//if less 3s then go to power off
	//set_pwr_hold(POWER_OFF);
	#endif
}
*/

//==============================================================================================================
//static unsigned char pre_work_mod = DEVICE_POWER_ON;
/*
void ParseWorkMod(void)
{
	unsigned char  work_mod = get_work_mod();

	if (work_mod == USB_WORK_MOD)
	{
		return;
	}
	
	//if((g_DrvType == DRV_BOT) || (g_DrvType == DRV_CCID))
	//{
		//set_work_mod(USB_WORK_MOD);
		//return;
	//}

	if ( (g_currWorkingMode&0x0F) == USB_WORK_STA)
	{
		set_work_mod(USB_WORK_MOD);
		return;
	}

	if (read_power_switch())
	{
		if ((pre_work_mod == IDLE_WORK_MOD) || (pre_work_mod == DEVICE_POWER_ON))
		{
			set_work_mod(CONNECTING_WORK_MOD);
		}
		else// if (pre_work_mod != USB_WORK_MOD)
		{
			set_work_mod(DEVICE_POWER_OFF);
		}
	}
	else
	{
		pre_work_mod = get_work_mod();
	}
	#if(DEBUG)
	//entong_uart_print_string("OUT:ParseWorkMod()!");
	#endif
	
}
*/

//==============================================================================================================


/*
void clean_auto_poweroff_cnt(void)
{
	AutoPowerOffCounter = system_sec_get();//ETGetTimeTick();
	time_tick_save=0;
	//ETSetTime0Tick();
	return;
}
*/

/*
void auto_poweroff_handle(void)
{
	unsigned char workMod = key_Mode_Get();

	switch(workMod)
	{
      case KEY_MODE_BTCONNECTED:
	  case KEY_MODE_USB:
	  	clean_auto_poweroff_cnt();
	  break;
	  case KEY_MODE_SLEEP:	  	
      case KEY_MODE_lIMBO:
      //case KEY_MODE_READY_TO_SLEEP:
	  	//DbgPrintf("mode=%d,AutoPowerOffCounter=%d \r\n",workMod,(system_sec_get() - AutoPowerOffCounter));
		//if ((system_sec_get() - AutoPowerOffCounter) >= Timer_AUTO_POWEROFF_COUNTER)
		if ((ETGetTime0Tick() - time_tick_save) >= Timer_AUTO_POWEROFF_COUNTER)
		{
			if ((USB_detect()==0)&&(Motor_str.Motor_enalbe==0))
			{
			    //DbgPrintf("USB not connected,mode=%d\r\n",workMod);
				key_Mode_Set(KEY_MODE_READY_TO_SLEEP);
				clean_auto_poweroff_cnt();
			}
			else
			{
			  //DbgPrintf("USB connected,mode=%d\r\n",workMod);
              clean_auto_poweroff_cnt();
			}

		}
	  break;
      case KEY_MODE_STEP_TO_LIMBO:
		if ((system_sec_get() - AutoPowerOffCounter) >= Step_AUTO_POWEROFF_COUNTER)
		{
			if ((USB_detect()==0)&&(Motor_str.Motor_enalbe==0))
			{
			    //DbgPrintf("USB not connected,mode=%d\r\n",workMod);
				key_Mode_Set(KEY_MODE_READY_TO_SLEEP);
				clean_auto_poweroff_cnt();
			}
			else
			{
			  //DbgPrintf("USB connected,mode=%d\r\n",workMod);
              clean_auto_poweroff_cnt();
			}

		}	  	
	  break;
	  case KEY_MODE_IDLE:	  	
      //case KEY_MODE_PAIR: 
        //DbgPrintf("mode=%d,AutoPowerOffCounter=%d \r\n",workMod,(system_sec_get() - AutoPowerOffCounter));
	    if ((system_sec_get() - AutoPowerOffCounter) >= AUTO_POWEROFF_COUNTER)
		{
			if ((USB_detect()==0)&&(Motor_str.Motor_enalbe==0))
			{
			    //DbgPrintf("USB not connected,mode=%d\r\n",workMod);
				key_Mode_Set(KEY_MODE_READY_TO_SLEEP);
				clean_auto_poweroff_cnt();
			}
			else
			{
			  //DbgPrintf("USB connected,mode=%d\r\n",workMod);
              clean_auto_poweroff_cnt();
			}
		}	  	
	  break;
	  default:
	  break;
	}
}*/
#endif

KEY_MODE key_Mode_Get(void)
{
     return keyMD.KeyMode;
}
/*
uint8 Key_Mode_CheckisUsbBusy(void)
{
   if((keyMD.KeyMode>KEY_MODE_BTCONNECTED) && (keyMD.KeyMode<KEY_MODE_BT_COS_MODE))
   	{
   	return 1;
   	}
   return 0;
}
*/

void key_Mode_Init(void)
{
  keyMD.KeyMode=KEY_MODE_IDLE;
//  keyMD.KEY_WU_FLAG=KEY_WP_NULL;
}


uint8_t key_Mode_Set(KEY_MODE temp_mode)
{    
	if((temp_mode !=keyMD.KeyMode))
	{
		keyMD.KeyMode=temp_mode;
		//display_USB_BT_logo();
        //if(keyMD.KeyMode!=KEY_MODE_SLEEP)
		//display_Keymode(keyMD.KeyMode);	
		
		return 1;
	}
	
	return 0;

}

#endif

//==============================================================================================================

