#include "ET1502B.h"
#include "btprotocol.h"
#include "uartProtocol.h"
#include "etkeyboard.h"
//#include "typedefine.h"
//#include "toshiba.h"
//#include "display.h"
#include "et_debug.h"
//#include "et_inc.h"
#include <string.h>
#include "sys_time.h"
#include "data_transfer.h"
#include "etVersion.h"
#include "ble_nus.h"
//#include "app_uart.h"
#include "etSimple_uart.h"
//#include "global.h"

#include "etLcdDisplayApp.h"
#include "Ble_nus.h"
#include "etSpi.h"
#include "etalarm.h"
#include "uartProtocol.h"
#include "nrf_delay.h"
#include "etMotor.h"
#include "et_WDT.h"
#include "et_battery.h"
#include "et_flash_offset.h"
#include "data_manger.h"
#include "gSensor_driver.h"
#include "gSensor.h"

stru_uart_recv uart_recv;
stru_new_recv new_recv;

hsTaskData theSink ;

const UINT8 BT_DISCONNECT_CODE[]={0xaa,0x0d,0x00,0x05,0xFF,0x00,0x00,0x00,0x00,0xEE,0x55};
extern uint32_t bt_send_to_AG(uint8_t* buff,uint16_t temp_length,UINT8 ser_flag);
extern void disconntec_AG(void);
/*
static uint8_t  ble_rev_data_buff[PACKET_COS_MAX_BYTE];
static uint16_t ble_rev_index = 0;
static uint16_t ble_send_index = 0;
*/
static void set_cos_mode(void)
{
  uart_recv.cos_protocol_cos_mode=1;
}
void clr_cos_mode(void)
{
  uart_recv.cos_protocol_cos_mode=0;
}
unsigned char Get_Cos_mode(void)
{
  return uart_recv.cos_protocol_cos_mode;
}
UINT8 Protocol_check_BT_connected(void)
{
    if((theSink.BLE_connected==1))
    {
		  return 1;
    }
	  return 0;

}

void Protocol_set_BT_connected(void)
{
      theSink.BLE_connected=1;
	    theSink.BT_ctr.mtu.BLE_MTU=GATT_MTU_SIZE_DEFAULT-3;
}
void Protocol_set_BT_Disconnected(void)
{
	theSink.BLE_connected=0;
  clr_cos_mode();
  display_clr_COSdata();
  Key_value_reset();
}

void Protocol_set_notify(unsigned char val)
{
  theSink.notify_ok=val;
}
void Protocol_set_TX_complete(void)
{
  uart_recv.cos_protocol_send_complete=1;
}
static void Protocol_Clr_TX_complete(void)
{
  uart_recv.cos_protocol_send_complete=0;
}
static unsigned char Protocol_check_TX_complete(void)
{
  return uart_recv.cos_protocol_send_complete;
}

void Protocol_set_MTU(UINT8 t_mtu)
{
	theSink.BT_ctr.mtu.BLE_MTU=t_mtu-3;//GATT_MTU_SIZE_DEFAULT
}
void Protocol_ctr_reset(void)
{

  uart_recv.cos_revDataLen=0;
  uart_recv.cos_revDataTotalLen=0;
  uart_recv.cos_protocol_rev_data_ready=0;
  new_recv.protocol_rev_data_ready=0;
  theSink.BT_ctr.mtu.BLE_MTU=GATT_MTU_SIZE_DEFAULT-3;
  Protocol_Clr_TX_complete();
  clr_cos_mode();
}

void Protocol_data_init(void)
{
  	 Protocol_ctr_reset();
}


static UINT8 Get_Datacheck(UINT8 *buff,UINT16 temp_length)
{
     UINT16 count=0;
     UINT8 temp_check=0;

     for(count=0;count<temp_length;count++)
     {
          temp_check += buff[count];  
     }
       temp_check =~temp_check & 0xff;
        return temp_check;
}


/***********************************************************************************************************/
static void delay_time_ms(UINT16 ms)
{
	UINT16 i = 0;

	while(ms--)
	{
		for (i = 0; i < 100; i++);
		//{;}
	}
}
/*
static void BT_Get_KeyVersion(unsigned char * prt,unsigned char *str_length)
{
	UINT8 temp=0,temp2=0;

	 prt[temp++]=FLAG_FIRMWARE_VERSION;	
	 for(temp2=0;temp2<strlen(BTversion);temp2++)
		prt[temp++]=BTversion[temp2];
	
	 prt[temp++]=FLAG_HW_VERSION;	 
	 for(temp2=0;temp2<strlen(HWversion);temp2++)
		prt[temp++]=HWversion[temp2];
	
	
	 prt[temp++]=FLAG_COS_VERSION;	 
	 for(temp2=0;temp2<strlen(cosversion);temp2++)
	 {
		prt[temp++]=cosversion[temp2];
	 }
	 *str_length=temp;
}
*/
static void Protocol_packed_data(unsigned char temp_Mark,unsigned char *buff, unsigned char tag, unsigned int temp_length, unsigned char *sdata)
{
	UINT16 slen = 0;
	UINT16 i = 0, temp = 0;
	unsigned char check = 0;
	
	buff[slen++] = PACKET_DATA_HEAD;
	buff[slen] = tag;
	check += buff[slen++];
	buff[slen] =((temp_length>>8)& 0xff);
    buff[slen] |= temp_Mark;
	buff[slen] &=0xff;
	check += buff[slen++];
	buff[slen] =(temp_length & 0xff);
	check += buff[slen++];
	
	temp = temp_length;
	if(temp>0)
	{
		for (i = 0; i < temp; i++)
		{
			buff[slen] = sdata[i];
			check += buff[slen++];
		}
	}
	buff[slen++] = ((~check) & 0xff);/*kevin add 141229*/
	buff[slen++] = PACKET_DATA_TAIL;

	
}

static uint32_t Protocol_send_data_to_AG(unsigned char *buff,unsigned int temp_length,unsigned char flag,UINT8 ser_flag)
{
   UINT8 count=0;
	 uint32_t sendReturn=0;
	 Protocol_Clr_TX_complete();
		do
		 {		 
			sendReturn=bt_send_to_AG(buff,temp_length,ser_flag);
			if(sendReturn!=NRF_SUCCESS)
			{
			nrf_delay_ms(3);
			count++;
#if ENABLE_WDT	 
     et_WDT_feed();
#endif			
			}
		 }
		while((sendReturn!=NRF_SUCCESS)&&(count<BT_SEND_DELAY_MS));	
		if(count>=BT_SEND_DELAY_MS)
			return 0;
		count=0;
		if(flag==1)
		{
     do
     {
       //nrf_delay_ms(1);
       delay_time_ms(1);
			 count++;
#if ENABLE_WDT	 
     et_WDT_feed();
#endif			 
		 }
		 while((Protocol_check_TX_complete()==0)&&(count<BT_SEND_DELAY_MS));
		}
		if(count>=BT_SEND_DELAY_MS)
			return 0;
		else
			return 1;
		//return bt_send_to_AG(buff, temp_length);
}
static UINT8 Protocol_Get_Send_NUM(UINT16 temp_length)
{

	UINT8 count=1;
	UINT16 temp_back_len=temp_length;
	//UINT16 temp_mtu=theSink.rundata->battr.MTU-1;
	UINT16 temp_mtu=theSink.BT_ctr.mtu.BLE_MTU;

	if((theSink.BLE_connected==1))
	{



    temp_mtu=theSink.BT_ctr.mtu.BLE_MTU;

	  if((temp_back_len+CMD_PACKET_MIN_LENGTH)>temp_mtu)
	  {
	   do
	   {
		 temp_back_len =temp_back_len +CMD_PACKET_MIN_LENGTH- temp_mtu;
		 count++;
		 if((temp_back_len+CMD_PACKET_MIN_LENGTH)<=temp_mtu)
			 break;
	   }while(count<254);
	  }
		 
	}
	return count;


}

