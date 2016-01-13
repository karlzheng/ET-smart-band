#include <stdio.h>
#include <string.h>
#include "uartProtocol.h"
#include "nrf_gpio.h"
#include "ET1502B.h"
#include "etSimple_uart.h"
#include "btprotocol.h"
#include "etMultiPageDisplay.h"
#include "et_battery.h"
#include "etSpi.h"
#include "et_debug.h"
#include "et_flash_offset.h"
//#include "app_util_platform.h"
#if(ENABLE_COS)
#define MAX_DATA_LEN      300
#define TIMEOUT_RESEND    4000 //4s
#define RESEND_TIMER      3
//======================================
#define CMD_STATUS_OK 0x00
#define CMD_STATUS_ERROR 0x01
#define CMD_STATUS_OVERTIMER 0x02

#define CMD_RESEND_DELAY 5
//======================================
#define Z8_POWER_CLOSE NRF_GPIO->OUTSET = Z8_PW_PIN_MASK
#define Z8_POWER_OPEN  NRF_GPIO->OUTCLR = Z8_PW_PIN_MASK

typedef struct _uart_resend{
	unsigned char tag;
	unsigned char dat;
}uart_resend;


struct _uart_recv uart_recv_buff;
struct _COS_resend_Str COS_ReSend_buff;
//unsigned int cos_sta_counter = 100;// 1s
//unsigned char cos_sta_flg = 0;

//static unsigned int  NonAckResendCounter = 0;
static unsigned char NonAckResendFlg = RESEND_FLG_CLEAR;
static unsigned char  NonAckResendDelay = CMD_RESEND_DELAY;
//static unsigned char NonAckResendNum = 0;
static unsigned char GetKeyID=0;
static unsigned char SetResetFlag=0;
static unsigned char SetCosReady=0;
static unsigned char BT_status=0x0f;
static unsigned char COS_status=0xff;

static uart_resend UartResend = {ANS_WORK_STA_TAG, 0xF0};

//unsigned char cos_ver[20] = {0};
//unsigned char cos_ver_len = 0;

//void init_z8_vdd_pin(void)
//{
//	NRF_GPIO->DIRSET = Z8_PW_PIN_MASK;
//	Z8_POWER_CLOSE;
//}

void z8_Power_ctr(unsigned char onoff)
{
	switch(onoff)
	{
		case POWER_ON:
      NRF_GPIO->DIRSET = Z8_PW_PIN_MASK;
			Z8_POWER_OPEN;  
 #if DEBUG_UART_EN    
    //DbgPrintf("tag=%d,check_resend=%d,Delay=%d\r\n",UartResend.tag,NonAckResendFlg,NonAckResendDelay);
      //DbgPrintf("retag=%x%x%x\r\n",UartResend.tag,NonAckResendFlg,NonAckResendDelay);
  //DbgPrintf("Z8_POWER_OPEN\r\n");
#endif     
			break;
		case POWER_OFF:
		default:
      NRF_GPIO->DIRSET = Z8_PW_PIN_MASK;
			Z8_POWER_CLOSE;
#if DEBUG_UART_EN    
    //DbgPrintf("tag=%d,check_resend=%d,Delay=%d\r\n",UartResend.tag,NonAckResendFlg,NonAckResendDelay);
      //DbgPrintf("retag=%x%x%x\r\n",UartResend.tag,NonAckResendFlg,NonAckResendDelay);
  //DbgPrintf("POWER_OFF\r\n");
#endif    
			break;
	}
}

void cos_clear_buff(void)
{
	//memset((void *)uart_recv_buff.data, 0x00, sizeof(uart_recv_buff.data));
	//uart_recv_buff.frame_flg = 0;
	uart_recv_buff.total_len = 0;
	//uart_recv_buff.check = 0x00;
	uart_recv_buff.head = 0x00;
	//uart_recv_buff.tag = 0x00;
	//uart_recv_buff.tail = 0x00;
	uart_recv_buff.len_h = 0;
	uart_recv_buff.len_l = 0;
}

