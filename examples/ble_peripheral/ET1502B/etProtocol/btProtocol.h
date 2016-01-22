#ifndef _PROTOCOL_H_
#define  _PROTOCOL_H_
#include <stdint.h>
//#include "boards.h"
#include "et_define.h"
#include "ET1502B.h"
//#if ENABLE_PROTOCOL
//stru_uart_recv uart_recv;
//hsTaskData theSink ;

/*********************************CMD*******************************/
#define  CMD_NULL    0x00
#define  CMD_GET_MTU       0x01
#define  CMD_SEND_AG_STATUS  0x02
#define  CMD_GET_VERSION   0x03
#define  CMD_SET_CLOCK     0x04
#define  CMD_GET_CLOCK     0x05
#define  CMD_SET_ALARM     0x06
#define  CMD_GET_ALARM     0x07
#define  CMD_SET_AUTO_POWEROFF 0x08
#define  CMD_GET_AUTO_POWEROFF 0x09
#define  CMD_SET_MOTOR  0x0a
#define  CMD_GET_MOTOR 0x0b
#define  CMD_GET_STEP_NUMBER 0x0c
#define  CMD_COS_DATA  0x0d
#define  CMD_RESET_STEP_NUMBER 0x0e
#define  CMD_GET_JUMP_NUMBER 0x0f
#define  CMD_RESET_JUMP_NUMBER 0x10
#define  CMD_GET_SITUP_NUMBER 0x11
#define  CMD_RESET_SITUP_NUMBER 0x12
#define  CMD_BATTERY_VOLTAGE 0x13
#define  CMD_SEND_MESSAGE 0x14
#define  CMD_SEND_INCOMING_CALL 0x15
#define  CMD_AS_RJ_INCOMING_CALL 0x16
#define  CMD_SEND_MAIL_PROMPT 0x17
#define  CMD_SEND_WECHAT_PROMPT 0x18
#define  CMD_SEND_INCOMING_CALL_STATUS 0x19
#define  CMD_SEND_MESSAGE_PROMPT 0x1a
#define  CMD_SET_ALERT_TIME 0x1b
#define  CMD_FIND_AG 0x1c//CMD_GET_ALERT_TIME 0x1c
#define  CMD_FIND_DEVICE  0x1d
#define  CMD_SET_AUTOSEND_STEP_NUM 0x1e
#define  CMD_CTR_COS_POWER  0x1f
#define  CMD_GET_COS_STATUS  0x20
#define  CMD_COS_AG_TRADE_COMPLETE  0x21

#define CMD_SEND_SPORT_DATA_TO_APP	0x22
//#define CMD_REQUIRE_DATA_FROM_APP	0x23
#define CMD_SEND_SLEEP_DATA_TO_APP	0x23

#define CMD_USR_INFO				0x24
#define CMD_SEND_REALTIMER_DATA 0x25
#define CMD_GET_INFO  0x26
#define CMD_SEND_HISTRY_EVRYDAY_SPORT_DATA_TO_APP  0x27
#define CMD_EVERYDAY_TOTAL_SLEEPDATA 0x29

#define CMD_ERASE_FLASH 0x30
#define CMD_WRITE_FLASH 0x31
#define CMD_READ_FLASH 0x32


#define  CMD_DISPLAY_AG_NAME 0xa0
#define  CMD_TEST_SENSOR 0xa7
#define  CMD_TEST_LCD 0xa8
#define  CMD_GET_DEVICE_STATUS 0xfb
#define  CMD_TEST_AG_COS_CHANNEL 0xfc
#define  CMD_BT_LOOPBACK_TEST 0xfd
#define  CMD_STATUS 0xfe
#define  CMD_DFU_UPDATED 0xff

/******************************CMD STATUS***********************/
#define CMD_STATUS_SUCCESS 0x00
#define CMD_STATUS_FAIL  0x01
#define CMD_STATUS_CHECK_ERROR 0x02
#define CMD_STATUS_DATALENGTH_FAIL 0x03
#define CMD_STATUS_ALARM_OVER 0x04
/**********************************************************************/
/****************************PROTOCOL VERSION************************************/
//#define PROTOCOL_VERSION_V1 0x01
//#define PROTOCOL_VERSION_V2 0x02
//#define AG_KEY_PROTOCOL_VERSION PROTOCOL_VERSION_V2
/***************************packet format******************************/
#define ENABLE_STATUS_RETURN 1





/****************************************************************************/

#define CMD_PACKET_MIN_LENGTH 6
#define CMD_STATUS_DATA_NUM 2
#define CMD_PACKET_BASE_LENGTH 6


#define PACKET_COS_MAX_BYTE 410//512