void Protocol_data_to_AG(UINT8 temp_cmd,UINT8 *buff,UINT16 temp_length,UINT8 ser_flag)
{
		 UINT16 packet_data_len;
		 UINT16 send_length=0;
		 UINT8 count=0,temp=0;//,mutil_count=0;
		 UINT8 mutiPacket_length[3];
		 //uint32_t sendReturn=0;
	
		 static UINT8 prt[GATT_MTU_SIZE_DEFAULT];//PACKET_MAX_BYTE
		
		 if(prt==NULL)
			return; 	
	   Protocol_Clr_TX_complete();
		 count=Protocol_Get_Send_NUM(temp_length);	
		 if(count==1)
		 {
				 send_length=temp_length+CMD_PACKET_MIN_LENGTH;
				 Protocol_packed_data(PACKET_MARK_FULL,prt,temp_cmd,temp_length,buff);
			 Protocol_send_data_to_AG(prt,send_length,0,ser_flag);
			 //uart_recv.protocol_send_reach_len=0;
			 //uart_recv.protocol_send_total_len=0;
			 //uart_recv.protocol_send_CMD=temp_cmd;
			 //uart_recv.protocol_send_data_busy=1;
		 }
		 else
		 {
		 	 theSink.BLE_connected=1;
			 send_length=theSink.BT_ctr.mtu.BLE_MTU;
			 packet_data_len=send_length-CMD_PACKET_MIN_LENGTH;
			 
			 mutiPacket_length[0]=temp_length>>8 & 0xff;
			 mutiPacket_length[1]=temp_length & 0xff;
			 mutiPacket_length[2]=Get_Datacheck(buff,temp_length);
			 Protocol_packed_data(PACKET_MARK_MULTI,prt,temp_cmd,3,mutiPacket_length);
			 Protocol_send_data_to_AG(prt,CMD_PACKET_MIN_LENGTH+3,1,ser_flag);
			 /*do{
			 sendReturn=Protocol_send_data_to_AG(prt,CMD_PACKET_MIN_LENGTH+3);
			 if(sendReturn!=NRF_SUCCESS)
				 nrf_delay_ms(30);
			 }
			 while(sendReturn!=NRF_SUCCESS);
			 */
			 
			 //Protocol_send_data_to_AG(prt,CMD_PACKET_MIN_LENGTH+3);

			 
			 for(temp=0;temp<count-1;temp++)
			 {
				 
				 Protocol_packed_data(PACKET_MARK_NEXT,prt,temp_cmd,packet_data_len,(buff+packet_data_len*temp));
				 Protocol_send_data_to_AG(prt,send_length,1,ser_flag);
				 /*mutil_count++;
				 if(mutil_count>2)
				 {
						nrf_delay_ms(70);
						mutil_count=0;
				 }			 
				 do
				 {			 
					sendReturn=Protocol_send_data_to_AG(prt,send_length);
					if(sendReturn!=NRF_SUCCESS)
					nrf_delay_ms(30);			 
				 }
				 while(sendReturn!=NRF_SUCCESS);	
				 mutil_count++;
	       */
					
			 }
			 if(temp==(count-1))
			 {
				 temp_length=temp_length-(packet_data_len*temp);
				 Protocol_packed_data(PACKET_MARK_COMPLETE,prt,temp_cmd,temp_length,(buff+(packet_data_len*temp)));
				 Protocol_send_data_to_AG(prt,temp_length+CMD_PACKET_MIN_LENGTH,0,ser_flag);
				 /*mutil_count++;
				 if(mutil_count>2)
				 {
						nrf_delay_ms(70);
						mutil_count=0;
				 }			 
				 do
				 {			 
					Protocol_send_data_to_AG(prt,temp_length+CMD_PACKET_MIN_LENGTH);
					if(sendReturn!=NRF_SUCCESS)
					nrf_delay_ms(30);			 
				 }
				 while(sendReturn!=NRF_SUCCESS);
				 */
			 }			 
	
		 }


}
static void Protocol_get_cos_ver_req(UINT8 ser_flag)
{
   //UINT8 prt[strlen(cosversion)+strlen(cosversion)+strlen(cosversion)];
   UINT8 prt[100];
   UINT8 temp=0;//,temp2=0;
   Get_KeyVersion(prt,&temp);
   Protocol_data_to_AG(CMD_GET_VERSION,prt,temp,ser_flag);
   
}
void Protocol_COSData_2_AG(UINT8 *buff,UINT16 temp_length)
{
 if(Protocol_check_BT_connected())
 {
 Protocol_data_to_AG(CMD_COS_DATA,buff,temp_length,SERVICE_COS);
 }
}
static void Protocol_Packet_Status(unsigned char * data_prt,UINT8 temp_flag,UINT8 ser_flag)
{
	
	UINT8 temp_check[CMD_STATUS_DATA_NUM];
	temp_check[0]=data_prt[1];	
	temp_check[1]=temp_flag;


  Protocol_data_to_AG(CMD_STATUS,temp_check,CMD_STATUS_DATA_NUM,ser_flag);
}

static void Protocol_send_MTU_to_AG(UINT8 ser_flag)
{

		UINT8 projectname[]="ET1502B";
		UINT8 temp_MTU[4+sizeof(projectname)];
		UINT8 temp=0;
		temp_MTU[0]=0x02;/*protocol version*/
		temp_MTU[1]=0x03;/**/
		if((theSink.BLE_connected==1))
		{
		temp_MTU[2]=(theSink.BT_ctr.mtu.BLE_MTU>>8)& 0xff;
		temp_MTU[3]=(theSink.BT_ctr.mtu.BLE_MTU & 0xff);
		}

		for(temp=0;temp<(sizeof(projectname)-1);temp++)
		 temp_MTU[4+temp]=projectname[temp];	

		Protocol_data_to_AG(CMD_GET_MTU,temp_MTU,3+sizeof(projectname),ser_flag);	
   
}
/*
static void Protocol_send_DeviceName_to_AG(void)
{

		const char  projectname[]="ET1502A";
		#if 0
		UINT8 temp_MTU[3+sizeof(projectname)];
		UINT8 temp=0;
		temp_MTU[0]=0x02;//protocol version
		temp_MTU[1]=0x03;
		if((theSink.BLE_connected==1))
		{
		temp_MTU[2]=theSink.BT_ctr.mtu.BLE_MTU>>8;
		temp_MTU[2] &=0xff;
		temp_MTU[3]=(theSink.BT_ctr.mtu.BLE_MTU & 0xff);
		}
		else 
		{
		 temp_MTU[2]=theSink.BT_ctr.mtu.SPP_MTU>>8;
		 temp_MTU[2] &=0xff;
		 temp_MTU[3]=(theSink.BT_ctr.mtu.SPP_MTU & 0xff);
		}
		for(temp=0;temp<(sizeof(projectname)-1);temp++)
		 temp_MTU[4+temp]=projectname[temp];	
       #endif
		Protocol_data_to_AG(CMD_SEND_AG_STATUS,(UINT8 *)projectname,strlen(projectname));	
   
}*/
/*
static void Protocol_send_AG_status(unsigned char * data_prt,unsigned short packet_length)
{
    if(data_prt[4]==0x01)
    {
      theSink.AG_status=1;//AG ready/idle status
	}
	else
	{
      theSink.AG_status=0;//AG busy status
	}
    //UINT8 temp_projectName[]="ET1502A";	
	//Protocol_data_to_AG(CMD_GET_PRJ_NAME,temp_projectName,(sizeof(temp_projectName)-1));
}
*/
static void Protocol_SetClock(unsigned char * data_prt,unsigned short packet_length)
{
   UINT32 temp_time=0;
   temp_time =data_prt[4]<<24;
   temp_time |=data_prt[5]<<16;
   temp_time |=data_prt[6]<<8;
   temp_time |=data_prt[7];
   system_time_updated(temp_time);
   system_day_sec_init();
 
}
static void Protocol_GetClock(UINT8 ser_flag)
{
  UINT8 temp_clock[4];
  UINT32 temp_systime=system_sec_get();
  temp_clock[0]=(temp_systime>>24) & 0xff;
  temp_clock[1]=(temp_systime>>16) & 0xff;
  temp_clock[2]=(temp_systime>>8) & 0xff;
  temp_clock[3]=(temp_systime) & 0xff; 
 
  Protocol_data_to_AG(CMD_GET_CLOCK,temp_clock,4,ser_flag);
}
/*
static UINT8 Protocol_Alarm_ADD(unsigned char * data_prt,unsigned short packet_length)
{
  unsigned char alarm_id=0;
  unsigned char count=0;

  if(packet_length>0)
  {

		for(count=0;count<ALARM_ITEM_NUMBER_MAX;count++)
		{
	        if((g_RamAlarmBlock.pdata[count].alarm_week==data_prt[1]) && (g_RamAlarmBlock.pdata[count].alarm_hour==data_prt[2]) &&(g_RamAlarmBlock.pdata[count].alarm_minute==data_prt[3]))
	        {
	           	 if((g_RamAlarmBlock.pdata[count].alarm_enable==1))
	           	 {			   	   
	           	 }
				 else
				 {
	               g_RamAlarmBlock.pdata[count].alarm_enable=1;
				 }
				 return 0;
	        }
		}

        for(count=0;count<ALARM_ITEM_NUMBER_MAX;count++)
        {
           if((g_RamAlarmBlock.pdata[count].alarm_enable==1))
           //if((g_RamAlarmBlock.pdata[count].alarm_week & 0x80)>0)
		   	alarm_id++;
		}
		if(alarm_id>=ALARM_ITEM_NUMBER_MAX)
		{
		   return 1;
		}
        g_RamAlarmBlock.pdata[alarm_id].alarm_enable=1;
		g_RamAlarmBlock.pdata[alarm_id].alarm_ID=alarm_id;
		g_RamAlarmBlock.pdata[alarm_id].alarm_week=data_prt[1];
		g_RamAlarmBlock.pdata[alarm_id].alarm_hour=data_prt[2];
		g_RamAlarmBlock.pdata[alarm_id].alarm_minute=data_prt[3];  

		
  } 
  return 0;
}*/
static void Protocol_SetAlarm(unsigned char * data_prt,unsigned short packet_length,UINT8 ser_flag)
{
	
   UINT8 temp=0;
   UINT8 length=data_prt[3]/ALARM_ITEM_ONE_SIZE;
   unsigned char alarm_id=0;

	  unsigned char keyid[FLASH_GET_LENGTH];

    for(temp=0;temp<length;temp++)
    { 
        if(temp<ALARM_ITEM_NUMBER_MAX)
        {
			  alarm_id=temp;
			  g_RamAlarmBlock.pdata[alarm_id].alarm_enable=((data_prt[4+temp*ALARM_ITEM_ONE_SIZE] & ALARM_ENABLE_BIT)==0)? 0:1;
        if(g_RamAlarmBlock.pdata[alarm_id].alarm_enable==1)
        {
        g_RamAlarmBlock.pdata[alarm_id].alarm_work_today=ALARM_NO_WORKED_TODAY;
        }
        else
        {
        g_RamAlarmBlock.pdata[alarm_id].alarm_work_today=ALARM_WORKED_TODAY;
        }          
			  g_RamAlarmBlock.pdata[alarm_id].alarm_ID=alarm_id;
			  g_RamAlarmBlock.pdata[alarm_id].alarm_week=data_prt[5+temp*ALARM_ITEM_ONE_SIZE];			  
			  g_RamAlarmBlock.pdata[alarm_id].alarm_hour=data_prt[6+temp*ALARM_ITEM_ONE_SIZE];
			  g_RamAlarmBlock.pdata[alarm_id].alarm_minute=data_prt[7+temp*ALARM_ITEM_ONE_SIZE];
				g_RamAlarmBlock.pdata[alarm_id].alarm_set=1;
        }
				else
				{
					Protocol_Packet_Status(data_prt,CMD_STATUS_ALARM_OVER,ser_flag);
					return;
				}    
    }
		
	etSpim1ReadNorFlashStdMode(KEYID_START_ADDR,keyid,FLASH_GET_LENGTH);
	keyid[ALARM_FLAG_OFFSET]=MAC_VALID_TAG1;
	keyid[ALARM_LENGTH_OFFSET]=data_prt[3];
	memcpy(keyid+ALARM_DATA_OFFSET,data_prt+4,data_prt[3]);
	etWrite_KEYID_to_flash(KEYID_START_ADDR,keyid,FLASH_GET_LENGTH);

	Protocol_Packet_Status(data_prt,CMD_STATUS_SUCCESS,ser_flag);
		
}