static void save_cos_data(uint8_t *str, unsigned short len)
{
  unsigned short temp_len=0;
  COS_ReSend_buff.resend_total_len=len;
	for(temp_len=0;temp_len<len;temp_len++)
	{
	  if(temp_len<(FRAME_DATA_MAX+6))
	  {
		COS_ReSend_buff.resend_data[temp_len]=str[temp_len];
	  }
	}

	//memcpy(COS_ReSend_buff.resend_data,str,len);

}
static void ResendBTdata_to_Cos(void)
{
	simple_uart_putstringbuff(COS_ReSend_buff.resend_data,COS_ReSend_buff.resend_total_len);
}
void ResendCosData_to_AG(void)
{
  Protocol_COSData_2_AG(COS_ReSend_buff.resend_data,COS_ReSend_buff.resend_total_len);
}
void cos_send_string(uint8_t *str, unsigned short len)
{  
  save_cos_data(str,len);
	set_uart_resend_tag(str[1],0);
	//simple_uart_putstringbuff(COS_ReSend_buff.resend_data,COS_ReSend_buff.resend_total_len);
  simple_uart_putstringbuff(str,len);
}

void get_cos_ver_req(void)
{
	unsigned char gSendData[10] = {0};

	gSendData[0] = FRAME_HEAD;
	gSendData[1] = GET_COS_VER_ANS_TAG;
	gSendData[2] = 0x00;
	gSendData[3] = 0x00;
	gSendData[4] = 0x4E;
	gSendData[5] = FRAME_END;

	//DEBUG (("Send Get COS Version Request Frame To COS\n"));
	cos_send_string(gSendData, 6);
	set_uart_resend_tag(GET_COS_VER_ANS_TAG, 0);
}

void get_bt_name_req(void)
{
	unsigned char gSendData[10] = {0};


	gSendData[0] = FRAME_HEAD;
	gSendData[1] = GET_BT_NAME_ANS_TAG;
	gSendData[2] = 0x00;
	gSendData[3] = 0x00;
	gSendData[4] = 0x4F;
	gSendData[5] = FRAME_END;

	//DEBUG (("Send Get BlueTooth Name Request Frame To COS\n"));
	cos_send_string(gSendData, 6);
	set_uart_resend_tag(GET_BT_NAME_ANS_TAG, 0);

}

static unsigned char get_cos_data_timeout(unsigned char *buff, unsigned int ms)
{
	unsigned char st = 0;
	//unsigned int time = GetTickCount();
	unsigned int time = 0xfff<<ms;

	//while((GetTickCount() - time) <  ms)
	while(time--)
	{
		st = UartRecv(buff);
		if (st)
		{
			//simple_uart_put(*buff);
			break;
		}
	}
	return st;
}

static unsigned char uart_recv_data(struct _uart_recv *recv_buff)
{
	unsigned char st = 0;
	//unsigned char tmp = 0;
	unsigned short i = 0;
	unsigned short len = 0;
	unsigned int delay_time = 8;


	st = UartRecv(&recv_buff->head);
	if (recv_buff->head != FRAME_HEAD)
	{
		return 2;
	}
	st = get_cos_data_timeout(&recv_buff->tag, delay_time);//tag
	if (st == 0)//timeout
		  goto END1;
	st = get_cos_data_timeout(&recv_buff->len_h, delay_time);//len_h
	if (st == 0)//timeout
		  goto END1;
	st = get_cos_data_timeout(&recv_buff->len_l, delay_time);//len_l
	if (st == 0)//timeout
		  goto END1;

	len = (unsigned short)(recv_buff->len_h&0x0f);
	len = len<<8;
	len += (unsigned short)recv_buff->len_l;

	recv_buff->data_len = (unsigned short)len;

	if (len > MAX_DATA_LEN)
	{
		goto END1;
	}

	for (i = 0; i < len; i++)
	{
			st = get_cos_data_timeout(&recv_buff->data[i], delay_time);
		if (st == 0)
			break;
	}
	if (st == 0)
		goto END1;
	st = get_cos_data_timeout(&recv_buff->check, delay_time);//check
	if (st == 0)//timeout
		goto END1;
	st = get_cos_data_timeout(&recv_buff->tail, delay_time);//tail
	if (st == 0)//timeout
		goto END1;
	return 0;
END1:
	return 1;
}


