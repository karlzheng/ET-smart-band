#include "ET1502B.h"
#include "data_transfer.h"
#if DATA_TRANSFER_FEATURE_ENABLE
#include "data_manger.h"
#include "arithmetic_data_manger.h"
#include "btprotocol.h"
#include "ke_timer.h"
#include "usr_task.h"
#include "sleep_data.h"
#include "et_debug.h"
#include <string.h>
//#include"uartDrv.h"
#define SEND_TIME_OUT_TIME_COUNT		(10)
#define SEND_DATA_TIME_OUT_TIME			4//3000//(200)	//1.5 S
//#define AUTO_SEND_DATA_TIME				1000//(100)//(5*100)	//5S
//#define RESET_SEND_DATA_TIME			(60*100)

unsigned char timeOut_count = 0;
unsigned char readDataSouce = 0;
unsigned char gAppGetDataFlg = 0;

unsigned int ReciveTime=0;/*(秒) 接收到数据的时间参数*/
unsigned int FlashTime=0; /*Flash 中记录的时间参数*/
unsigned char Read_SportBlock=0;/*多少个256BYTE*/
unsigned char Read_SleepBlock=0;/*多少个256BYTE*/

//unsigned char Read_StartRecord=0;/*多少条16BYTE记录*/
unsigned char DateNumer=0;/*总共取多少天记录*/





unsigned char get_ble_connect_status(void)
{
	return ((theSink.BLE_connected) && (theSink.notify_ok)); //  0:disconnection   1:connecttion
}

unsigned char get_auto_send_data_to_app_status(void)
{
	return theSink.AutoSend_TadayDataFlg;
}

unsigned char get_auto_send_SleepData_to_app_status(void)
{
	return theSink.AutoSend_SleepDataFlg;
}

void send_sport_get_app_ack(unsigned char * data,unsigned short length)
{
	unsigned char ackFlg = data[5];//data[4]
	if(ackFlg==0)//ack ok
	{
		timeOut_count = 0;
		ke_timer_clear(USR_SEND_DATA_TIMEOUT_TIMER);
		theSink.AutoSend_TadayDataFlg=1;
	}	
}
void send_sleep_get_app_ack(unsigned char * data,unsigned short length)
{
	unsigned char ackFlg = data[5];//data[4]
	if(ackFlg==0)//ack ok
	{
		timeOut_count = 0;
		ke_timer_clear(USR_SEND_DATA_TIMEOUT_TIMER);
		theSink.AutoSend_SleepDataFlg=1;
	}	
}

void send_histy_sport_record_get_app_ack(unsigned char * data,unsigned short length)
{
	//unsigned char ackFlg = data[5];//data[4]
	if(data[5]==0)//ack ok
	{
		//timeOut_count = 0;
		ke_timer_clear(USR_SEND_DATA_TIMEOUT_TIMER);
		theSink.AutoSend_HistoricalDataFlg=1;
	}	
}



void send_data_timeout_timer(unsigned short const msgid)
{
	/*timeOut_count++;

	if(timeOut_count < SEND_TIME_OUT_TIME_COUNT)
	{
		ke_timer_set(USR_AUTO_SEND_DATA_TO_APP_TIMER,2);
	}
	else
	{
		timeOut_count = 0;
		gAppGetDataFlg = 0;
		ke_timer_set(USR_AUTO_SEND_DATA_TO_APP_TIMER,RESET_SEND_DATA_TIME);
	}
  */ //kevin delete
  //auto_send_sport_record_to_app_timer();//kevin add
}

#if 0
#if ((DEBUG_UART_EN) )
	UINT16 K;
       DbgPrintfbuff("pbuf:",strlen("pbuf:"));
      for(K=0;K<256;K++)
      DbgPrintf("%x ",pbuf[K]);
     DbgPrintfbuff("\r\n",2);
#endif 
#endif