static void Protocol_GetAlarm(UINT8 ser_flag)
{
	
	UINT8 temp_alarm[ALARM_ITEM_ONE_SIZE*ALARM_ITEM_NUMBER_MAX+1];
	UINT8 count=0,temp1=0;
	UINT8 temp2=0;
	memset(temp_alarm,0,sizeof(temp_alarm));
	temp_alarm[0]=ALARM_ITEM_NUMBER_MAX;
	for(temp1=0;temp1<ALARM_ITEM_NUMBER_MAX;temp1++)
	{
	   if(g_RamAlarmBlock.pdata[temp1].alarm_set==1)
	   {
	    if(g_RamAlarmBlock.pdata[temp1].alarm_enable==1)	    
	    {
           temp2=ALARM_ENABLE_BIT |temp1;
	    }
			else
			{
         temp2=temp1;
			}
		   temp_alarm[1+count*ALARM_ITEM_ONE_SIZE]=temp2;
		   temp_alarm[2+count*ALARM_ITEM_ONE_SIZE]=g_RamAlarmBlock.pdata[temp1].alarm_week;
		   temp_alarm[3+count*ALARM_ITEM_ONE_SIZE]=g_RamAlarmBlock.pdata[temp1].alarm_hour;
		   temp_alarm[4+count*ALARM_ITEM_ONE_SIZE]=g_RamAlarmBlock.pdata[temp1].alarm_minute;	
		   count++;		   
	   }

	}
	Protocol_data_to_AG(CMD_GET_ALARM,temp_alarm,(1+count*ALARM_ITEM_ONE_SIZE),ser_flag);

}

/*
static void Protocol_SetAutoPowerOff(unsigned char * data_prt,unsigned short packet_length)
{
    if((data_prt[4]==1))
    {
		
		//theSink.clk_alarm.time_ctr.AutoSwitchOffTime_s=(data_prt[5]<<8 |data_prt[6]);			
    }
	else
	{
		
		//theSink.clk_alarm.time_ctr.AutoSwitchOffTime_s=0;
	}
}*/
static void Protocol_GetAutoPowerOff(UINT8 ser_flag)
{
    UINT8 temp[3];
        temp[0]=0x00;
		temp[1]=0x00;
		temp[2]=0x00;
  		Protocol_data_to_AG(CMD_GET_AUTO_POWEROFF,temp,3,ser_flag);
}

/*
static void Protocol_SetMotor(unsigned char * data_prt,unsigned short packet_length)
{
   if(data_prt[4]==0x01)
   theSink.Enable_Moter=1;
   else
   	theSink.Enable_Moter=0;

}*/
static void Protocol_GetMotor(UINT8 ser_flag)
{
   unsigned char temp=0;
   if(theSink.Enable_Moter==0x01)
   temp=1;
   else
   temp=0;
   Protocol_data_to_AG(CMD_GET_MOTOR,&temp,1,ser_flag);
}

