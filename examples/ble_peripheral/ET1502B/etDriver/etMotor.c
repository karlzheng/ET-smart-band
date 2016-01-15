//#include "et_inc.h"
//#include "etTimer.h"
#include "etMotor.h"
#include "ET1502B.h"
#include "et_Spi_gpio.h"
#include "etLcdDisplayApp.h"
#include "et_debug.h"
#if SLEEP_RECORD_ARITHMETIC
#include "sleep_record.h"
#endif

#include "nrf_drv_timer.h"
#include "app_error.h"

#if SLEEP_RECORD_ARITHMETIC
extern void Sleep_Set_Motor_Vibrate_Flag(unsigned char flag);
#endif
//#include "Boards.h"
//#include "et_spi_gpio.h"
const nrf_drv_timer_t TIMER_PWM = NRF_DRV_TIMER_INSTANCE(1);
//#define MOTOR_CTR_HI   NRF_GPIO->OUTSET = MOTOR_PW_PIN_MASK
//#define MOTOR_CTR_LO   NRF_GPIO->OUTCLR = MOTOR_PW_PIN_MASK
#define MOTOR_CTR_DISABLE   NRF_GPIO->OUTSET = MOTOR_PW_PIN_MASK
#define MOTOR_CTR_ENABLE   NRF_GPIO->OUTCLR = MOTOR_PW_PIN_MASK

