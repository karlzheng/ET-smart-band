//#include "timerdrv.h"
//#include "UartDrv.h"
//#include "IomDrv.h"
#include "et_inc.h"
#define TIMER_1MS_LOAD_VALUE 9600//0x7080//0x4b00//0x7080//(8000+1600)  //1ms
//#define TIMER_1MS_LOAD_VALUE 0x7080  //about 3ms
static BOOL  g_Timerflag=FALSE;
BOOL  g_Timer0flag=FALSE;

static unsigned int time_tick_auto = 0;


//unsigned char time_flag=0;
#if 1
void etTimer0IrqService(void)
{
	//TIMER0->CSR |= TIMER_DONESTS_FLAG;
	//g_Timer0flag=FALSE;
	time_tick_auto++;
	//ETTimer0Start();
	//MotorVibrate();
}

unsigned int ETGetTime0Tick(void)
{
	return time_tick_auto;
}
void  ETSetTime0Tick(void)
{
  time_tick_auto=0;
}


void ETTimer0Init(void)
{ 
   //  ModuleIrqRemoval(Timer0_Exception); //kevin add
	//ModuleIrqRegister(Timer0_Exception, etTimer0IrqService);

	//TimerInit(T0, SCU_OSC_CLK, 1, TIMER0_VALUE);	
	//TimerIrqEn(T0);
	//TimerStart(T0);
}
void ETTimer0Stop(void)
{
    if(g_Timer0flag==TRUE)
    {
	//TimerStop(T0);
	g_Timer0flag=FALSE;
    }
}
void ETTimer0Start(void)
{
	//TimerStart(T0);
	g_Timer0flag=TRUE;
}

#endif
/** 
 * @brief 	Timer1 Irq Service
 * @param 
 * @return 
 * @note
 */


void ET_Timer1IrqService(void)
{
//	TIMER1->CSR |= TIMER_DONESTS_FLAG;	//flag clear
    g_Timerflag=FALSE;
//	vUartTimer1IntCallback();
			/*	if(Test_timer==1)
			{
			    Test_timer=0;
				
				IomSetVal(GP6_GPIO6, 1);
			}
			else
			{
			    Test_timer=1;
             	IomSetVal(GP6_GPIO6,0);
			}*/	
	//TimerStart(T1);	//start again 
	//TimerStop(T1);	//stop timer0
	
}
void ET_Timer1Init(void)
{
 //   ModuleIrqRemoval(Timer1_Exception); //kevin add
	//ModuleIrqRegister(Timer1_Exception, ET_Timer1IrqService);	//irq service setting

	//TimerInit(T1, SCU_OSC_CLK, 1, TIMER_1MS_LOAD_VALUE);	//init	timer0 for 1ms
	
	//TimerIrqEn(T1);	//irq enable
	
	//TimerStart(T1);	//start
}
void ET_Timer1Start(void)
{
	//TimerStart(T1);
	g_Timerflag=TRUE;
}

void ET_Timer1Stop(void)
{
    if(g_Timerflag==TRUE)
    {
		//TimerStop(T1);
		g_Timerflag=FALSE;
    }
}

void ET_delay(UINT16 delay_count)
{
   UINT16 delay=0;
   for(;delay_count>0;delay_count--)
   {
      for(delay=2400;delay>0;delay--);
   }

}
*/