/*
void Protocol_GetPedometerNum(void)
{
        UINT8 temp[4];
        temp[0]=(theSink.pedometer.pedometer_num>>24) & 0xff;
		temp[1]=(theSink.pedometer.pedometer_num>>16) & 0xff;
		temp[2]=(theSink.pedometer.pedometer_num>>8) & 0xff;
		temp[3]=theSink.pedometer.pedometer_num & 0xff;
  		Protocol_data_to_AG(CMD_GET_STEP_NUMBER,temp,4);   
    
}*/
#if 0
static void Protocol_COS_TO_AG(UINT8 sw)
{
	
		//TWORD data SW12,i;
		//TBYTE tmpSW[2];
		UINT16 SW12,i;
		UINT8 tmpSW[2];
		BOOL ret;
	
		SW12 = gSW_Table[sw];
		if (sw == SW61XX)
		{
			SW12  |= G_Len_Response; 
		}
		if (sw == SW6CXX)
		{
			SW12  |= G_Len_6CXX ; 
		}
		if (sw == SW63CX)
		{
			SW12  |= G_Len_63CX ;
		}
	
		if (sw == SW_ATR)
		{
		    /*
			TBYTE check = 0x0D;					
			ptempbuf[0] = 0xAA;
			ptempbuf[1] = 0x0D;
			ptempbuf[2] = (TBYTE)(G_APDU_LeEx >> 8);;
			ptempbuf[3] = (TBYTE)G_APDU_LeEx;
			memcpy(&ptempbuf[4], G_APDU_Data, (TWORD)G_APDU_LeEx);
	
			check += ptempbuf[2];
			check += ptempbuf[3];
			for( i=0; i<G_APDU_LeEx;i++)
			{
				check += G_APDU_Data[i];
			}
	
			ptempbuf[4+(TWORD)G_APDU_LeEx] = ~check;
			ptempbuf[5+(TWORD)G_APDU_LeEx] = 0x55;
	
			ret=Toshiba_App_SendDataToHost(ptempbuf, G_APDU_LeEx+6, sendtype);	       
			G_APDU_LeEx = 0;	
			return ret;	
			*/
			Protocol_data_to_AG(CMD_COS_DATA,G_APDU_Data,G_APDU_LeEx);
			G_APDU_LeEx = 0;
			return;
		}
	
		if (G_APDU_LeEx == 0)
		{
		    /*
			TBYTE check = 0x0D;
	
			G_APDU_Data[256] = (SW12 >> 8);
			G_APDU_Data[257] = (TBYTE)SW12;
				
			ptempbuf[0] = 0xAA;
			ptempbuf[1] = 0x0D;
			ptempbuf[2] = 0x00;
			ptempbuf[3] = 2;
	

			memcpy(&ptempbuf[4], &G_APDU_Data[256], 2);
			
			check += ptempbuf[2];
			check += ptempbuf[3];
			for( i=0; i<2;i++)
			{
				check += G_APDU_Data[256+i];
			}
	
			ptempbuf[6] = ~check;
			ptempbuf[7] = 0x55;	
			ret=Toshiba_App_SendDataToHost(ptempbuf, 8, sendtype);
			
			*/
			G_APDU_Data[256] = (SW12 >> 8);
			G_APDU_Data[257] = (TBYTE)SW12;
			Protocol_data_to_AG(CMD_COS_DATA,&G_APDU_Data[256],2);
		 }
		else
		{	
			//TBYTE check = 0x0D;
	
			tmpSW[0] = (SW12 >> 8);
			tmpSW[1] = (TBYTE)SW12;
		/*
			ptempbuf[0] = 0xAA;
			ptempbuf[1] = 0x0D;
			ptempbuf[2] = (TBYTE)((G_APDU_LeEx+2) >> 8);
			ptempbuf[3] = (TBYTE)(G_APDU_LeEx+2);

	
			if( G_APDU_CLA == 0x00 && G_APDU_INS == 0xC0 )
				memcpy(&ptempbuf[4], G_APDU_Data+3, (TWORD)G_APDU_LeEx);
			else
				memcpy(&ptempbuf[4], G_APDU_Data, (TWORD)G_APDU_LeEx);

			memcpy(&ptempbuf[4+(TWORD)G_APDU_LeEx], tmpSW, 2);
			
			check += ptempbuf[2];
			check += ptempbuf[3];
			for( i=0; i<G_APDU_LeEx;i++)
			{
				if( G_APDU_CLA == 0x00 && G_APDU_INS == 0xC0 )
					check += G_APDU_Data[i+3];
				else
					check += G_APDU_Data[i];
			}
			check += tmpSW[0];
			check += tmpSW[1];
	
			ptempbuf[4+(TWORD)G_APDU_LeEx+2] = ~check;
			ptempbuf[4+(TWORD)G_APDU_LeEx+2+1] = 0x55;
	
			ret=Toshiba_App_SendDataToHost(ptempbuf, 4+(TWORD)G_APDU_LeEx+2+1+1, sendtype);
	        */
			if( G_APDU_CLA == 0x00 && G_APDU_INS == 0xC0 )				
				memcpy(uart_recv.uart_union.data_str.part_data, G_APDU_Data+3, (TWORD)G_APDU_LeEx);//memcpy(&ptempbuf[4], G_APDU_Data+3, (TWORD)G_APDU_LeEx);
			else				
			    memcpy(uart_recv.uart_union.data_str.part_data, G_APDU_Data, (TWORD)G_APDU_LeEx);//memcpy(&ptempbuf[4], G_APDU_Data, (TWORD)G_APDU_LeEx);
			    
			memcpy(uart_recv.uart_union.data_str.part_data+G_APDU_LeEx, tmpSW,2);//memcpy(&ptempbuf[4], G_APDU_Data, (TWORD)G_APDU_LeEx);
            Protocol_data_to_AG(CMD_COS_DATA,uart_recv.uart_union.data_str.part_data,G_APDU_LeEx+2);
			
			if(G_Len_Response)
			{
				if( G_APDU_CLA == 0x00 && G_APDU_INS == 0xC0 )
					XRamCopy(G_APDU_Data+3,G_APDU_Data+3+G_APDU_LeEx,G_Len_Response);
				else
					XRamCopy(G_APDU_Data,G_APDU_Data+G_APDU_LeEx,G_Len_Response);
			}
		}  
		
		G_APDU_LeEx = 0;
	
		//return ret;


}
#endif
static void Protocol_DataToCos(unsigned char * data_prt,unsigned short packet_length)
{
        //UINT8 sw;
		//theSink.COS_data_ready=1;
		//UINT16 temp_length=0;
		if(packet_length==11)
		{
			if((data_prt[4]==BT_DISCONNECT_CODE[4])&&
			   (data_prt[5]==BT_DISCONNECT_CODE[5])&&
			   (data_prt[6]==BT_DISCONNECT_CODE[6])&&
			   (data_prt[7]==BT_DISCONNECT_CODE[7])&&
			   (data_prt[8]==BT_DISCONNECT_CODE[8]))
			{
				 //Toshiba_TCU_MNG_LE_DISCONNECT_REQ();
         disconntec_AG();
         Key_value_reset();
				 return;
			
			}
		}
		
#if ENABLE_COS

      if(Get_Cos_Ready()==0)
		  {
			uart_recv.cos_protocol_rev_data_ready=1;
      simple_uart_init(UART_POWEROFF_BT_DELAY);//kevin add 160112
			return;
      }
      set_cos_mode();

      AG_to_Cos_send(data_prt,packet_length);
#endif
		//copy apdu data
		/*memcpy(&G_APDU_DataLV.head[1], data_prt[PACKET_DATA_OFFSET], packet_length-CMD_PACKET_BASE_LENGTH);
		
		//process apdu
		SendAPDU(&G_APDU_DataLV.head[1], packet_length-CMD_PACKET_BASE_LENGTH);

        Protocol_COS_TO_AG(g_cos_sw);*/
}
/*
static void Protocol_ResetPedometerNum(void)
{
		 theSink.pedometer.pedometer_num = 0;
}*/
/*
static void Protocol_GetJumpNum(void)
{
	     UINT8 temp[4];
		 temp[0]=(theSink.pedometer.jump_num>>24) & 0xff;
		 temp[1]=(theSink.pedometer.jump_num>>16) & 0xff;
		 temp[2]=(theSink.pedometer.jump_num>>8) & 0xff;
		 temp[3]=theSink.pedometer.jump_num & 0xff;
		 Protocol_data_to_AG(CMD_GET_JUMP_NUMBER,temp,4);	

}
*/
/*
static void Protocol_ResetJumpNum(void)
{
		 theSink.pedometer.jump_num = 0;
}
*/
/*
static void Protocol_GetSitupNum(void)
{
	     UINT8 temp[4];
		 temp[0]=(theSink.pedometer.situp_num>>24) & 0xff;
		 temp[1]=(theSink.pedometer.situp_num>>16) & 0xff;
		 temp[2]=(theSink.pedometer.situp_num>>8) & 0xff;
		 temp[3]=theSink.pedometer.situp_num & 0xff;
		 Protocol_data_to_AG(CMD_GET_SITUP_NUMBER,temp,4);	

}*/
	/*
static void Protocol_ResetSitupNum(void)
{
		 theSink.pedometer.situp_num = 0;
}*/
static void Protocol_GetBatteryVolt(UINT8 ser_flag)
{
		
		UINT8 battery_level = 80;

		battery_level=Get_BatteryPercent();
		/* calculate %battery level using: (currentV - minV)/(maxV - minV)*100 */
	/*	if (theSink.rundata->battery_limits.max_battery_v > theSink.rundata->battery_limits.min_battery_v)
		{
			if (reading.voltage < theSink.rundata->battery_limits.min_battery_v)
			{
				battery_level = 0;
			}
			else if (reading.voltage > theSink.rundata->battery_limits.max_battery_v)
			{
				battery_level = 100;
			}
			else
			{
				battery_level = (UINT8)(((uint32)(reading.voltage - theSink.rundata->battery_limits.min_battery_v)	* (uint32)100) / (uint32)(theSink.rundata->battery_limits.max_battery_v - theSink.rundata->battery_limits.min_battery_v));
			}
		} */
		Protocol_data_to_AG(CMD_BATTERY_VOLTAGE,&battery_level,1,ser_flag);	

}
/*
static void Protocol_Send_MESSAGE(unsigned char * data_prt,UINT16 packet_length)
{


}*/
static void Protocol_InComingCall(unsigned char * data_prt,UINT16 packet_length)
{  
      if((data_prt[4]==0x01)||(data_prt[4]==0x03))
      {
        display_IncomingCall(0,data_prt+7,packet_length-8);		 
	  }
	  else
	  {
		  if(data_prt[5]>0)
		  {
		      display_IncomingCall(1,data_prt+6,data_prt[5]);
		  }
		  else
		  {
			  display_IncomingCall(1,data_prt+6,packet_length-8);

		  }
	  }

}
/*
static void Protocol_MAIL_PROMPT(void)
{
  //lcd_display_Mail_prompt();
}
static void Protocol_WeChat_PROMPT(void)
{
  //lcd_display_WeChat_prompt();

}

//static void Protocol_Send_INCOMING_CALL_STATUS(unsigned char * data_prt,UINT16 packet_length)
//{
   //lcd_clear_handle();

//}
static void Protocol_Send_MESSAGE_prompt(unsigned char * data_prt,UINT16 packet_length)
{
   //lcd_display_Message_prompt();
}

static void Protocol_Set_Alert_Time(unsigned char * data_prt,UINT16 packet_length)
{

}
static void Protocol_Get_Alert_Time(void)
{
	
	UINT8 temp_alarm[ALERT_MAX_NUM*ALERT_ONE_OFFSET];
	UINT8 temp=0,temp1=0;
	memset(temp_alarm,0,sizeof(temp_alarm));
	temp_alarm[0]=ALERT_MAX_NUM;
	for(temp1=0;temp1<ALERT_MAX_NUM;temp1++)
	{
	    if(theSink.clk_alarm.alert[temp1].alert_enable==1)
	    {
	    temp_alarm[1+temp*ALERT_ONE_OFFSET]=theSink.clk_alarm.alert[temp1].alert_enable>0?ALERT_ENABLE_BIT_OFFSET:0;
		temp_alarm[1+temp*ALERT_ONE_OFFSET] |=theSink.clk_alarm.alert[temp1].alarm_pause>0?ALERT_PAUSE_BIT_OFFSET:0;
		temp_alarm[1+temp*ALERT_ONE_OFFSET] |=theSink.clk_alarm.alert[temp1].alert_count;
		temp_alarm[2+temp*ALERT_ONE_OFFSET]=theSink.clk_alarm.alert[temp1].alert_hour;
		temp_alarm[3+temp*ALERT_ONE_OFFSET]=theSink.clk_alarm.alert[temp1].alert_minute;
		temp++;
	    }
		else
		{


		}

	}
	Protocol_data_to_AG(CMD_GET_ALERT_TIME,temp_alarm,1+temp*ALERT_ONE_OFFSET);
  

}*/
static void Protocol_FIND_DEVICE(void)
{
	//Protocol_data_to_AG(CMD_FIND_DEVICE,NULL,0);
	MotorCtr(1,MOTOR_TYPE_TEST);
}
static void Protocol_SET_AUTOSEND_STEP_NUM(unsigned char * data_prt,UINT16 packet_length)
{	
	
#if 1
	if(data_prt[4] &&(packet_length>6))
	{
		//theSink.Enable_Send_stepNum=1;
		theSink.AutoSend_RealtimeDataFlg=1;
		theSink.Step_RealtimeData_send_ok_flag=0;
		theSink.Step_Flag=1;//有STEP 标志
	}
	else
	{
		//theSink.Enable_Send_stepNum=0;
		theSink.AutoSend_RealtimeDataFlg=0;
		theSink.Step_RealtimeData_send_ok_flag=0;
	}

	if(data_prt[5] &&(packet_length>7))
	{
		theSink.AutoSend_TadayDataFlg = 1;
		theSink.SportDataFlg_over = 1;
		theSink.SportDataFlg_end=0;
		Read_SportBlock=0;
		//ReciveTime=0;
		//ReciveTime=ReciveTime |data_prt[6];
		//ReciveTime=(ReciveTime<<8) |data_prt[7];
		//ReciveTime=(ReciveTime<<8) |data_prt[8];
		//ReciveTime=(ReciveTime<<8) |data_prt[9];
	}
	else
	{
		theSink.AutoSend_TadayDataFlg=0;
		theSink.SportDataFlg_over = 0;
		theSink.SportDataFlg_end=0;
	}

	if(data_prt[6] &&(packet_length>8))
	{
		theSink.AutoSend_HistoricalDataFlg=1;
		theSink.AutoSend_HistoricalDataFlg_end=0;
		DateNumer=data_prt[8];
	}
	else
	{
		theSink.AutoSend_HistoricalDataFlg=0;
		theSink.AutoSend_HistoricalDataFlg_end=0;
	}

	if(data_prt[7]&&(packet_length>9))
	{
		theSink.AutoSend_SleepDataFlg=1;
		theSink.SleepDataFlg_over=1;
		theSink.SleepDataFlg_end=0;
		Read_SleepBlock=0;
	}
	else
	{
		theSink.AutoSend_SleepDataFlg=0;
		theSink.SleepDataFlg_over=0;
		theSink.SleepDataFlg_end=0;
	}
	if(data_prt[8]&&(packet_length>10))
	{
		theSink.AutoSend_EveryDataTotalSleepRecordFlg=1;
		theSink.EveryDataTotalSleepRecord_over=1;
		theSink.EveryDataTotalSleepRecord_end=0;
	}
	else
	{
		theSink.AutoSend_EveryDataTotalSleepRecordFlg=0;
		theSink.EveryDataTotalSleepRecord_over=0;
		theSink.EveryDataTotalSleepRecord_end=0;
	}  
	#endif

}
static void Protocol_Erase_flash(unsigned char * data_prt,UINT16 packet_length,UINT8 ser_flag)
{
	UINT8 flash_status=0;    
	UINT32 flash_address=0;
	flash_address =data_prt[4]<<24;
	flash_address |=data_prt[5]<<16;
	flash_address |=data_prt[6]<<8;
	flash_address |=data_prt[7];
	//etSpim1NorFlashBlockErase(flash_address);
	//etSpim1NorFlashChipErase();
	etSpim1NorFlashSectorErase(flash_address);
	Protocol_data_to_AG(CMD_ERASE_FLASH,&flash_status,1,ser_flag);
}
static void Protocol_WriteData_To_flash(unsigned char * data_prt,UINT16 packet_length,UINT8 ser_flag)
{
	UINT8 flash_status=0;
#if DEBUG_UART_EN
        //DBG_MSG(("write data:\r\n"));
		//for (uint32_t i = 0; i <packet_length-10; i++)
		//{
		//	while(app_uart_put(data_prt[8+i]) != NRF_SUCCESS);
		//}		
        //DBG_MSG(("\r\n"));
#endif	
	UINT32 flash_address=0;
	flash_address =data_prt[4]<<24;
	flash_address |=data_prt[5]<<16;
	flash_address |=data_prt[6]<<8;
	flash_address |=data_prt[7];	
    etSpim1NorFlashPageProgrameStdMode(flash_address,data_prt+8,packet_length-10);
	Protocol_data_to_AG(CMD_WRITE_FLASH,&flash_status,1,ser_flag);
}