#if 0
/****************Auto sport data to app**************************/
unsigned char auto_send_sport_record_to_app_timer(void)
{
	unsigned char pbuf[256]={0};
	data_block_t* pRamBlock = get_ram_block(DATA_TYPE_SPORT);
	if(get_ble_connect_status()&&get_auto_send_data_to_app_status()&&theSink.SportDataFlg_over)
	{	
		if(Read_SportBlock<16)
		{
			read_flash((DEV_GSENSOR_SPORT_DATA_START_ADDR+FLASH_RW_BLOCK_SIZE*Read_SportBlock), pbuf, FLASH_RW_BLOCK_SIZE);

			Read_SportBlock=Read_SportBlock+1;
			if((pbuf[0]==0x1A)&&(pbuf[1]==0x2B)&&(pbuf[2]==0x3C)&&(pbuf[3]==0x4D))
			{	
				theSink.AutoSend_TadayDataFlg=0;
				#if SERVICE_STEP_EN
				Protocol_send_sport_data_to_ag(pbuf,(pbuf[7]+1)*16,SERVICE_STEP);/*发送*/
				#else
				Protocol_send_sport_data_to_ag(pbuf,(pbuf[7]+1)*16,SERVICE_COS);
				#endif
				ke_timer_set(USR_SEND_DATA_TIMEOUT_TIMER,SEND_DATA_TIME_OUT_TIME);
			}
			else
			{
				theSink.AutoSend_TadayDataFlg=0;
				Read_SportBlock=0;
				theSink.SportDataFlg_over=0;
				if(pRamBlock->head->dataItemNums)/*发送RAM 里的片段历史数据*/
				{	
					data_manger_set_previous_head(pRamBlock->head->previousHead);/*加命令包头*/
					#if SERVICE_STEP_EN
					Protocol_send_sport_data_to_ag((unsigned char*)pRamBlock->head,(pRamBlock->head->dataItemNums+1)*16,SERVICE_STEP);/*STEP通道*/
					#else
					Protocol_send_sport_data_to_ag((unsigned char*)pRamBlock->head,(pRamBlock->head->dataItemNums+1)*16,SERVICE_COS);/*COS 通道*/
					#endif
					ke_timer_set(USR_SEND_DATA_TIMEOUT_TIMER,SEND_DATA_TIME_OUT_TIME);
				}
			}
		}
		else
		{
			theSink.AutoSend_TadayDataFlg=0;
			theSink.SportDataFlg_over=0;
			Read_SportBlock=0;
		}
	}
	return 0;
}
#endif

