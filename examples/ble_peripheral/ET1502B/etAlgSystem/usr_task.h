#ifndef _USR_TASK_H_
#define _USR_TASK_H_
#include "ET1502B.h"
//usr timer msg
enum
{
	USR_TIMER_0 = 0,
#if GSENSOR_INER_PIN1_ENABLE
	G_SENSOR_INTR1_TIMER,
#endif
#if GSENSOR_INER_PIN2_ENABLE
//	G_SENSOR_INTR2_TIMER,
#endif
#if LIS3DH_FIFO_READ_MODE == LIS3DH_FIFO_READ_BY_TIMER
//	USR_GSENSOR_FIFO_READ_TIMER,
#endif

#if DATA_TRANSFER_FEATURE_ENABLE
	USR_AUTO_SEND_DATA_TO_APP_TIMER,
	USR_SEND_DATA_TIMEOUT_TIMER,
#endif  
	USR_TIME_CHECK_TIMER,	
	//USR_GET_BUTTON,
	//USR_DISPLAY_DELAY,
	//USR_TEST_READ_DATA_TIMER,
	//USR_CHECK_RSSI,
	//USR_ALARM_CHECK_TIMER,
	//USR_DISP_COS_STA_BATT_STA,
	//USR_DISP_BT_LOGO,
	//USR_DISP_CLR_BUTTON_LOGO,
	USR_TIMER_MAX
};

typedef void (*ke_msg_func_t)(unsigned short const msgid);
typedef struct
{
    unsigned short id;
    ke_msg_func_t func;
}ke_msg_handler;

extern const ke_msg_handler usr_timer_state[];
extern unsigned short get_usr_using_timer_count(void);

#endif