static void Protocol_ReadData_To_flash(unsigned char * data_prt,UINT16 packet_length,UINT8 ser_flag)
{
	UINT32 readlength=0;
	UINT32 flash_address=0;
  UINT8 read_buff[256];
	
	flash_address =data_prt[4]<<24;
	flash_address |=data_prt[5]<<16;
	flash_address |=data_prt[6]<<8;
	flash_address |=data_prt[7];
  readlength=data_prt[8]*256 +data_prt[9];	
	if(readlength>256)
	{
  etSpim1ReadNorFlashStdMode(flash_address,read_buff,256);
	Protocol_data_to_AG(CMD_READ_FLASH,read_buff,256,ser_flag);
	}
	else
	{
	etSpim1ReadNorFlashStdMode(flash_address,read_buff,readlength);
	Protocol_data_to_AG(CMD_READ_FLASH,read_buff,readlength,ser_flag);
	}
}


static void Protocol_test_sensor(UINT8 ser_flag)
{
  UINT8 temp_ID=0;
	LIS3DH_GetWHO_AM_I(&temp_ID);
	Protocol_data_to_AG(CMD_TEST_SENSOR,&temp_ID,1,ser_flag);

}

static void Protocol_Send_COS_status(UINT8 ser_flag)
{
    UINT8 cos_status[1]={0};
#if ENABLE_COS
		if(Get_Cos_Ready()==0)
		{
       cos_status[0]=1;
			 simple_uart_init(UART_POWEROFF_BT_DELAY); 
		}
		if(Get_COS_status()==1)
		{
     cos_status[0]=2;
		 simple_uart_init(UART_POWEROFF_BT_DELAY); 
		}
#endif
 		
    Protocol_data_to_AG(CMD_GET_COS_STATUS,cos_status,1,ser_flag);

}
/*
static void Protocol_Display_AG_Name(unsigned char * data_prt,UINT16 packet_length)
{
    //UINT8 name_length=0;
	UINT16 data_length=0;
	
	data_length=data_prt[2] & PACKET_L_MSB_FLAG;
	data_length <<=8 ;
	data_length |=data_prt[3]; 
}*/
/*
static void Protocol_SendDeviceStatus(void)
{
   unsigned char status=0;
   Protocol_data_to_AG(CMD_GET_DEVICE_STATUS,&status,1);

}
*/

static void Protocol_BT_loopbackTest(unsigned char * data_prt,unsigned short packet_length,UINT8 ser_flag)
{
   //Protocol_send_data_to_AG(data_prt,packet_length);
/*   UINT16 i=0;
   DbgPrintf("test data:\r\n");
   for(i=0;i<packet_length;i++)
   {
    DbgPrintf("%x ",data_prt[i]);
   }
   DbgPrintf("\r\n");
   */
   Protocol_data_to_AG(CMD_BT_LOOPBACK_TEST,&data_prt[PACKET_DATA_OFFSET],packet_length-CMD_PACKET_BASE_LENGTH,ser_flag);
}
/*************************************************************************************************************************/
static unsigned char Protocol_check_data(unsigned char * data_prt,unsigned short packet_length)
{
     UINT16 count=0;
     UINT8 temp_check=0;

	 
	if(packet_length<(CMD_PACKET_BASE_LENGTH))
		   return 0;

     if(data_prt[0]!=0xaa)
         return 0;
     if(data_prt[packet_length-1]!=0x55)
         return 0;
     for(count=1;count<packet_length-2;count++)
     {
          temp_check += data_prt[count];  
     }
    temp_check = (~temp_check)&0xff;
    if(temp_check == data_prt[packet_length-2])
        return 1;
    else 
        return 0;
}