#if 1
/****************Auto sport data to app**************************/
unsigned char auto_send_sport_record_to_app_timer(void)
{
	unsigned char pbuf[256]={0};
	data_block_t* pRamBlock = get_ram_block(DATA_TYPE_SPORT);


	if(get_ble_connect_status()==0)
	{
		theSink.AutoSend_TadayDataFlg=0;
		theSink.SportDataFlg_over=0;
		Read_SportBlock=0;
		return 0;
	}
/*
#if ((DEBUG_UART_EN) )
       DbgPrintfbuff("Read_SportBlock:",strlen("Read_SportBlock:"));
      DbgPrintf("%x ",Read_SportBlock);
     DbgPrintfbuff("\r\n",2);
#endif 
*/

	if((theSink.SportDataFlg_end==1)&&(theSink.AutoSend_TadayDataFlg==1))
	{
			#if SERVICE_STEP_EN
			Protocol_send_sport_data_to_ag(0,0,SERVICE_STEP);
			#else
			Protocol_send_sport_data_to_ag(0,0,SERVICE_COS);
			#endif
			theSink.AutoSend_TadayDataFlg=0;
			theSink.SportDataFlg_over=0;
			theSink.SportDataFlg_end=0;
			Read_SportBlock=0;
			return 1;
	}
	if(get_auto_send_data_to_app_status()&&theSink.SportDataFlg_over)
	{	
		if(Read_SportBlock<16)
		{
			read_flash((DEV_GSENSOR_SPORT_DATA_START_ADDR+FLASH_RW_BLOCK_SIZE*Read_SportBlock), pbuf, FLASH_RW_BLOCK_SIZE);

			
			if((pbuf[0]==0x1A)&&(pbuf[1]==0x2B)&&(pbuf[2]==0x3C)&&(pbuf[3]==0x4D))
			{	
			  Read_SportBlock=Read_SportBlock+1;
				theSink.AutoSend_TadayDataFlg=0;
				#if SERVICE_STEP_EN
				Protocol_send_sport_data_to_ag(pbuf,(pbuf[7]+1)*16,SERVICE_STEP);/*发送*/
				#else
				Protocol_send_sport_data_to_ag(pbuf,(pbuf[7]+1)*16,SERVICE_COS);
				#endif
				ke_timer_set(USR_SEND_DATA_TIMEOUT_TIMER,SEND_DATA_TIME_OUT_TIME);
			}
			else
			{
				if(pRamBlock->head->dataItemNums)/*发送RAM 里的片段历史数据*/
				{	
					theSink.AutoSend_TadayDataFlg=0;
					data_manger_set_previous_head(pRamBlock->head->previousHead);/*加命令包头*/
					#if SERVICE_STEP_EN
					Protocol_send_sport_data_to_ag((unsigned char*)pRamBlock->head,(pRamBlock->head->dataItemNums+1)*16,SERVICE_STEP);/*STEP通道*/
					#else
					Protocol_send_sport_data_to_ag((unsigned char*)pRamBlock->head,(pRamBlock->head->dataItemNums+1)*16,SERVICE_COS);/*COS 通道*/
					#endif
					ke_timer_set(USR_SEND_DATA_TIMEOUT_TIMER,SEND_DATA_TIME_OUT_TIME);
				}
				Read_SportBlock=0;
				theSink.SportDataFlg_over=0;
				theSink.SportDataFlg_end=1;
			}
		}
		else
		{
			if(pRamBlock->head->dataItemNums)/*发送RAM 里的片段历史数据*/
			{ 
				theSink.AutoSend_TadayDataFlg=0;
				data_manger_set_previous_head(pRamBlock->head->previousHead);/*加命令包头*/
				#if SERVICE_STEP_EN
				Protocol_send_sport_data_to_ag((unsigned char*)pRamBlock->head,(pRamBlock->head->dataItemNums+1)*16,SERVICE_STEP);/*STEP通道*/
				#else
				Protocol_send_sport_data_to_ag((unsigned char*)pRamBlock->head,(pRamBlock->head->dataItemNums+1)*16,SERVICE_COS);/*COS 通道*/
				#endif
				ke_timer_set(USR_SEND_DATA_TIMEOUT_TIMER,SEND_DATA_TIME_OUT_TIME);
			}

			theSink.AutoSend_TadayDataFlg=0;
			theSink.SportDataFlg_over=0;
			Read_SportBlock=0;
		}
	}
	return 1;
}
#endif