#define PACKET_STEP_MAX_BYTE 410//512
#define PACKET_MARK_FULL 0x00
#define PACKET_MARK_MULTI 0xc0
#define PACKET_MARK_NEXT 0x80
#define PACKET_MARK_COMPLETE 0x40
#define PACKET_MARK_FLAG 0xf0
#define PACKET_L_MSB_FLAG 0x0f
#define PACKET_CMD_OFFSET 0x01
#define PACKET_LENGTH_OFFSET 0x02
#define PACKET_DATA_OFFSET 0x04
#define PACKET_DATA_HEAD 0xAA
#define PACKET_DATA_TAIL 0x55


/************************version  flag***************************************/
#define FLAG_COS_VERSION 0xfe
#define FLAG_FIRMWARE_VERSION 0xfd
#define FLAG_HW_VERSION 0xfc


/************************ALARM***************************************/

//#define ALARM_MAX_NUM 0x03
//#define ALARM_ONE_OFFSET 0x04

#define ALARM_ID_BIT  0x07
#define ALARM_ENABLE_BIT 0x08
#define ALARM_UPDATE_CTR_BIT  0x30

#define ALARM_DATA_UPDATE  0x00
#define ALARM_DATA_DELETE  0x10
#define ALARM_DATA_ADD  0x20


/************************ALERT***************************************/
#define ALERT_MAX_NUM 0x01
#define ALERT_ONE_OFFSET 0x03
#define ALERT_PAUSE_BIT_OFFSET 0x20
#define ALERT_ENABLE_BIT_OFFSET 0x10
//===================================================
#define ENABLE_PROTOCOL_CONTINUE 0
#define BT_SEND_DELAY_MS 30
//===============================================
#define SERVICE_COS 0
#define SERVICE_STEP 1
#define SERVICE_STEP_EN 1

typedef enum
{
  BT_OFF_MODE=0,
  BT_PAIR_MODE,
  BT_CONNECTED_MODE,
  BT_MODE_MAX
}Bluetooth_MODE;
typedef struct
{     
	unsigned cos_protocol_rev_data_ready:1;
	unsigned cos_protocol_send_complete:1;
  unsigned cos_protocol_cos_mode:2;
	unsigned cos_protocol_noused2:28;
	
  unsigned cos_revDataTotalLen:16;
	unsigned cos_revDataLen:16;
	union{
			unsigned char cos_taltol_data[PACKET_COS_MAX_BYTE];
			struct{
				unsigned char cos_head; //all data item number ;
				unsigned char cos_CMD; //one data item size;
				unsigned char cos_length_h; //data crc16 value
				unsigned char cos_length_l;	//the user id num !
				unsigned char cos_part_data[PACKET_COS_MAX_BYTE-4];//eq:if cmd is cmd_ack,so argv[0]=ack_type;
				//unsigned char check;
				//unsigned char tail;
				//unsigned char packet_num;
			}cos_data_str;
     }cos_uart_union;     
}stru_uart_recv;
extern stru_uart_recv uart_recv;

typedef struct
{  
    
	//unsigned protocol_receive_num:8;
	//unsigned protocol_send_num:8;
	//unsigned protocol_send_reach_len:16;	
	//unsigned protocol_send_total_len:16;

	//unsigned protocol_send_total_num:8;
	//unsigned protocol_send_reach_num:8;
	//unsigned protocol_send_CMD:8;
	
	//unsigned protocol_send_data_busy:1;
	//unsigned protocol_send_complete:1;
  unsigned protocol_rev_data_ready:1;
	unsigned protocol_noused2:31;

	
  unsigned revDataTotalLen:16;
	unsigned revDataLen:16;
	union{
			unsigned char taltol_data[PACKET_STEP_MAX_BYTE];
			struct{
				unsigned char head; //all data item number ;
				unsigned char CMD; //one data item size;
				unsigned char length_h; //data crc16 value
				unsigned char length_l;	//the user id num !
				unsigned char part_data[PACKET_STEP_MAX_BYTE-4];//eq:if cmd is cmd_ack,so argv[0]=ack_type;
				//unsigned char check;
				//unsigned char tail;
				//unsigned char packet_num;
			}data_str;
     }uart_union;
}stru_new_recv;
extern stru_new_recv new_recv;

/*
typedef struct
{
    //=====word 1===
	unsigned                year:16;
	unsigned                month:8;
	unsigned                day:8;
	//=====word 2===
    unsigned                week:8;	
	unsigned                hour:8;
	unsigned                minute:8;	
	unsigned                second:8;	
}clock_str;*/
	/*
typedef struct
{
	UINT16                AutoSwitchOffTime_s;
}Time_str;
*/
/*
typedef struct
{
    unsigned                un_used_9:4;
    unsigned                alarm_enable:1;	
	unsigned                alarm_ID:3;
    unsigned                alarm_week:8;	
	unsigned                alarm_hour:8;
	unsigned                alarm_minute:8;	
	
}alarm_str;
*/
/*
typedef struct
{
    unsigned                un_used_10:26;
	unsigned                alarm_pause:1;
	unsigned                alert_enable:1;
	unsigned                alert_count:4;
	unsigned		        alert_hour:8;
	unsigned                alert_minute:8;	
	unsigned                alert_alert_interval:16;	
}alert_str;
*/

