//====================
#include "ET1502B.h"
#include "etWorkMod.h"
#include "etmotor.h"
//#include "etbattery.h"
#include "etI2c.h"
#include "etLcdDisplayApp.h"
#include "etkeyboard.h"
#include "sys_time.h"
#include "gSensor.h"
#include "etSpi.h"
#include "et_debug.h"
//#include "et_calender.h"
#include "et_battery.h"
//#include "et_inc.h"
//#include "wdtdrv.h"
//#include "WdtDemo.h"
#include "ke_timer.h"
#include "btprotocol.h"
#include "etalarm.h"
#include "et_WDT.h"

//#include "etMultiPageDisplay.h"
#ifdef ENABLE_SIMPLE_UART //kevin add
#include "etSimple_uart.h"
#endif
#include "uartProtocol.h"
#include "gSensor_driver.h"
#include "data_transfer.h"
#include "arithmetic_data_manger.h"

unsigned char g_charge_status = 0;
unsigned char charger_exist_check(void)
{
	return g_charge_status;
}

static void USB_check(void)
{
   static uint8_t USB_display_flag=0;
  if(USB_detect()==1)
  {
  	g_charge_status = 1;
#if DEBUG_UART_EN    
   // DbgPrintf("USB_detect\r\n");
#endif  
#if ENABLE_COS   
      
      if(Protocol_check_BT_connected()==1)
      {
       simple_uart_init(UART_POWEROFF_BT_DELAY);
      }
      else
      {
       simple_uart_init(UART_POWEROFF_DELAY);
      }
#endif
    
#if ENABLE_DISPLAY	  
				 if(Display_idle_logo()==TRUE)
				   USB_display_flag=1;   
#endif
	}
	else
	{
		g_charge_status = 0;
#if DEBUG_UART_EN    
    //DbgPrintf("USB_not detect\r\n");
#endif 	
#if ENABLE_DISPLAY
				if(USB_display_flag==1)
				{
				  USB_display_flag=0;
				  lcd_power_control(LCD_CLOSE);
				}
#endif

#if ENABLE_COS         
					etCheck_reset();//when change MAC_ADDR,will reset
          etCheck_to_PoweroffCOS();
#endif

	}

}

static void Manage_other_Event(void)
{
	if(system_time_get_second_ready())
	{
     system_time_get_second_clr();
		 if(theSink.Step_RealtimeData_send_ok_flag>0)
		 {
		 theSink.Step_RealtimeData_send_ok_flag=0;
		 }
#if ENABLE_WDT	 
     et_WDT_feed();
#endif
    
#if ENABLE_DISPLAY
     Display_delay_check();
     Check_RSSI_display();
#endif
     //Display_default_message();		
     USB_check();

#if ENABLE_COS
	  check_uart_resend_tag();
#endif	

 }


}



void dev_init(void)
{
#if DEBUG_UART_EN    
    DbgPrintf("\r\n===dev_init===\r\n");
#endif  
#if ENABLE_WDT
   et_WDT_init();	
#endif

#if DEBUG_UART_EN    
    //DbgPrintf_COS_log("et_Z8_powerCtr OK\r\n");
#endif	
#if ENABLE_ALARM
	 MotorInit();
#endif
#if DEBUG_UART_EN 	 
	//DbgPrintf("MotorInit--->ok\r\n");
#endif
	  battery_adc_dev_init();
#if DEBUG_UART_EN    
    //DbgPrintf("adc init OK\r\n");
#endif	
#if ENABLE_ALARM
    alarm_init();
#endif
#if DEBUG_UART_EN    
	//DbgPrintf("adc init OK\r\n");
#endif
	 system_time_init();
#if DEBUG_UART_EN    
   // DbgPrintf("rtc init OK\r\n");
#endif 	
    ETTimer2Init();
#if DEBUG_UART_EN    
	 // DbgPrintf("ETTimer2Init OK\r\n");
#endif 	

    USB_detect_init();
#if DEBUG_UART_EN    
	//DbgPrintf("USB_detect_init OK\r\n");
#endif 
#if ENABLE_PEDOMETER
#ifdef ENABLE_NEW_BOARD
    sensor_GPIO_i2c_init();
#endif
	  gSensor_init();
#endif
#if DEBUG_UART_EN    
	 //DbgPrintf_COS_log("gSensor_init OK\r\n");
#endif
   ETTimer1Init();
#if ENABLE_DISPLAY
	 et_i2c_init();
   LCD_init();
	#if DEBUG_UART_EN    
    DbgPrintf("lcd init OK\r\n");
	#endif	
#endif
  //app_arthmetic_set_total_steps(999990);

}





void main_loop(void)
{
#if(ENABLE_COS) 
     uart_data_handle();
#endif
  
#if ENABLE_PROTOCOL
	    protocol_data_can_send();	
			newService_protocol_data_can_send();
#endif
  
#if ENABLE_PEDOMETER 
    sensor_INT_enable_check();
		ke_schedule();
    
		auto_send_sport_record_to_app_timer();//today each historical segment
		auto_send_sleep_record_to_app_timer();//yesterday sleep record
		auto_send_sport_Live_to_app((UINT8*)&g_sportInfoDMItem,14,SERVICE_STEP);//now step data
		//SaveEveryDayHistySport_Data_info2_datamanger();
		auto_send_histy_sport_record_to_app_timer();//history everyday sport data 
    auto_send_history_TotalSleepRecord();//history sleep record
   //Get_sensor_ID();
#endif


#if ENABLE_DISPLAY		
		DisplayKeyState(read_switch_val());		
#endif
      Manage_other_Event();
}