/***************Auto send sleep data*******************************************/
#if 1
unsigned char auto_send_sleep_record_to_app_timer(void)
{
	unsigned char pbuf[256]={0};
	//data_block_t* pRamBlock = get_ram_block(DATA_TYPE_SLEEP);

	if(get_ble_connect_status()==0)
	{
		theSink.AutoSend_SleepDataFlg=0;
		theSink.SleepDataFlg_over=0;
		Read_SleepBlock=0;
		return 0;
	}


	if((theSink.SleepDataFlg_end==1)&& get_auto_send_SleepData_to_app_status())
		{
			#if SERVICE_STEP_EN
				Protocol_send_sleep_data_to_ag(0,0,SERVICE_STEP);
			#else
				Protocol_send_sleep_data_to_ag(0,0,SERVICE_COS);
			#endif
				theSink.AutoSend_SleepDataFlg=0;
				theSink.SleepDataFlg_over=0;
				theSink.SleepDataFlg_end=0;
				Read_SleepBlock=0;
				return 1;
		}


	if(get_auto_send_SleepData_to_app_status()&&(theSink.SleepDataFlg_over==1))
	{
		//if(Read_SleepBlock<SLEEP_DATA_BLOCK_NUM)
		//{
			//read_flash((DEV_GSENSOR_SLEEP_DATA_START_ADDR+FLASH_RW_BLOCK_SIZE*Read_SleepBlock),pbuf,FLASH_RW_BLOCK_SIZE);
			//Read_SleepBlock=Read_SleepBlock+1;
      UINT8 flag=0;
      UINT8 PAGE_NUM=0;
      UINT8 retu_len=0;
      theSink.SleepDataFlg_over=0;
      retu_len=get_sleep_state_from_ram(pbuf,256,&flag,Read_SleepBlock,&PAGE_NUM);
      Read_SleepBlock=retu_len;
			/*if((pbuf[0]==0x1A)&&(pbuf[1]==0x2B)&&(pbuf[2]==0x3C)&&(pbuf[3]==0x4D))*/
      if(retu_len>0)
			{	
				//theSink.AutoSend_SleepDataFlg=0;
        //theSink.SleepDataFlg_over=0;
        if(retu_len>32)
          retu_len=32;
        #if SERVICE_STEP_EN
        {
          if(flag==0)
          {
          Protocol_send_sleep_data_to_ag(pbuf,256,SERVICE_STEP);/*send*/
          }
          else
          {
           theSink.SleepDataFlg_end=1; 
          Protocol_send_sleep_data_to_ag(pbuf,retu_len*8,SERVICE_STEP);/*send*/
          }
        }
				#else
        {
          if(flag==0)
          {
          Protocol_send_sleep_data_to_ag(pbuf,256,SERVICE_COS);/*send*/
          }
          else
          {
            theSink.SleepDataFlg_end=1;
          Protocol_send_sleep_data_to_ag(pbuf,retu_len*8,SERVICE_COS);/*send*/
          }
        }
				#endif      
        //#if 0
				//#if SERVICE_STEP_EN
				//Protocol_send_sleep_data_to_ag(pbuf,(pbuf[7]+1)*16,SERVICE_STEP);//send
				//#else
				//Protocol_send_sleep_data_to_ag(pbuf,(pbuf[7]+1)*16,SERVICE_COS);//send
				//#endif
				//ke_timer_set(USR_SEND_DATA_TIMEOUT_TIMER,SEND_DATA_TIME_OUT_TIME);//time out
        #endif
			}
      else
      {
        //theSink.AutoSend_SleepDataFlg=0;
        //theSink.SleepDataFlg_over=0;
        theSink.SleepDataFlg_end=1;
        Read_SleepBlock=0;
      }
			/*else
			{
				if(pRamBlock->head->dataItemNums)
				{	
					theSink.AutoSend_SleepDataFlg=0;
					data_manger_set_previous_head(pRamBlock->head->previousHead);//add the head
					#if SERVICE_STEP_EN
					Protocol_send_sleep_data_to_ag((unsigned char*)pRamBlock->head,(pRamBlock->head->dataItemNums+1)*16,SERVICE_STEP);//STEP通道
					#else
					Protocol_send_sleep_data_to_ag((unsigned char*)pRamBlock->head,(pRamBlock->head->dataItemNums+1)*16,SERVICE_COS);//COS 通道
					#endif
					ke_timer_set(USR_SEND_DATA_TIMEOUT_TIMER,SEND_DATA_TIME_OUT_TIME);
				}
				theSink.SleepDataFlg_over=0;
				theSink.SleepDataFlg_end=1;
				Read_SleepBlock=0;
			}*/
		//}
		//else
		//{
		//	theSink.AutoSend_SleepDataFlg=0;
		//	theSink.SleepDataFlg_over=0;
		//	theSink.SleepDataFlg_end=1;
		//	Read_SleepBlock=0;
		//}
	}
	return 1;
}
#endif