/************************************************************************************************************************/
/*
void Protocol_SendVersion_To_AG(UINT8 * data_prt,UINT16 packet_length)
{
   unsigned char firmware_version[]="R1.00.00";   
   UINT8 prt[strlen((const char *)firmware_version)+packet_length+3];
   UINT16 length=0;
   prt[length++]=FLAG_FIRMWARE_VERSION;
   memcpy(prt+length,firmware_version,strlen((const char *)firmware_version));
    length +=strlen((const char *)firmware_version);
   prt[length++]=FLAG_COS_VERSION;
   memcpy(prt+length,data_prt,packet_length);
   length +=packet_length;

   Protocol_data_to_AG(CMD_GET_VERSION,prt,length);

   
}
*/
//send taday sport data to app
void Protocol_send_sport_data_to_ag(UINT8 * data_prt,UINT16 packet_length,UINT8 ser_flag)
{
	Protocol_data_to_AG(CMD_SEND_SPORT_DATA_TO_APP,data_prt,packet_length,ser_flag);
}
//send sleep data to app
void Protocol_send_sleep_data_to_ag(UINT8 * data_prt,UINT16 packet_length,UINT8 ser_flag)
{
	Protocol_data_to_AG(CMD_SEND_SLEEP_DATA_TO_APP,data_prt,packet_length,ser_flag);
}
void Protocol_send_everyDayTotalSleepData_to_ag(UINT8 * data_prt,UINT16 packet_length,UINT8 ser_flag)
{
	Protocol_data_to_AG(CMD_EVERYDAY_TOTAL_SLEEPDATA,data_prt,packet_length,ser_flag);
}


void auto_send_sport_Live_to_app(UINT8 * data_prt,UINT16 packet_length,UINT8 ser_flag)/*实时数据*/
	{
		if((theSink.AutoSend_RealtimeDataFlg==1) && (theSink.Step_Flag==1) && (theSink.Step_RealtimeData_send_ok_flag==0))//有STEP 标志
		{
			theSink.Step_Flag=0;
			theSink.Step_RealtimeData_send_ok_flag=2;
			Protocol_data_to_AG(CMD_SEND_REALTIMER_DATA,data_prt,packet_length,ser_flag);
		}
	}

void Protocol_send_histry_sport_data_to_ag(UINT8 * data_prt,UINT16 packet_length,UINT8 ser_flag)
{
	Protocol_data_to_AG(CMD_SEND_HISTRY_EVRYDAY_SPORT_DATA_TO_APP,data_prt,packet_length,ser_flag);
}
//void Protocol_send_ack_to_ag_by_app_require(UINT8 * data_prt,UINT16 packet_length,UINT8 ser_flag)
//{
//	Protocol_data_to_AG(CMD_REQUIRE_DATA_FROM_APP,data_prt,packet_length,ser_flag);
//}
void Protocol_send_realTime_data(UINT8 * data_prt,UINT16 packet_length,UINT8 ser_flag)
{
	Protocol_data_to_AG(CMD_SEND_REALTIMER_DATA,data_prt,packet_length,ser_flag);
}
/*
void Protocol_send_ack_to_ag_by_usr_info(UINT8 * data_prt,UINT16 packet_length,UINT8 ser_flag)
{
	Protocol_data_to_AG(CMD_STATUS,data_prt,packet_length,ser_flag);
}
*/
void Protocol_send_usr_info(UINT8 * data_prt,UINT16 packet_length,UINT8 ser_flag)
{
	Protocol_data_to_AG(CMD_GET_INFO,data_prt,packet_length,ser_flag);
}

void Protocol_AutoSendStepNum(UINT8 * data_prt,UINT16 packet_length,UINT8 ser_flag)
{
	Protocol_data_to_AG(CMD_SET_AUTOSEND_STEP_NUM,data_prt,packet_length,ser_flag);
}

void Protocol_CMD_FIND_AG(UINT8 ser_flag)
{  
	Protocol_data_to_AG(CMD_FIND_AG,NULL,0,ser_flag);
}