static unsigned char check_frame(void)
{
	unsigned char st = 1;
	unsigned char check = 0;
	unsigned int len = uart_recv_buff.data_len;

	if (uart_recv_buff.tail != FRAME_END)
		return st;

	check = uart_recv_buff.tag + uart_recv_buff.len_h + uart_recv_buff.len_l;
	while(len)
	{
		check += uart_recv_buff.data[--len];
	}
	check = ~check;
	if (check != uart_recv_buff.check)
		return st;
	return 0;
}

static void packed_data_frame(unsigned char *buff, unsigned short *len, unsigned char tag, unsigned char len_h, unsigned char len_l, unsigned char *sdata)
{
	unsigned int slen = 0;
	unsigned int i = 0, temp = 0;
	unsigned char check = 0;

	buff[slen++] = FRAME_HEAD;
	buff[slen] = tag;
	check += buff[slen++];
	buff[slen] = len_h;
	check += buff[slen++];
	buff[slen] = len_l;
	check += buff[slen++];

	temp = (unsigned int)(len_h&0x0f);
	temp = temp<<8;
	temp += len_l;
	if(temp>0)
	{
		for (i = 0; i < temp; i++)
		{
			buff[slen] = sdata[i];
			check += buff[slen++];
		}
	}
	buff[slen++] = ~check;
	buff[slen++] = FRAME_END;
	*len = slen;
}

void  send_key_val_to_cos(unsigned char key_val)
{
	unsigned short slen = 0;
	unsigned char buff[50] = {0};


	packed_data_frame(buff,  &slen, REQ_KEY_STA_TAG, 0, 1, &key_val);
	cos_send_string(buff, slen);
	set_uart_resend_tag(REQ_KEY_STA_TAG, key_val);
}


static void send_ack(unsigned char cmd,unsigned char status)
{
	unsigned char buff[10] = {0};
	unsigned short len = 0;
  unsigned char buff2[2];
  buff2[0]= cmd;
  buff2[1]= status;

	/*buff[0] = FRAME_HEAD;
	buff[1] = ACK_TAG;
	buff[2] = 0x00;
	buff[3] = 0x02;
	buff[4] = 0xA3;
	buff[5] = FRAME_END;
cos_send_string(buff, 6);
*/

	packed_data_frame(buff, &len, (unsigned char)ACK_TAG, 0, 2, buff2);
	cos_send_string(buff, len);
	//set_uart_resend_tag(ACK_TAG, 0);
}
void send_key_to_cos(unsigned char T_KEY)
{
	unsigned char buff[10] = {0};
	unsigned short len = 0;
  unsigned char buff2[1];
  buff2[0]= T_KEY;


	packed_data_frame(buff, &len, (unsigned char)ANS_KEY_STA_TAG, 0, 1, buff2);
	cos_send_string(buff, len);
	//set_uart_resend_tag(ACK_TAG, 0);
}