void auto_send_history_TotalSleepRecord(void)
{
	unsigned char pbuf[256]={0};
	//data_block_t* pRamBlock = get_ram_block(DATA_TYPE_SLEEP);

	if(get_ble_connect_status()==0)
	{
		theSink.AutoSend_EveryDataTotalSleepRecordFlg=0;
		theSink.EveryDataTotalSleepRecord_over=0;
    theSink.EveryDataTotalSleepRecord_end=0;
	}


	if((theSink.EveryDataTotalSleepRecord_end==1)&& (theSink.AutoSend_EveryDataTotalSleepRecordFlg==1))
		{
			#if SERVICE_STEP_EN
				Protocol_send_everyDayTotalSleepData_to_ag(0,0,SERVICE_STEP);
			#else
				Protocol_send_everyDayTotalSleepData_to_ag(0,0,SERVICE_COS);
			#endif
		theSink.AutoSend_EveryDataTotalSleepRecordFlg=0;
		theSink.EveryDataTotalSleepRecord_over=0;
    theSink.EveryDataTotalSleepRecord_end=0;

		}


	if((theSink.AutoSend_EveryDataTotalSleepRecordFlg==1)&&(theSink.EveryDataTotalSleepRecord_over==1))
	{
    
     // UINT8 flag=0;
      //UINT8 PAGE_NUM=0;
      UINT8 retu_len=0;
      theSink.EveryDataTotalSleepRecord_over=0;
      read_sleep_record_from_flash(pbuf,256);

			if((pbuf[0]==0x1A)&&(pbuf[1]==0x2B)&&(pbuf[2]==0x3C)&&(pbuf[3]>0))
			{	
        retu_len=pbuf[3];
        if(retu_len>15)
          retu_len=15;
        #if SERVICE_STEP_EN
          Protocol_send_everyDayTotalSleepData_to_ag(pbuf+16,retu_len*16,SERVICE_STEP);/*send*/
				#else
          Protocol_send_everyDayTotalSleepData_to_ag(pbuf+16,retu_len*16,,SERVICE_COS);/*send*/

				#endif     

			}
      else
      {
        
        theSink.EveryDataTotalSleepRecord_end=1;
      }
			
	}


}
/**********************************************************************/

/*发送历史每天运动数据累计值*/
#if 0
unsigned char auto_send_histy_sport_record_to_app_timer(void)
{	
	unsigned char pbuf[256]={0};
	unsigned char Head[16]={0};
	//unsigned char headHistry[16]={0x1A, 0x2B, 0x3C, 0x4D,0x01,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	unsigned int Addr=0;
	if(get_ble_connect_status())
	{	
		if((theSink.AutoSend_HistoricalDataFlg_end)&&(theSink.AutoSend_HistoricalDataFlg)) //kevin add
		{
			#if SERVICE_STEP_EN
			Protocol_send_histry_sport_data_to_ag(0,0,SERVICE_STEP);
			#else
			Protocol_send_histry_sport_data_to_ag(0,0,SERVICE_COS);
			#endif
			theSink.AutoSend_HistoricalDataFlg=0;
			theSink.AutoSend_HistoricalDataFlg_end=0;
			return 0;
		}	
		if(theSink.AutoSend_HistoricalDataFlg)
		{
			Addr=DEV_GSENSOR_SPORT_Histry_DATA_Head_ADDR;
			read_flash(Addr,Head,16);
			//if (0 != memcmp(pbuf, Bbuf, 4))/*memcmp 相等返回0*/
			if(!((Head[0]==0x1A)&&(Head[1]==0x2B)&&(Head[2]==0x3C)&&(Head[3]==0x4D)))//g_DataHeadLabel
			{
				theSink.AutoSend_HistoricalDataFlg=0;
				//theSink.AutoSend_HistoricalDataFlg_end=1;//kevin add	
				#if SERVICE_STEP_EN
				Protocol_send_histry_sport_data_to_ag(0,0,SERVICE_STEP);
				#else
				Protocol_send_histry_sport_data_to_ag(0,0,SERVICE_COS);
				#endif				
				return 0;
			}
			if((DateNumer==0)||(Head[7]<DateNumer))
			{
				DateNumer=Head[7];
			}	
		
			Addr=DEV_GSENSOR_SPORT_Histry_DATA_START_ADDR+(Head[7]-DateNumer)*16;
			read_flash(Addr, &pbuf[16], DateNumer*16);
			Head[7]=DateNumer;
			memcpy(pbuf,Head,16);
			#if SERVICE_STEP_EN
			Protocol_send_histry_sport_data_to_ag(pbuf,(pbuf[7]+1)*16,SERVICE_STEP);/*发送*/
			#else
			Protocol_send_histry_sport_data_to_ag(pbuf,(pbuf[7]+1)*16,SERVICE_COS);
			#endif
			theSink.AutoSend_HistoricalDataFlg=0;
			theSink.AutoSend_HistoricalDataFlg_end=1;
			return 1;
		}
		/*if(theSink.AutoSend_HistoricalDataFlg_end)
		{
			#if SERVICE_STEP_EN
			Protocol_send_histry_sport_data_to_ag(0,0,SERVICE_STEP);
			#else
			Protocol_send_histry_sport_data_to_ag(0,0,SERVICE_COS);
			#endif
			theSink.AutoSend_HistoricalDataFlg=0;
			theSink.AutoSend_HistoricalDataFlg_end=0;
			return 1;
		}*/
	}
	else
	{
		theSink.AutoSend_HistoricalDataFlg=0;
		theSink.AutoSend_HistoricalDataFlg_end=0;
		return 0;
	}
  return 0;
}
#endif
#if 1//new