/*************************************************************************************************************************/
static void Protocol_handle(UINT8 * data_prt,UINT16 packet_length,UINT8 flag)
{           

        switch(data_prt[1])
        {
         case CMD_GET_MTU:
					
                	Protocol_send_MTU_to_AG(flag);
			    break;
				case CMD_SEND_AG_STATUS:

					//Protocol_send_AG_status(data_prt,packet_length);
					//Protocol_send_DeviceName_to_AG();
#ifdef ENABLE_STATUS_RETURN
					//Protocol_Packet_Status(data_prt,CMD_STATUS_SUCCESS);
#endif					
				break;
				case CMD_GET_VERSION:

					Protocol_get_cos_ver_req(flag);
				break;
				case CMD_SET_CLOCK:

					Protocol_SetClock(data_prt,packet_length);
#ifdef ENABLE_STATUS_RETURN
					Protocol_Packet_Status(data_prt,CMD_STATUS_SUCCESS,flag);
#endif
				break;
				case CMD_GET_CLOCK:
					Protocol_GetClock(flag);
				break;				
				case CMD_SET_ALARM:
					
					Protocol_SetAlarm(data_prt,packet_length,flag);
				break;				
				case CMD_GET_ALARM:

					Protocol_GetAlarm(flag);
				break;  
				case CMD_SET_AUTO_POWEROFF:

          //Protocol_SetAutoPowerOff(data_prt,packet_length);
#ifdef ENABLE_STATUS_RETURN
					Protocol_Packet_Status(data_prt,CMD_STATUS_SUCCESS,flag);
#endif

				break; 	
				case CMD_GET_AUTO_POWEROFF:
					Protocol_GetAutoPowerOff(flag);
				break; 
				case CMD_SET_MOTOR:
           //Protocol_SetMotor(data_prt,packet_length);
#ifdef ENABLE_STATUS_RETURN
					Protocol_Packet_Status(data_prt,CMD_STATUS_SUCCESS,flag);
#endif

				break; 	
				case CMD_GET_MOTOR:
                    Protocol_GetMotor(flag);
				break; 	
				case CMD_GET_STEP_NUMBER:
                    // Protocol_GetPedometerNum(); 
				break; 
				case CMD_COS_DATA:
             Protocol_DataToCos(data_prt,packet_length);
				return;
				case CMD_RESET_STEP_NUMBER:
                 //    Protocol_ResetPedometerNum(); 
#ifdef ENABLE_STATUS_RETURN
					Protocol_Packet_Status(data_prt,CMD_STATUS_SUCCESS,flag);
#endif

				break; 			
				case CMD_GET_JUMP_NUMBER:
        //             Protocol_GetJumpNum(); 
				break; 	
				case CMD_RESET_JUMP_NUMBER:
        //             Protocol_ResetJumpNum();
#ifdef ENABLE_STATUS_RETURN
					Protocol_Packet_Status(data_prt,CMD_STATUS_SUCCESS,flag);
#endif					 
				break; 
				case CMD_GET_SITUP_NUMBER:
					//Protocol_GetSitupNum();
				break;	
				case CMD_RESET_SITUP_NUMBER:
					//Protocol_ResetSitupNum();
#ifdef ENABLE_STATUS_RETURN
					Protocol_Packet_Status(data_prt,CMD_STATUS_SUCCESS,flag);
#endif

				break;	
				case CMD_BATTERY_VOLTAGE:
                    Protocol_GetBatteryVolt(flag);
				break;
				case CMD_SEND_MESSAGE:
                    //Protocol_Send_MESSAGE(data_prt,packet_length);
#ifdef ENABLE_STATUS_RETURN
					Protocol_Packet_Status(data_prt,CMD_STATUS_SUCCESS,flag);
#endif

				break;
				case CMD_SEND_INCOMING_CALL:
					Protocol_InComingCall(data_prt,packet_length);
#ifdef ENABLE_STATUS_RETURN
					Protocol_Packet_Status(data_prt,CMD_STATUS_SUCCESS,flag);
#endif

				break;
				case CMD_AS_RJ_INCOMING_CALL:
                    
				break;
				case CMD_SEND_MAIL_PROMPT:
					//Protocol_MAIL_PROMPT();
#ifdef ENABLE_STATUS_RETURN
					Protocol_Packet_Status(data_prt,CMD_STATUS_SUCCESS,flag);
#endif

				break;	
				case CMD_SEND_WECHAT_PROMPT:
           //Protocol_WeChat_PROMPT();
#ifdef ENABLE_STATUS_RETURN
					Protocol_Packet_Status(data_prt,CMD_STATUS_SUCCESS,flag);
#endif

				break;
				case CMD_SEND_INCOMING_CALL_STATUS:
                     //Protocol_Send_INCOMING_CALL_STATUS(data_prt,packet_length);
#ifdef ENABLE_STATUS_RETURN
					 Protocol_Packet_Status(data_prt,CMD_STATUS_SUCCESS,flag);
#endif

				break;	
				case CMD_SEND_MESSAGE_PROMPT:
                // Protocol_Send_MESSAGE_prompt(data_prt,packet_length);
#ifdef ENABLE_STATUS_RETURN
					 Protocol_Packet_Status(data_prt,CMD_STATUS_SUCCESS,flag);
#endif

				break;	
				case CMD_SET_ALERT_TIME:
           //          Protocol_Set_Alert_Time(data_prt,packet_length,flag);
#ifdef ENABLE_STATUS_RETURN
					 Protocol_Packet_Status(data_prt,CMD_STATUS_SUCCESS,flag);
#endif

				break;
				//case CMD_GET_ALERT_TIME:
         //            Protocol_Get_Alert_Time();
				//break;
				case CMD_FIND_DEVICE:
                     Protocol_FIND_DEVICE(); 
#ifdef ENABLE_STATUS_RETURN
					 Protocol_Packet_Status(data_prt,CMD_STATUS_SUCCESS,flag);
#endif										 
				break;		
				case CMD_SET_AUTOSEND_STEP_NUM:
#ifdef ENABLE_STATUS_RETURN
					Protocol_Packet_Status(data_prt,CMD_STATUS_SUCCESS,flag);/*ACK*/
#endif
					Protocol_SET_AUTOSEND_STEP_NUM(data_prt,packet_length);
				break;
        case CMD_CTR_COS_POWER:
					if(data_prt[4]==1)
					{
					simple_uart_init(UART_POWEROFF_BT_DELAY);
					}
					else
					{
          simple_uart_init(UART_POWEROFF_DELAY);
					}
#ifdef ENABLE_STATUS_RETURN
					  Protocol_Packet_Status(data_prt,CMD_STATUS_SUCCESS,flag);
#endif					
				break;
				
				case CMD_GET_COS_STATUS:
					 Protocol_Send_COS_status(flag);
			
				break;
				case CMD_COS_AG_TRADE_COMPLETE:	
					 simple_uart_init(UART_POWEROFF_DELAY);
#ifdef ENABLE_STATUS_RETURN
					  Protocol_Packet_Status(data_prt,CMD_STATUS_SUCCESS,flag);
#endif					
				break;
        case CMD_ERASE_FLASH:
					 Protocol_Erase_flash(data_prt,packet_length,flag);
				break;
        case CMD_WRITE_FLASH:
					 Protocol_WriteData_To_flash(data_prt,packet_length,flag);
				break;
        case CMD_READ_FLASH:
					 Protocol_ReadData_To_flash(data_prt,packet_length,flag);
				break;

				case CMD_DISPLAY_AG_NAME:
               //      Protocol_Display_AG_Name(data_prt,packet_length);
#ifdef ENABLE_STATUS_RETURN
					 Protocol_Packet_Status(data_prt,CMD_STATUS_SUCCESS,flag);
#endif

				break;			
        case CMD_TEST_SENSOR:
				     Protocol_test_sensor(flag);
				break;
				case CMD_TEST_LCD:
					 Dislay_Test_LCD();
#ifdef ENABLE_STATUS_RETURN
					 Protocol_Packet_Status(data_prt,CMD_STATUS_SUCCESS,flag);
#endif					
				break;
				case CMD_GET_DEVICE_STATUS:
					//Protocol_SendDeviceStatus();					  
				break;
				case CMD_TEST_AG_COS_CHANNEL:
					 //Protocol_DataToCos(data_prt,packet_length);
				break;
				case CMD_BT_LOOPBACK_TEST:
					 Protocol_BT_loopbackTest(data_prt,packet_length,flag);
				break;
				case CMD_STATUS:
					switch(data_prt[4])
					{
						case CMD_SEND_SPORT_DATA_TO_APP:
							send_sport_get_app_ack(data_prt,packet_length);
							break;
						case CMD_SEND_SLEEP_DATA_TO_APP:
							send_sleep_get_app_ack(data_prt,packet_length);
							break;
						case CMD_SEND_HISTRY_EVRYDAY_SPORT_DATA_TO_APP:
							send_histy_sport_record_get_app_ack(data_prt,packet_length);
						  break;
						case CMD_SEND_REALTIMER_DATA:
							theSink.Step_RealtimeData_send_ok_flag=0;
							break;
						case CMD_COS_DATA:
							if(data_prt[4]>0)
							{
							ResendCosData_to_AG();
							}
					  break;
						default:
						break;
					}
				break;
				case CMD_DFU_UPDATED:
						
				break;

				
				#if 0	
				case CMD_REQUIRE_DATA_FROM_APP:/*APP ???óμ±ìì?÷????êy?Y*/
				{								  /*?-à′:APP???ó???ˉêy?Y*/
					return_app_require_data(data_prt,packet_length);				
					
				}
				break;
				#endif
				case CMD_USR_INFO://Receive app info
					receive_app_usr_info(data_prt,packet_length);
#ifdef ENABLE_STATUS_RETURN
					 Protocol_Packet_Status(data_prt,CMD_STATUS_SUCCESS,flag);
#endif					
					break;
				case CMD_GET_INFO:
					Get_app_usr_info(flag);//Send info to APP
					break;       
        
				default:				
				break;

      }


    

}

void protocol_data_can_send(void)
{
    if(uart_recv.cos_protocol_rev_data_ready==1)
    {
      uart_recv.cos_protocol_rev_data_ready=0;
			Protocol_handle(uart_recv.cos_uart_union.cos_taltol_data,uart_recv.cos_revDataTotalLen,SERVICE_COS);		
	}
}