//typedef struct
//{
   // alarm_str alarm[ALARM_MAX_NUM];
	//alert_str alert[ALERT_MAX_NUM];
	//clock_str clock;
	//Time_str time_ctr;
//}clk_alarm_str;
/*
typedef struct
{
	UINT32                pedometer_num;
	UINT32                jump_num;
	UINT32                situp_num;
	
}pedometer_str;*/
/*
typedef struct
{
	UINT16                gsensor_x;
	UINT16                gsensor_y;
	UINT16                gsensor_z;	
}Gsensor_str;
*/

typedef struct
{
// unsigned  SPP_MTU:16;
// unsigned  BLE_MTU:16;
   unsigned char BLE_MTU;
}mtu_stru;

typedef struct
{
	Bluetooth_MODE BT_mode;
	mtu_stru mtu;
}Bluetooth_stru;


typedef struct
{

  //clk_alarm_str          clk_alarm;
	//pedometer_str          pedometer;
	//Gsensor_str            G_sensor;	
	Bluetooth_stru BT_ctr;
	
	unsigned BLE_connected:1;
	unsigned notify_ok:1;
	unsigned Enable_Moter:1;
	//unsigned Enable_Send_stepNum:1;//unsigned COS_data_ready:1;
	unsigned AutoSend_HistoricalDataFlg:1;   /*历史每天记录累计数据自动发送标志*/
	unsigned AutoSend_HistoricalDataFlg_end:1;
	unsigned AutoSend_RealtimeDataFlg:1;  /*实时数据自动发送标志*/
	unsigned AutoSend_TadayDataFlg:1;      /*当天各个片段数据发送标志*/	
	unsigned AutoSend_SleepDataFlg:1;
	unsigned Step_Flag:1; //有STEP 标志
	unsigned Step_RealtimeData_send_ok_flag:2;
	unsigned SleepDataFlg_over:1; /*发送睡眠数据开始SET 1 最后一个包SET 0*/
	unsigned SleepDataFlg_end:1;	/*end*/
	unsigned SportDataFlg_over:1; /*发送每天历史数据开始SET 1 最后一个包SET 0*/
	unsigned SportDataFlg_end:1; /*END*/
	unsigned AutoSend_EveryDataTotalSleepRecordFlg:1;
  unsigned EveryDataTotalSleepRecord_over:1;
  unsigned EveryDataTotalSleepRecord_end:1;
	unsigned unused:14;	
} hsTaskData;
extern hsTaskData theSink ;

extern void clr_cos_mode(void);
extern unsigned char Get_Cos_mode(void);
extern void Protocol_set_MTU(UINT8 t_mtu);
extern void Protocol_set_TX_complete(void);
//extern void Protocol_data_to_AG_continue(void);
//extern void Protocol_data_to_AG(UINT8 temp_cmd,UINT8 *buff,UINT16 temp_length);
extern void Protocol_COSData_2_AG(UINT8 *buff,UINT16 temp_length);
//extern void Protocol_GetPedometerNum(void);
//extern void Protocol_Packet_Status(unsigned char * data_prt,UINT8 temp_flag);

//void Protocol_SendVersion_To_AG(UINT8 * data_prt,UINT16 data_length);
extern void Protocol_data_init(void);
extern void Protocol_data_manage(UINT8 * data_prt,UINT16 packet_length);
extern void protocol_data_can_send(void);
extern UINT8 Protocol_check_BT_connected(void);

extern void Protocol_set_BT_connected(void);
extern void Protocol_set_BT_Disconnected(void);


extern void Protocol_send_sport_data_to_ag(UINT8 * data_prt,UINT16 packet_length,UINT8 ser_flag);
extern void Protocol_send_sleep_data_to_ag(UINT8 * data_prt,UINT16 packet_length,UINT8 ser_flag);
extern void Protocol_send_everyDayTotalSleepData_to_ag(UINT8 * data_prt,UINT16 packet_length,UINT8 ser_flag);
extern void auto_send_sport_Live_to_app(UINT8 * data_prt,UINT16 packet_length,UINT8 ser_flag);
extern void Protocol_send_histry_sport_data_to_ag(UINT8 * data_prt,UINT16 packet_length,UINT8 ser_flag);

extern void Protocol_send_ack_to_ag_by_app_require(UINT8 * data_prt,UINT16 packet_length,UINT8 ser_flag);
//extern void Protocol_send_ack_to_ag_by_usr_info(UINT8 * data_prt,UINT16 packet_length,UINT8 ser_flag);
extern void Protocol_send_usr_info(UINT8 * data_prt,UINT16 packet_length,UINT8 ser_flag);
extern void Protocol_CMD_FIND_AG(UINT8 ser_flag);
extern void Protocol_set_notify(unsigned char val);
//==============================================================
void newService_protocol_data_can_send(void);

void newService_Protocol_data_manage(UINT8 * data_prt,UINT16 packet_length);


#endif

//#endif
