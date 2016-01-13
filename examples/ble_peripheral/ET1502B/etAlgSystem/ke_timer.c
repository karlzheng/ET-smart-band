#include "ke_timer.h"
//#include "keyboard.h"
#include "etWorkMod.h"
#include "usr_task.h"
#include "et_define.h"
/*****************/

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "bsp.h"
#include "nrf_drv_timer.h"
#include "app_error.h"
#include "Boards.h"
#include "et_spi_gpio.h"
#include "et_debug.h"
#include "etMotor.h"

/*****************/
struct ke_timer usr_timer[USR_TIMER_MAX] = {{0,0}};
static unsigned char gTimerTickFlg = 0;
static unsigned short gUsrUsingTimerCount = 0;
/*****************/

#define NRF_Timer2Set	1
#if NRF_Timer2Set
const nrf_drv_timer_t TIMER_GSensor = NRF_DRV_TIMER_INSTANCE(2);
//const nrf_drv_timer_t TIMER_PWM = NRF_DRV_TIMER_INSTANCE(1);
//BOOL Timer2flag=FALSE;
/**
 * @brief Handler for timer events.
 */
void timer2_event_handler(nrf_timer_event_t event_type, void* p_context)
{
    	switch(event_type)
    	{
				case NRF_TIMER_EVENT_COMPARE0:
					//gTimerTickFlg = 1;	
					//MotorVibrate();
					//#if 0
					//if(Timer2flag==FALSE)
					//{
					//	FLASH_CS_LOW;
					//	Timer2flag=TRUE;
					//}
					//else
					//{
					//	FLASH_CS_HIGH;
					//	Timer2flag=FALSE;
					//}
					//#endif
					
					//#if DEBUG_UART_EN    
						//DbgPrintf("loop timer_tick\r\n");
					//#endif			
          break;
        
        	default:
           		 //Do nothing.
          break;
    }    
}

void ETTimer2Init(void)
{ 
    /*UINT32 time_ms = 10;//10; //Time(in miliseconds) between consecutive compare events.
    UINT32 time_ticks;
    UINT32 err_code = NRF_SUCCESS;
    gUsrUsingTimerCount = get_usr_using_timer_count();
    //Configure TIMER_LED for generating simple light effect - leds on board will invert his state one after the other.
    err_code = nrf_drv_timer_init(&TIMER_GSensor, NULL, timer2_event_handler);
    APP_ERROR_CHECK(err_code);
    
   time_ticks = nrf_drv_timer_ms_to_ticks(&TIMER_GSensor, time_ms);

    nrf_drv_timer_extended_compare(
         &TIMER_GSensor, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
    
    nrf_drv_timer_enable(&TIMER_GSensor);
    */
   gUsrUsingTimerCount = get_usr_using_timer_count();
}
#endif
//===================================================================
void ke_timer_set(unsigned short const timer_id,  unsigned short const delay)
{
	usr_timer[timer_id].flag = 1;
	usr_timer[timer_id].time = delay;
}

void ke_timer_clear(unsigned short const timer_id)
{
	usr_timer[timer_id].flag = 0;
}
void ke_timer_flag_set(void)
{
  gTimerTickFlg = 1;	
#if DEBUG_UART_EN    
		//DbgPrintf("gTimerTickFlg\r\n");
#endif
}
void ke_schedule(void)
{	
	if(gTimerTickFlg)
	{
		gTimerTickFlg = 0;
		MotorVibrate();
		for(unsigned short timer_id=0;timer_id<USR_TIMER_MAX;timer_id++)
		{
			if(usr_timer[timer_id].flag)
			{				
				if(usr_timer[timer_id].time > 0)
					usr_timer[timer_id].time--;
			}
		}
	}
	
	for(unsigned short timer_id=0;timer_id<USR_TIMER_MAX;timer_id++)
	{
		if(usr_timer[timer_id].flag)
		{
			if(usr_timer[timer_id].time == 0)
			{
				usr_timer[timer_id].flag = 0;
				//DbgPrintf("ke_schedule,ID=%d\r\n",timer_id);
				for(unsigned short fun_id=0;fun_id<gUsrUsingTimerCount;fun_id++)
				{
					if(usr_timer_state[fun_id].id == timer_id)
					{
						usr_timer_state[fun_id].func(timer_id);
					}
				}
			}
		}
	}
	
}
/*
void timer_tick(void)//kevin 10ms
{ 
	gTimerTickFlg = 1;	
	//key_board_scan();
#if DEBUG_UART_EN    
    DbgPrintf("loop timer_tick\r\n");
#endif
}*/
/*
void ke_timer_init(uint8_t temp)
{

    ModuleIrqRemoval(Timer4_Exception); //kevin add
	gUsrUsingTimerCount = get_usr_using_timer_count();
	ModuleIrqRegister(Timer4_Exception, timer_tick_10ms);
	TimerInit(T4, SCU_OSC_100K, 0, TIMER_TICK_10MS_VALUE);	
	TimerIrqEn(T4);
	TimerStart(T4);
	
}
*/


