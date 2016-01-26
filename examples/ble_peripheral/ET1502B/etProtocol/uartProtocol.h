#ifndef _UART_PROTOCOL_H_
#define _UART_PROTOCOL_H_
#include"ET1502B.h"
#include "btProtocol.h"
#if(ENABLE_COS)
//====================
#define POWER_ON  1
#define POWER_OFF  0
//=======================

#define FRAME_DATA_MAX   300

#define RESEND_FLG_SET     0x01
#define RESEND_FLG_CLEAR   0x00

#define FRAME_HEAD   0xAA
#define FRAME_END    0x55
/*tag*/
/*LCD TAG*/
#define LCD_DISPLAY_TAG           0x0F
#define LCD_CHAGELINE_TAG         0x1F
#define LCD_CLEARDISPLAY_TAG      0x2F
#define LCD_DISPLAY_DATA_COMPLETE 0xEF
/*data frame*/
#define DATA_FRAME_TAG            0x0D
/*key tag*/
#define REQ_KEY_STA_TAG           0x0B
#define ANS_KEY_STA_TAG           0x0B
/*control frame*/
#define REQ_WORK_STA_TAG          0x0C
#define ANS_WORK_STA_TAG          0x1C
/*operation end*/
#define OPERATION_END_TAG         0x2C
/*timeout state tag*/
#define TIMEOUT_STA_TAG           0x3C
#define REQ_RECV_FILE_TAG         0x4C
/*ack tag*/
#define ACK_TAG                   0x5C
#define RESEND_TAG                0x6C
#define FAIL_TAG                  0x7C
/*set bluetooth parameter*/
#define READ_PAIRLIST_TAG         0x0E
#define DELETE_PAIRLIST_TAG       0x4e
#define ANS_PAIRLIST_TAG          0x1E
#define RAND_PSW_TAG              0x8C
#define REQ_RAND_PSW_TAG          0x9C
#define Z8_STATUS_TO_DISPLAY      0xCF

#define Z8_DEBUG_TAG              0xDD
#define GET_BT_NAME_ANS_TAG       0xB0
#define GET_COS_VER_ANS_TAG       0xB1

struct _uart_recv{

	unsigned char head;
	unsigned char tag;
	unsigned char len_h;
	unsigned char len_l;
	unsigned char data[FRAME_DATA_MAX];
	unsigned char check;
	unsigned char tail;

	unsigned short total_len;
	unsigned short start_point;
	unsigned short data_len;
	unsigned char frame_flg;
	unsigned char uart_recv_flg;
	unsigned char uart_recv_message_num;
};

struct _COS_resend_Str{
  unsigned short resend_total_len;
	unsigned char resend_data[PACKET_COS_MAX_BYTE];
};

extern struct _uart_recv uart_recv_buff;
extern struct _COS_resend_Str COS_ReSend_buff;
//extern unsigned int cos_sta_counter;
//extern unsigned char cos_sta_flg;
//extern unsigned char cos_ver[20];
//extern unsigned char cos_ver_len;



//void init_z8_vdd_pin(void);
void z8_Power_ctr(unsigned char onoff);
void send_key_to_cos(unsigned char T_KEY);
extern void ResendCosData_to_AG(void);

//void cos_send_string(unsigned char *str, unsigned short len);
void AG_to_Cos_send(unsigned char *str, unsigned short len);
void cos_clear_buff(void);
void Clr_Cos_Ready(void);
unsigned char Get_Cos_Ready(void);
void ans_work_state_handle(void);
void save_COS_status(unsigned char t_data);
unsigned char Get_COS_status(void);

//unsigned char ParseRecvUartData(void);
//void  send_key_val_to_cos(unsigned char key_val);
void display_uart_lsr_val(void);
void get_cos_ver_req(void);
void get_bt_name_req(void);
void set_uart_resend_tag(unsigned char tag, unsigned char dat);
void check_uart_resend_tag(void);
//void NonAckResend(void);
void cos_clear_buff(void);
void Send_BT_status_COS(void);
void etCheck_reset(void);
void SetToGetKEYID(void);
void uart_data_handle(void);
#endif
#endif
