#include "ET1502B.h"
#include "usr_task.h"
#include "gSensor.h"
#include "app_arithmetic.h"
#include "sys_time.h"
#include "data_manger.h"
#include "sleep_record.h"
#include "et_inc.h"
#include "data_transfer.h"

static void usr_timer_handler(unsigned short const msgid)
{
	
}

const ke_msg_handler usr_timer_state[] =
{
    {USR_TIMER_0,     					(ke_msg_func_t) usr_timer_handler},
	#if GSENSOR_INER_PIN1_ENABLE
	{G_SENSOR_INTR1_TIMER,           	(ke_msg_func_t) app_event_g_sensor_intr1_handler},
	#endif
	#if GSENSOR_INER_PIN2_ENABLE
	//{G_SENSOR_INTR2_TIMER,           	(ke_msg_func_t) app_event_g_sensor_intr2_handler},
	#endif

    #if LIS3DH_FIFO_READ_MODE == LIS3DH_FIFO_READ_BY_TIMER
    //{USR_GSENSOR_FIFO_READ_TIMER,     	(ke_msg_func_t) usr_gsensor_fifo_read_timer_handler},
    #endif

	#if DATA_TRANSFER_FEATURE_ENABLE
	{USR_AUTO_SEND_DATA_TO_APP_TIMER,	(ke_msg_func_t) auto_send_sport_record_to_app_timer},
	{USR_SEND_DATA_TIMEOUT_TIMER,		(ke_msg_func_t) send_data_timeout_timer},
	#endif

	{USR_TIME_CHECK_TIMER,				(ke_msg_func_t) usr_clock_time_unit_handler}
	//{USR_DISPLAY_DELAY,(ke_msg_func_t)Display_delay_check},
	//#if DATA_MANGER_FEATURE_ENABLE
	//{USR_TEST_READ_DATA_TIMER,     		(ke_msg_func_t) test_read_sport_record},
	//#endif
	//{USR_DISP_COS_STA_BATT_STA,(ke_msg_func_t)Display_CosStatus_Batt_BT_ICON},
	//{USR_DISP_BT_LOGO,(ke_msg_func_t)Display_BT_Logo_flash},
	//{USR_DISP_CLR_BUTTON_LOGO,(ke_msg_func_t)DISP_CLR_ButtonLOGO},
	//{USR_CHECK_RSSI,(ke_msg_func_t)display_start_get_RSSI},
};

unsigned short get_usr_using_timer_count(void)
{
	return (unsigned short)(sizeof(usr_timer_state) / sizeof(ke_msg_handler));
}


