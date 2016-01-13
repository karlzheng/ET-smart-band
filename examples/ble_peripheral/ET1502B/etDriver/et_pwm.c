#include "app_error.h"
#include "bsp.h"
#include "nrf_delay.h"
#include "et_pwm.h"
#if 0
APP_PWM_INSTANCE(PWM1,1);                   // Create the instance "PWM1" using TIMER1.
#define MOTOR_CTR_HI   NRF_GPIO->OUTSET = MOTOR_PW_PIN_MASK
#define MOTOR_CTR_LO   NRF_GPIO->OUTCLR = MOTOR_PW_PIN_MASK

static volatile bool ready_flag;            // A flag indicating PWM status.
void pwm_ready_callback(uint32_t pwm_id)    // PWM callback function
{
    ready_flag = true;
}
void et_pwm_init(void)
{
    ret_code_t err_code;
    
    /* 2-channel PWM, 200Hz, output on DK LED pins. */
    //app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_2CH(5000L, BSP_LED_0, BSP_LED_1);
	 app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_1CH(1000L, MOTOR_PW_PIN);
    
    /* Switch the polarity of the second channel. */
    pwm1_cfg.pin_polarity[1] = APP_PWM_POLARITY_ACTIVE_HIGH;
    //pwm1_cfg.pin_polarity[0] = APP_PWM_POLARITY_ACTIVE_HIGH;
    /* Initialize and enable PWM. */
    err_code = app_pwm_init(&PWM1,&pwm1_cfg,pwm_ready_callback);
	  MOTOR_CTR_HI;
    APP_ERROR_CHECK(err_code);
	// while (app_pwm_channel_duty_set(&PWM1, 0, 23) == NRF_ERROR_BUSY);
	 #if 0
    app_pwm_enable(&PWM1);
    while (app_pwm_channel_duty_set(&PWM1, 0, 23) == NRF_ERROR_BUSY);
    //uint32_t value;
    while(true)
    {
        for (uint8_t i = 0; i < 255; ++i)
        {
            //value = (i < 20) ? (i * 5) : (100 - (i - 20) * 5);
					  value=23;
            
            ready_flag = false;
            /* Set the duty cycle - keep trying until PWM is ready... */
            //while (app_pwm_channel_duty_set(&PWM1, 0, value) == NRF_ERROR_BUSY);
            
            /* ... or wait for callback. */
            //while(!ready_flag);
            //APP_ERROR_CHECK(app_pwm_channel_duty_set(&PWM1, 1, value));
            nrf_delay_ms(10);
					 
        }
				app_pwm_disable(&PWM1);
				
    }
		#endif    
}
void et_pwm_enable(void)
{
    app_pwm_enable(&PWM1);
    while (app_pwm_channel_duty_set(&PWM1, 0, 23) == NRF_ERROR_BUSY);
}
void et_pwm_disable(void)
{
    app_pwm_disable(&PWM1);
	  MOTOR_CTR_HI;
}
#endif