unsigned char auto_send_histy_sport_record_to_app_timer(void)
{	
	unsigned char pbuf[256]={0};
	//unsigned char Head[16]={0};
	//unsigned char headHistry[16]={0x1A, 0x2B, 0x3C, 0x4D,0x01,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	unsigned int Addr=0;
	if(get_ble_connect_status())
	{	
		if((theSink.AutoSend_HistoricalDataFlg_end)&&(theSink.AutoSend_HistoricalDataFlg)) //kevin add
		{
			#if SERVICE_STEP_EN
			Protocol_send_histry_sport_data_to_ag(0,0,SERVICE_STEP);
			#else
			Protocol_send_histry_sport_data_to_ag(0,0,SERVICE_COS);
			#endif
			theSink.AutoSend_HistoricalDataFlg=0;
			theSink.AutoSend_HistoricalDataFlg_end=0;
			return 0;
		}	
		if(theSink.AutoSend_HistoricalDataFlg)
		{
			Addr=DEV_GSENSOR_SPORT_Histry_DATA_START_ADDR;
			read_flash(Addr,pbuf,256);
			if(!((pbuf[0]==0x1A)&&(pbuf[1]==0x2B)&&(pbuf[2]==0x3C)&&(pbuf[3]==0x4D)))//g_DataHeadLabel
			{
				theSink.AutoSend_HistoricalDataFlg=0;
				//theSink.AutoSend_HistoricalDataFlg_end=1;//kevin add		
				#if SERVICE_STEP_EN
				Protocol_send_histry_sport_data_to_ag(0,0,SERVICE_STEP);
				#else
				Protocol_send_histry_sport_data_to_ag(0,0,SERVICE_COS);
				#endif				
				return 0;
			}
			//if((DateNumer==0)||(Head[7]<DateNumer))
			//{
			//	DateNumer=Head[7];
			//}	
		
			//Addr=DEV_GSENSOR_SPORT_Histry_DATA_START_ADDR+(Head[7]-DateNumer)*16;
			//read_flash(Addr, &pbuf[16], DateNumer*16);
			//Head[7]=DateNumer;
			//memcpy(pbuf,Head,16);
			#if SERVICE_STEP_EN
			Protocol_send_histry_sport_data_to_ag(pbuf,(pbuf[7]+1)*16,SERVICE_STEP);/*??*/
			#else
			Protocol_send_histry_sport_data_to_ag(pbuf,(pbuf[7]+1)*16,SERVICE_COS);
			#endif
			theSink.AutoSend_HistoricalDataFlg=0;
			theSink.AutoSend_HistoricalDataFlg_end=1;
			return 1;
		}
	}
	else
	{
		theSink.AutoSend_HistoricalDataFlg=0;
		theSink.AutoSend_HistoricalDataFlg_end=0;
		return 0;
	}
  return 0;
}
#endif



void receive_app_usr_info(unsigned char * data,unsigned short length)
{
	if(data[3] >=8 )//receive usr info ok
	{
		receive_userInfo_from_app(&data[4]);
	}
}
void Get_app_usr_info(UINT8 ser_flag)
{
	unsigned char AckBuf[12];
	Get_userInfo_from_app(AckBuf);
	Protocol_send_usr_info(AckBuf,12,ser_flag);
}
#if FREE_NOTIC_ARITHMETIC

void receive_app_free_notic_info(unsigned char * data,unsigned short length)
{
	unsigned char AckBuf[2]={0x00,0x00};
	AckBuf[0] = CMD_FREE_NOTIC_INFO;//ack cmd
	if(length == 11 )//receive usr info ok
	{
		receive_freeInfo_from_app(&data[4]);
		AckBuf[1] = 0x00;
	}
	else
	{
		AckBuf[1] = 0x01;
	}
	Protocol_send_ack_to_ag_by_usr_info(AckBuf,2);
}
#endif