static void send_error(void)
{
	unsigned char buff[20] = {0};
	//unsigned short len = 0;

	buff[0] = FRAME_HEAD;
	buff[1] = FAIL_TAG;
	buff[2] = 0x00;
	buff[3] = 0x00;
	buff[4] = 0x83;
	buff[5] = FRAME_END;
	//packed_data_frame(buff, &len, FAIL_TAG, 0, 0, 0);
	cos_send_string(buff, 6);
//	set_uart_resend_tag(FAIL_TAG, 0);
}
static void send_timeout(void)
{
	unsigned char buff[50] = {0};
	unsigned short len = 0;


	packed_data_frame(buff, &len, TIMEOUT_STA_TAG, 0, 0, 0);
	cos_send_string(buff, len);
//	set_uart_resend_tag(TIMEOUT_STA_TAG, 0);
}
unsigned char get_work_state(void)
{
  unsigned char ttemp=0;
  if(Protocol_check_BT_connected())
  {
  ttemp = 0x03;
  }
  else
  {
  ttemp =0x01;
  }
  
  if((USB_detect()==1))
  {    
   ttemp +=0xf0;    
  } 
  else
  {
    ttemp &=0x0f;
  }
  return ttemp;
}
void ans_work_state_handle(void)
{
	unsigned char buff[10] = {0};
	unsigned short len = 0;
  unsigned char buff2=0;

  buff2=get_work_state();
 /* if(Protocol_check_BT_connected())
  buff2[0]= 0x03;
  else
  buff2[0]=0x01;
  
  if(USB_detect())
  {
   buff2[0] |=0xf0;
  } 
  */
	packed_data_frame(buff, &len,REQ_WORK_STA_TAG, 0, 1, &buff2);
	cos_send_string(buff, len);
  set_uart_resend_tag(REQ_WORK_STA_TAG, 0);
}

void save_COS_status(unsigned char t_data)
{
  COS_status=t_data;
}
unsigned char Get_COS_status(void)
{
 if((COS_status & 0x0f)==0x04)
 return 1;
 else
 return 0;
}

/*
static void Get_keyID_API(void)
{
	unsigned char buff[10] = {0};
	unsigned short len = 0;
  unsigned char buff2[1];


	packed_data_frame(buff, &len,GET_BT_NAME_ANS_TAG, 0, 0, buff2);
	cos_send_string(buff, len);
  set_uart_resend_tag(GET_BT_NAME_ANS_TAG, 0);
}
*/
static void set_work_state_handle(void)
{

}

static void cos_status_display_handle(void)
{
//	cos_sta_counter = uart_recv_buff.data[0]*10;
//	cos_sta_flg = 1;
	return;
}

//========================================================================================================================

//========================================================================================================================

void etCheck_reset(void)
{
	 if(SetResetFlag==1)
	 {
     SetResetFlag=0;
		 NVIC_SystemReset();
	 }
}

static void etSet_reset(unsigned char t_val)
{
   SetResetFlag=t_val;
}

static void get_bt_name_ans_handle(unsigned char *data,unsigned short len)
{
	  unsigned char keyid[FLASH_GET_LENGTH];
		unsigned short temp_len=0;

    //etRead_KEYID_from_flash(keyid,FLASH_PAGE_LENGTH);
    etSpim1ReadNorFlashStdMode(KEYID_START_ADDR,keyid,FLASH_GET_LENGTH);
     if(len<7)
     return;
		if((keyid[TAG1_ADDR_OFFSET]==MAC_VALID_TAG1) && (keyid[TAG2_ADDR_OFFSET]==MAC_VALID_TAG2))
		{
      for(temp_len=0;temp_len<len;temp_len++)
      {
       	if(data[temp_len]!=keyid[temp_len+MAC_ADDRESS_ADDR_OFFSET])
       	{
            keyid[TAG1_ADDR_OFFSET]=MAC_VALID_TAG1;
            keyid[TAG2_ADDR_OFFSET]=MAC_VALID_TAG2;
            keyid[KEYID_LENGHT_ADDR_OFFSET]=(len-MAC_ADDRESS_LENGTH) & 0xff;
            memcpy(keyid+MAC_ADDRESS_ADDR_OFFSET,data,len);
       	    etWrite_KEYID_to_flash(KEYID_START_ADDR,keyid,FLASH_GET_LENGTH);
          
						etSet_reset(1);
        
            return;
			  }
			}      
		}
		else
		{
            keyid[TAG1_ADDR_OFFSET]=MAC_VALID_TAG1;
            keyid[TAG2_ADDR_OFFSET]=MAC_VALID_TAG2;
            keyid[KEYID_LENGHT_ADDR_OFFSET]=(len-MAC_ADDRESS_LENGTH) & 0xff;
            memcpy(keyid+MAC_ADDRESS_ADDR_OFFSET,data,len);
       	    etWrite_KEYID_to_flash(KEYID_START_ADDR,keyid,FLASH_GET_LENGTH);
      
            etSet_reset(1);    
		}
}