Motor_stru Motor_str;
static unsigned char toggle=0;
//======================================================================
void timer_pwm_event_handler(nrf_timer_event_t event_type, void* p_context)
{
    //static uint32_t i;
    //uint32_t led_to_invert = (1 << leds_list[(i++) % LEDS_NUMBER]);
    
  /*  switch(event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:
            //LEDS_INVERT(led_to_invert);
				    if(toggle==0)
						{
               toggle=1;
               nrf_gpio_pin_clear(MOTOR_PW_PIN);
            }
						else
						{
              toggle++;
							if(toggle>5)
								toggle=0;
							nrf_gpio_pin_set(MOTOR_PW_PIN);
            }
            break;
        
        default:
            //Do nothing.
            break;
    }  */ 
					//if(toggle==0)
          //toggle=1;
         /* if(toggle<2)
					{
						 toggle ++;
						 //nrf_gpio_pin_clear(MOTOR_PW_PIN);
						MOTOR_CTR_ENABLE;	
					}
					else
					{
						toggle++;
						//if(toggle>3)
            if(toggle>4)
							toggle=0;
						//nrf_gpio_pin_set(MOTOR_PW_PIN);
						MOTOR_CTR_DISABLE;	
					}*/	
          toggle ++;
          if(toggle>2)
					toggle=0;
          if(toggle<1)
					{
						 //toggle ++;
						 //nrf_gpio_pin_clear(MOTOR_PW_PIN);
						MOTOR_CTR_ENABLE;	
					}
					else 
					{
						//toggle++;
						//if(toggle>3)
            //if(toggle>4)
						//	toggle=0;
						//nrf_gpio_pin_set(MOTOR_PW_PIN);
						MOTOR_CTR_DISABLE;	
					}          
}
//===================================================================
void ETTimer1Init(void)
{
    uint32_t time_ms = 300; //Time(in miliseconds) between consecutive compare events.
    uint32_t time_ticks;
    uint32_t err_code = NRF_SUCCESS;
    
    //Configure all leds on board.
   // LEDS_CONFIGURE(LEDS_MASK);
   // LEDS_OFF(LEDS_MASK);
    
    //Configure TIMER_LED for generating simple light effect - leds on board will invert his state one after the other.
    err_code = nrf_drv_timer_init(&TIMER_PWM, NULL, timer_pwm_event_handler);
    APP_ERROR_CHECK(err_code);
    
    //time_ticks = nrf_drv_timer_ms_to_ticks(&TIMER_LED, time_ms);
	  time_ticks =nrf_drv_timer_us_to_ticks(&TIMER_PWM, time_ms);
    
    nrf_drv_timer_extended_compare(
         &TIMER_PWM, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
    
    //nrf_drv_timer_enable(&TIMER_PWM);
    //nrf_gpio_cfg_output(MOTOR_PW_PIN);//kevin
  
}
void enable_pwm(void)
{
  nrf_drv_timer_enable(&TIMER_PWM);
}
void disable_pwm(void)
{
  nrf_drv_timer_disable(&TIMER_PWM);
	MOTOR_CTR_DISABLE;//MOTOR_CTR_HI;
}
//=====================================================================
void MotorInit(void)
{
	NRF_GPIO->DIRSET = MOTOR_PW_PIN_MASK;
	MOTOR_CTR_DISABLE;//MOTOR_CTR_HI;//IomSetVal(MOTOR_GPIO,1); //
//	Motor_str.Motor_HiLevelTime=0;
//	Motor_str.Motor_LoLevelTime=0;
	Motor_str.Motor_dispAlarm_flag=0;
    Motor_str.Motor_shake_delay=0;
	Motor_str.Motor_NoShake_delay=0;
	Motor_str.Motor_enalbe=0;	
	//MOTOR_CTR_HI;//Iomconfig(MOTOR_GPIO);
	//MOTOR_CTR_HI;//IomInOutSet(MOTOR_GPIO,0);	//out mode
	//MOTOR_CTR_HI;//IomSetVal(MOTOR_GPIO,1); //
}
static void Motor_data_set(void)
{
	switch(Motor_str.Motor_shake_type)
	{
	 case MOTOR_TYPE_ALARM://alarm 
	 Motor_str.Motor_shake_delay=MOTOR_TYPE_ALARM_DELAY;
	 Motor_str.Motor_NoShake_delay=MOTOR_TYPE_NoShake_ALARM_DELAY;
	 break;
	 case MOTOR_TYPE_INCOMING://incoming cal
	 Motor_str.Motor_shake_delay=MOTOR_TYPE_INCOMING_DELAY;
	 Motor_str.Motor_NoShake_delay=MOTOR_TYPE_NoShake_INCOMING_DELAY;
	 break;
	 case MOTOR_TYPE_TEST://incoming cal
	 Motor_str.Motor_shake_delay=MOTOR_TYPE_TEST_DELAY;
	 Motor_str.Motor_NoShake_delay=MOTOR_TYPE_NoShake_TEST_DELAY;
	 break;  
	 default:
	 break;
	}
	//Motor_str.Motor_HiLevelTime=0;
  //  Motor_str.Motor_LoLevelTime=0;

}

void MotorVibrate(void)// motor PWM contorl
{
    //return;
	if(Motor_str.Motor_enalbe==1)
	{
         if(Motor_str.Motor_shake_count)
         {
             if(Motor_str.Motor_shake_delay>0)
             {

							 Motor_str.Motor_shake_delay--;
							 if(Motor_str.Motor_shake_delay==0)
							 {
								 disable_pwm();
							 }
             }
						else if(Motor_str.Motor_NoShake_delay>0)
						{

							Motor_str.Motor_NoShake_delay--;			  
							if(Motor_str.Motor_NoShake_delay==0)
							{
								//Motor_str.Motor_shake_count--;
								if(Motor_str.Motor_shake_count>0)
								{
								Motor_data_set();
								enable_pwm();
                Motor_str.Motor_shake_count--;
								}
							}						
						}
						else
						{
								                    
								if(Motor_str.Motor_shake_count>0)
								{
								Motor_data_set();
								enable_pwm();
                Motor_str.Motor_shake_count--;
								}
                else
                {
                 		MotorCtr(0,0);	
                }
						}
         }
				 else
				 {
					MotorCtr(0,0);		  
				 }
	}
}
void MotorCtr(uint8_t enable,unsigned char type)
{
  if(enable==0)
	{
    MOTOR_CTR_DISABLE;//MOTOR_CTR_HI;
    if(Motor_str.Motor_enalbe==1)
    {
      disable_pwm();
      Motor_str.Motor_enalbe=0;
      if(Motor_str.Motor_dispAlarm_flag==1)
      {
        Motor_str.Motor_dispAlarm_flag=0;
         display_check_to_clear_Screen();
      }
      disable_pwm();
   }
	}
	else
	{
			
    if(check_LCDDispCanChange())
      return;
		switch(type)
		{
		 case MOTOR_TYPE_ALARM://alarm 
		 Motor_str.Motor_shake_count=10;
		 Motor_str.Motor_shake_type=MOTOR_TYPE_ALARM;
		 Motor_data_set();
		 break;
		 case MOTOR_TYPE_INCOMING://incoming cal
		 Motor_str.Motor_shake_count=7;
		 Motor_str.Motor_shake_type=MOTOR_TYPE_INCOMING;
		 Motor_data_set();
		 break;
		 case MOTOR_TYPE_TEST://incoming cal
		 Motor_str.Motor_shake_count=2;
		 Motor_str.Motor_shake_type=MOTOR_TYPE_INCOMING;
		 Motor_data_set();
		 break;		 
		 default:
		 break;
		}
		#if SLEEP_RECORD_ARITHMETIC
        Sleep_Set_Motor_Vibrate_Flag(1);
		#endif
		Motor_str.Motor_enalbe=1;
    toggle=0;
    MOTOR_CTR_ENABLE;	
		enable_pwm();		
	}

}