static void protocol_Save_data(UINT8 *buff,UINT16 temp_length,UINT8 ser_flag)
{
   	  switch(buff[PACKET_LENGTH_OFFSET] & PACKET_MARK_FLAG)
   	  {
         case PACKET_MARK_FULL:	
					 /* if(buff[PACKET_CMD_OFFSET]!=CMD_STATUS)
					  {*/
						  if(buff[PACKET_CMD_OFFSET]==CMD_GET_MTU)
						  {
                Protocol_handle(buff,temp_length,ser_flag);
							}
							else
							{
									memcpy((uart_recv.cos_uart_union.cos_taltol_data),buff,temp_length);
									uart_recv.cos_revDataTotalLen=buff[PACKET_LENGTH_OFFSET]<<8 | buff[PACKET_LENGTH_OFFSET+1];			
								//uart_recv.uart_union.data_str.head=PACKET_DATA_HEAD;
								//uart_recv.uart_union.data_str.CMD=buff[PACKET_CMD_OFFSET];
								//uart_recv.uart_union.data_str.length_h=buff[PACKET_DATA_OFFSET];
								//uart_recv.uart_union.data_str.length_l=buff[PACKET_DATA_OFFSET+1];
								 uart_recv.cos_revDataTotalLen +=CMD_PACKET_BASE_LENGTH;
								uart_recv.cos_revDataLen =0;

								uart_recv.cos_protocol_rev_data_ready=1;
			             if(buff[PACKET_CMD_OFFSET]==CMD_COS_DATA)
			             {
#if ENABLE_COS              
			                simple_uart_init(UART_POWEROFF_BT_DELAY);
#endif             
			             }
							}
					  /*}
						else
					  {
             Protocol_handle_statusPacket(buff,temp_length);
						}*/

				 break;
         case PACKET_MARK_MULTI:
						uart_recv.cos_revDataTotalLen=buff[PACKET_DATA_OFFSET]<<8 | buff[PACKET_DATA_OFFSET+1];			
						uart_recv.cos_uart_union.cos_data_str.cos_head=PACKET_DATA_HEAD;
						uart_recv.cos_uart_union.cos_data_str.cos_CMD=buff[PACKET_CMD_OFFSET];
						uart_recv.cos_uart_union.cos_data_str.cos_length_h=buff[PACKET_DATA_OFFSET];
						uart_recv.cos_uart_union.cos_data_str.cos_length_l=buff[PACKET_DATA_OFFSET+1];
						uart_recv.cos_revDataTotalLen +=CMD_PACKET_BASE_LENGTH;
						uart_recv.cos_revDataLen =CMD_PACKET_BASE_LENGTH-2;
						uart_recv.cos_protocol_rev_data_ready=0;
					break;
					case PACKET_MARK_NEXT:		  
            if((uart_recv.cos_revDataLen+temp_length-CMD_PACKET_BASE_LENGTH)<PACKET_COS_MAX_BYTE)
            {
						memcpy((uart_recv.cos_uart_union.cos_taltol_data+uart_recv.cos_revDataLen),buff+PACKET_DATA_OFFSET,temp_length-CMD_PACKET_BASE_LENGTH);
						uart_recv.cos_revDataLen	+=temp_length-CMD_PACKET_BASE_LENGTH;
						uart_recv.cos_protocol_rev_data_ready=0;
            }
					break;
					case PACKET_MARK_COMPLETE:
					{
            if((uart_recv.cos_revDataLen+temp_length-PACKET_DATA_OFFSET)<PACKET_COS_MAX_BYTE)//kevin updated 151106
            {
                memcpy((uart_recv.cos_uart_union.cos_taltol_data+uart_recv.cos_revDataLen),buff+PACKET_DATA_OFFSET,temp_length-PACKET_DATA_OFFSET);
                uart_recv.cos_revDataLen  +=temp_length-PACKET_DATA_OFFSET;

                if(uart_recv.cos_revDataTotalLen==uart_recv.cos_revDataLen)
                {
                  uart_recv.cos_uart_union.cos_taltol_data[uart_recv.cos_revDataTotalLen-2]=Get_Datacheck(&uart_recv.cos_uart_union.cos_taltol_data[1],uart_recv.cos_revDataTotalLen-3);//check byte
                  uart_recv.cos_uart_union.cos_taltol_data[uart_recv.cos_revDataTotalLen-1]=0x55;//tail byte
                  uart_recv.cos_protocol_rev_data_ready=1;
                  if(buff[PACKET_CMD_OFFSET]==CMD_COS_DATA)
                   {
#if ENABLE_COS                     
                      simple_uart_init(UART_POWEROFF_BT_DELAY);
#endif
                   }
                  return;

                 
                }
                else
                {
                  Protocol_Packet_Status(buff,CMD_STATUS_CHECK_ERROR,ser_flag);
 
                }	
            }
            else
            {
                Protocol_Packet_Status(buff,CMD_STATUS_DATALENGTH_FAIL,ser_flag);

            }  
						uart_recv.cos_protocol_rev_data_ready=0;
					}						
					break;  		   
					default:
						Protocol_Packet_Status(buff,CMD_STATUS_CHECK_ERROR,ser_flag);
	   	
					break;
		}

	
}/*
void Save_protocol_data(UINT8 * data_prt,UINT16 packet_length)
{
  static uint16_t ble_rev_index = 0;
static uint16_t ble_send_index = 0;
  for(unsigned char len=0;len<packet_length;len++)
  {
   ble_rev_data_buff[ble_rev_index]==data_prt[len];
   ble_rev_index++;
   if(ble_rev_index>=PACKET_COS_MAX_BYTE)
    ble_rev_index=0;
  }
 
}*/
void Protocol_data_manage(UINT8 * data_prt,UINT16 packet_length)
{
    //UINT16 temp_length=0;

     //   temp_length=packet_length;



    if(Protocol_check_data(data_prt,packet_length))
    {
	     if(theSink.BLE_connected==0)
			 {
          Protocol_set_BT_connected();  
       }
       protocol_Save_data(data_prt,packet_length,SERVICE_COS);    
	}
	else
	{
       Protocol_Packet_Status(data_prt,CMD_STATUS_CHECK_ERROR,SERVICE_COS);

	}
}
//===================new service===========================
void newService_protocol_data_can_send(void)
{
    if(new_recv.protocol_rev_data_ready==1)
    {
      new_recv.protocol_rev_data_ready=0;
			Protocol_handle(new_recv.uart_union.taltol_data,new_recv.revDataTotalLen,SERVICE_STEP);		
	}
}

static void newService_protocol_Save_data(UINT8 *buff,UINT16 temp_length,UINT8 ser_flag)
{
   	  switch(buff[PACKET_LENGTH_OFFSET] & PACKET_MARK_FLAG)
   	  {
         case PACKET_MARK_FULL:	
					 /*if(buff[PACKET_CMD_OFFSET]!=CMD_STATUS)
					 {*/
						 if(buff[PACKET_CMD_OFFSET]==CMD_GET_MTU)
						 {
							 Protocol_handle(buff,temp_length,ser_flag);
						 }
						 else
						 {
								 memcpy((new_recv.uart_union.taltol_data),buff,temp_length);
								 new_recv.revDataTotalLen=buff[PACKET_LENGTH_OFFSET]<<8 | buff[PACKET_LENGTH_OFFSET+1];		 
							 //new_recv.uart_union.data_str.head=PACKET_DATA_HEAD;
							 //new_recv.uart_union.data_str.CMD=buff[PACKET_CMD_OFFSET];
							 //new_recv.uart_union.data_str.length_h=buff[PACKET_DATA_OFFSET];
							 //new_recv.uart_union.data_str.length_l=buff[PACKET_DATA_OFFSET+1];
								new_recv.revDataTotalLen +=CMD_PACKET_BASE_LENGTH;
							 new_recv.revDataLen =0;

							 new_recv.protocol_rev_data_ready=1;
								/*	if(buff[PACKET_CMD_OFFSET]==CMD_COS_DATA)
									{
#if ENABLE_COS              
										 simple_uart_init(UART_POWEROFF_BT_DELAY);
#endif             
									}*/
						 }



				 break;
         case PACKET_MARK_MULTI:
						new_recv.revDataTotalLen=buff[PACKET_DATA_OFFSET]<<8 | buff[PACKET_DATA_OFFSET+1];			
						new_recv.uart_union.data_str.head=PACKET_DATA_HEAD;
						new_recv.uart_union.data_str.CMD=buff[PACKET_CMD_OFFSET];
						new_recv.uart_union.data_str.length_h=buff[PACKET_DATA_OFFSET];
						new_recv.uart_union.data_str.length_l=buff[PACKET_DATA_OFFSET+1];
						new_recv.revDataTotalLen +=CMD_PACKET_BASE_LENGTH;
						new_recv.revDataLen =CMD_PACKET_BASE_LENGTH-2;
						new_recv.protocol_rev_data_ready=0;
					break;
					case PACKET_MARK_NEXT:		  
            if((new_recv.revDataLen+temp_length-CMD_PACKET_BASE_LENGTH)<PACKET_STEP_MAX_BYTE)
            {
						memcpy((new_recv.uart_union.taltol_data+new_recv.revDataLen),buff+PACKET_DATA_OFFSET,temp_length-CMD_PACKET_BASE_LENGTH);
						new_recv.revDataLen	+=temp_length-CMD_PACKET_BASE_LENGTH;
						new_recv.protocol_rev_data_ready=0;
            }
					break;
					case PACKET_MARK_COMPLETE:
					{
            if((new_recv.revDataLen+temp_length-PACKET_DATA_OFFSET)<PACKET_STEP_MAX_BYTE)//kevin updated 151106
            {
                memcpy((new_recv.uart_union.taltol_data+new_recv.revDataLen),buff+PACKET_DATA_OFFSET,temp_length-PACKET_DATA_OFFSET);
                new_recv.revDataLen  +=temp_length-PACKET_DATA_OFFSET;

                if(new_recv.revDataTotalLen==new_recv.revDataLen)
                {
                  new_recv.uart_union.taltol_data[new_recv.revDataTotalLen-2]=Get_Datacheck(&new_recv.uart_union.taltol_data[1],new_recv.revDataTotalLen-3);//check byte
                  new_recv.uart_union.taltol_data[new_recv.revDataTotalLen-1]=0x55;//tail byte
                  new_recv.protocol_rev_data_ready=1;
                  /*if(buff[PACKET_CMD_OFFSET]==CMD_COS_DATA)
                   {
#if ENABLE_COS                     
                      simple_uart_init(UART_POWEROFF_BT_DELAY);
#endif
                   }*/
                  return;

                 
                }
                else
                {
                  Protocol_Packet_Status(buff,CMD_STATUS_CHECK_ERROR,ser_flag);

                }	
            }
            else
            {
                Protocol_Packet_Status(buff,CMD_STATUS_DATALENGTH_FAIL,ser_flag);

            }  
						new_recv.protocol_rev_data_ready=0;
            
					}						
					break;  		   
					default:
						Protocol_Packet_Status(buff,CMD_STATUS_CHECK_ERROR,ser_flag);
  	
					break;
		}

	
}

void newService_Protocol_data_manage(UINT8 * data_prt,UINT16 packet_length)
{
    //UINT16 temp_length=0;
    //temp_length=packet_length;
    if(Protocol_check_data(data_prt,packet_length))
    {
	     if(theSink.BLE_connected==0)
			 {
          Protocol_set_BT_connected();  
       }
       newService_protocol_Save_data(data_prt,packet_length,SERVICE_STEP);       
    }
    else
    {
         Protocol_Packet_Status(data_prt,CMD_STATUS_CHECK_ERROR,SERVICE_STEP);

    }
}