static void save_cos_ver_ans_handle(unsigned char *data,unsigned short len)
{
	  unsigned char keyid[FLASH_GET_LENGTH];
		unsigned short temp_len=0;
  
    etSpim1ReadNorFlashStdMode(KEYID_START_ADDR,keyid,FLASH_GET_LENGTH);
  
		if((keyid[COS_VERSION_TAG1_ADDR_OFFSET]==MAC_VALID_TAG1) && (keyid[COS_VERSION_TAG2_ADDR_OFFSET]==MAC_VALID_TAG2))
		{
      for(temp_len=0;temp_len<len;temp_len++)
      {
       	if(data[temp_len]!=keyid[temp_len+COS_VERSION_DATA_ADDR_OFFSET])
       	{
            keyid[COS_VERSION_TAG1_ADDR_OFFSET]=MAC_VALID_TAG1;
            keyid[COS_VERSION_TAG2_ADDR_OFFSET]=MAC_VALID_TAG2;
            keyid[COS_VERSION_LENGHT_ADDR_OFFSET]=(len & 0xff);
            memcpy(keyid+COS_VERSION_DATA_ADDR_OFFSET,data,len);
       	    etWrite_KEYID_to_flash(KEYID_START_ADDR,keyid,FLASH_GET_LENGTH); 
            return;
			  }
			}      
		}
		else
		{
            keyid[COS_VERSION_TAG1_ADDR_OFFSET]=MAC_VALID_TAG1;
            keyid[COS_VERSION_TAG2_ADDR_OFFSET]=MAC_VALID_TAG2;
            keyid[COS_VERSION_LENGHT_ADDR_OFFSET]=(len & 0xff);
            memcpy(keyid+COS_VERSION_DATA_ADDR_OFFSET,data,len);
       	    etWrite_KEYID_to_flash(KEYID_START_ADDR,keyid,FLASH_GET_LENGTH);
		}
}






static void uart_resend_to_cos(void)
{
	unsigned char tag = UartResend.tag;
 #if DEBUG_UART_EN    
    //DbgPrintf("tag=%d,check_resend=%d,Delay=%d\r\n",UartResend.tag,NonAckResendFlg,NonAckResendDelay);
      //DbgPrintf("retag=%x%x%x\r\n",UartResend.tag,NonAckResendFlg,NonAckResendDelay);
  //DbgPrintf("retag=%x\r\n",UartResend.tag);
#endif 
	switch(tag)
	{
		case ANS_KEY_STA_TAG:
			//DEBUG(("ReSend Key Value to Z8 \r\n"));
			send_key_val_to_cos(UartResend.dat);
			break;
    case REQ_WORK_STA_TAG:
		case ANS_WORK_STA_TAG:
			//DEBUG(("ReSend Work Status to Z8 \r\n"));
			ans_work_state_handle();
			break;
		case GET_BT_NAME_ANS_TAG:
			//DEBUG(("ReSend Get BT Name Ans to Z8 \r\n"));
			get_bt_name_req();
			break;
		case GET_COS_VER_ANS_TAG:
			//DEBUG(("ReSend Get COS Version to Z8 \r\n"));
			get_cos_ver_req();
			break;
		case ACK_TAG:
			//DEBUG(("ReSend ACK to Z8 \r\n"));
			//send_ack();
		case RESEND_TAG:
			break;
		case FAIL_TAG:
			//DEBUG(("ReSend Fail to Z8 \r\n"));
			send_error();
			break;
		case TIMEOUT_STA_TAG:
			//DEBUG(("ReSend Timeout to Z8 \r\n"));
			send_timeout();
			break;
		case DATA_FRAME_TAG:
			ResendBTdata_to_Cos();
			break;
	}
}

void set_uart_resend_tag(unsigned char tag, unsigned char dat)
{
	UartResend.tag = tag;
	UartResend.dat = dat;
	NonAckResendFlg = RESEND_FLG_SET;
  NonAckResendDelay = CMD_RESEND_DELAY;
	//NonAckResendCounter = GetTickCount();//
}
static void clr_uart_resend_tag(unsigned char t_tag)
{
  if(UartResend.tag==t_tag)
  {
   NonAckResendFlg = RESEND_FLG_CLEAR;
  }
}
void check_uart_resend_tag(void)
{
 
  if(NonAckResendFlg==RESEND_FLG_SET)
  {
     
    if(NonAckResendDelay>0)
    {
   
      NonAckResendDelay--;
      if(NonAckResendDelay==0)
      {        
        if(Uart_open_check())
        {
        uart_resend_to_cos();
        }
      }
    }
  }
}

//void NonAckResend(void)
//{	
//}


static void go_to_command(void)
{
	unsigned char cmd = uart_recv_buff.tag;

	 switch(cmd)
	 {
		case DATA_FRAME_TAG:
			//CosToFreeAGData_buff();
			//data_frame_handle();   
#if DEBUG_UART_EN
     //DbgPrintf("DATA_to_AG:%d\r\n",uart_recv_buff.data_len);
     /*for(unsigned char i=0;i<uart_recv_buff.data_len;i++)
     {
      DbgPrintf("%x ",uart_recv_buff.data[i]);
     }
     DbgPrintf("\r\n");*/
#endif	
      save_cos_data(uart_recv_buff.data,uart_recv_buff.data_len);		
      Protocol_COSData_2_AG(uart_recv_buff.data,uart_recv_buff.data_len);
			break;
		case LCD_DISPLAY_TAG:
			//lcd_display_handle(uart_recv_buff.data);
#if DEBUG_UART_EN
    // DbgPrintf("LCD_DISPLAY_TAG:%d\r\n",uart_recv_buff.data_len);
    // for(unsigned char i=0;i<uart_recv_buff.data_len;i++)
    // {
    //  DbgPrintf("%x ",uart_recv_buff.data[i]);
    // }
    // DbgPrintf("\r\n");
#endif  
      lcd_display_handle(uart_recv_buff.data+2,uart_recv_buff.data_len-2);
  
			break;
		case LCD_DISPLAY_DATA_COMPLETE:
			lcd_display_complete_handle();
 //#if DEBUG_UART_EN
 //    DbgPrintf("LCD_DISPLAY_DATA_COMPLETE\r\n");
//#endif 
		   	 break;
		case LCD_CHAGELINE_TAG:
			lcd_changeline_handle();
			break;
		case LCD_CLEARDISPLAY_TAG:
			lcd_clear_handle();
			break;
		case REQ_KEY_STA_TAG:
			//key_state_handle();
			break;
		case REQ_WORK_STA_TAG:
			//ans_work_state_handle();
			save_COS_status(uart_recv_buff.data[0]);
			break;
		case ANS_WORK_STA_TAG:
			set_work_state_handle();
			break;
		case Z8_STATUS_TO_DISPLAY:
			cos_status_display_handle();
			break;
		case READ_PAIRLIST_TAG:
			break;
		case DELETE_PAIRLIST_TAG:
			break;
		case ANS_PAIRLIST_TAG:
		case RAND_PSW_TAG:
		case REQ_RAND_PSW_TAG:
			break;
		case GET_BT_NAME_ANS_TAG:
			get_bt_name_ans_handle(uart_recv_buff.data,uart_recv_buff.data_len);
			break;
		case GET_COS_VER_ANS_TAG:
			save_cos_ver_ans_handle(uart_recv_buff.data,uart_recv_buff.data_len);
			break;
		case Z8_DEBUG_TAG:/*z8 debug*/
			break;

    case ACK_TAG:
      if((uart_recv_buff.data[1]==CMD_STATUS_ERROR)||(uart_recv_buff.data[1]==CMD_STATUS_OVERTIMER))
      {
         UartResend.tag=uart_recv_buff.data[0];
         uart_resend_to_cos();
      }        
      break;
		case RESEND_TAG:
            //DEBUG(("\r\n=======================================================Z8 data fail==========================================\r\n"));
            uart_resend_to_cos();
            break;
		case FAIL_TAG:
            //DEBUG(("\r\n=======================================================Z8 Get data fail==========================================\r\n"));
            uart_resend_to_cos();
            break;
		case TIMEOUT_STA_TAG:
            //DEBUG(("\r\n=======================================================Z8 Get data overtime==========================================\r\n"));
            uart_resend_to_cos();
            break;
		default:
			break;
	 }
}
static void Set_Cos_Ready(void)
{
  SetCosReady=1;
}
void Clr_Cos_Ready(void)
{
  SetCosReady=0;
}
unsigned char Get_Cos_Ready(void)
{
  return SetCosReady;
}
void SetToGetKEYID(void)
{
  GetKeyID=1;
}
void clr_getKEYID(void)
{
  GetKeyID=0;
}
static void Get_KEYID_from_COS(void)
{

    if((Uart_open_check())&&(Get_Cos_Ready())&&(GetKeyID==1))
    {
        clr_getKEYID();
				//Get_keyID_API();
        get_bt_name_req();
    } 

}
static void Send_BT_status_to_COS(void)
{  
  if(get_work_state()!=BT_status)
  {
    if((Uart_open_check())&&(Get_Cos_Ready()))
    {
      if(USB_detect()==1)
      {
    #if DEBUG_UART_EN 
       //DbgPrintf_COS_log("USB exist\r\n");    
    #endif        
      } 
      else
      {
     #if DEBUG_UART_EN 
       // DbgPrintf_COS_log("USBnotexist\r\n");    
    #endif 
      }         
     ans_work_state_handle();		 
     BT_status=get_work_state();
      
   
    }    
  }
}
static void ParseRecvUartData(void)
{
	unsigned char st = 0;
	//unsigned char tag = 0xff;

	st = uart_recv_data(&uart_recv_buff);
	if (!st)//ok
	{
    Set_Cos_Ready();
		st = check_frame();
		if (!st)// check ok
		{
			//tag = uart_recv_buff.tag;
			if ((uart_recv_buff.tag != ACK_TAG)&&(uart_recv_buff.tag !=Z8_DEBUG_TAG)&&(uart_recv_buff.tag !=TIMEOUT_STA_TAG)&&(uart_recv_buff.tag !=FAIL_TAG)&&(uart_recv_buff.tag !=RESEND_TAG))//no ack frame,first to send ack
			{
				send_ack(uart_recv_buff.tag,CMD_STATUS_OK);
			}
      else
      {
        if((uart_recv_buff.tag == ACK_TAG)&&(uart_recv_buff.data[1]==CMD_STATUS_OK))
        {
         clr_uart_resend_tag(uart_recv_buff.data[0]);
        }
      }
		
			go_to_command();
			cos_clear_buff();
			//return tag;
      return;
		}
		else//send error frame
    {
			//send_error(uart_recv_buff.tag,CMD_STATUS_ERROR);
    send_ack(uart_recv_buff.tag,CMD_STATUS_ERROR);
    }
	}
	//else if (st == 2)
	//	return tag;
	cos_clear_buff();
	//return tag;
}

void uart_data_handle(void)
{
      ParseRecvUartData();
      Send_BT_status_to_COS();
      Get_KEYID_from_COS();

}
#endif

