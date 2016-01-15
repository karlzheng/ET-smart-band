#include "ET1502B.h"
#include "arithmetic_data_manger.h"
#if WALK_STEP_RECORD_ARITHMETIC
#include <string.h>
#include "app_arithmetic.h"
#include "arithmetic.h"
#include "et_debug.h"
#include "sys_time.h"
#include "data_manger.h"
#include "sleep_record.h"
#include "data_transfer.h"
#include "free_notic.h"
#include "common_arithmetic.h"
#include "btprotocol.h"



user_info_struct 	g_user = {0,"123",0,25,170,60,0x00,0x00,0x27,0x10,ARITHMETIC_STEPS_HAND};//{}

walk_info_t g_walkInfoStruct = {0,0,0,0,0,SPORT_MODE_MAX};/*最新显示数据*/

sport_info_struct g_sportInfoDMItem = {0};
//sport_info_struct g_sportInfoDMItemSave = {0};

static arth_personal_info g_arthPersonalInfo = {60,70,ARITHMETIC_STEPS_HAND};

//static unsigned int todayEndSecBuf = 0;

//s_tm curTime_Temp;
//	system_time_get(&curTime_Temp);
//static unsigned char Step_can_Save=0;

unsigned int app_arthmetic_get_total_steps(void)
{
	return g_walkInfoStruct.steps;
}

unsigned int app_arthmetic_get_total_calorie(void)
{
	return g_walkInfoStruct.calorie/100;
}

unsigned int app_arthmetic_get_total_distance(void)
{
	return g_walkInfoStruct.distance;
}

unsigned char app_arithmetic_get_left_right_hand(void)
{
	return g_arthPersonalInfo.left_rifht_hand;
}

static unsigned short steps_2_distance(unsigned char sportMode, unsigned short steps)
{
	unsigned short distance = 0;

	if(steps == 0)
		return 0;

	distance = steps * g_arthPersonalInfo.stepDisInterval;

	if(distance%100 >= 50)
		distance = distance/100 + 1;
	else 
		distance = distance/100;
	
//	DbgPrintf("steps_2_distance: sportMode = %d,distance = %d,steps=%d,stepDisInterval=%d, hight = %d\r\n",
//		sportMode,distance,steps,g_arthPersonalInfo.stepDisInterval,g_user.hight);
	return distance;
}

static unsigned short steps_2_calorie(unsigned char sportMode, unsigned short steps, unsigned int daltSec)
{
	volatile unsigned short speed = 0, calorie = 0;
	unsigned short distance = 0;
		
	if(steps == 0 || daltSec == 0)
		return 0;

	distance = steps * g_arthPersonalInfo.stepDisInterval;

	speed = distance/daltSec; // xx cm/s
	
	if(sportMode == SPORT_MODE_WALK)
		calorie = ((5 * speed * 3600)/4)/1000;	
	else if(sportMode == SPORT_MODE_RUN)
		calorie = ((((5 * speed * 3600)/4)/1000)*14)/10;  
	else
		calorie = ((((5 * speed * 3600)/4)/1000)*12)/10;

	calorie = (calorie * g_arthPersonalInfo.weight);  // because distance unit is meter

	if(sportMode == SPORT_MODE_WALK)
		calorie = calorie * 10/16;
//	calorie = (calorie*g_arthPersonalInfo.calorieMultiplyPercentage)/100;// the number disp on lcd must div by 100

	if(calorie%100 >= 50)
		calorie = calorie/100 + 1;
	else 
		calorie = calorie/100;	
	
	ARITH_INFO("steps_2_calorie: sportMode = %d,steps = %d, distance = %d,daltSec=%d,weight=%d,speed=%d,calorie=%d\r\n",
		sportMode,steps,distance,daltSec,g_arthPersonalInfo.weight,speed,calorie);
	return calorie;
}

static void set_user_info(user_info_struct* pInfo, unsigned char * pVal)
{
	pInfo->flag = 0xAA;

	memcpy((unsigned char*)pInfo->id_num,pVal,sizeof(user_info_struct)-1);
	ARITH_INFO("USR INFO: 0x%02x%02x%02x%02x. %d  %d  %d  %d\r\n",
		pInfo->id_num[3],pInfo->id_num[2],pInfo->id_num[1],pInfo->id_num[0],
		pInfo->sex,pInfo->age,pInfo->hight,pInfo->weight);
}

//all equal return 0 , other return 1 , id_num isn't equal return 2;
static unsigned char cmp_user_info(user_info_struct* pInfo1, user_info_struct* pInfo2)
{
	unsigned char equal = 0;

	if(memcmp(&pInfo1->id_num[0],&pInfo2->id_num[0],sizeof(user_info_struct)-1))
	{
		equal = 1;
	}
	if(memcmp(&pInfo1->id_num[0],&pInfo2->id_num[0],sizeof(pInfo1->id_num)))
	{
		equal = 2;
	}

	return equal;
}

static void arth_init_person_info(void)
{
	unsigned short distancePercentage;
	unsigned short stepDistanceInterval = 0;
	unsigned short footLength = 0;
	if(g_user.age == 0)
		g_user.age = 30;

	if(g_user.weight == 0)
		g_user.weight = 60;

	if(g_user.hight == 0)
		g_user.hight = 170;
	
	switch(g_user.age/10)
	{
		case 0:
			distancePercentage = 70;
			break;
		case 1:
			distancePercentage = 90;
			break;
		case 2:
		case 3:
			distancePercentage = 100;
			break;
		case 4:
			distancePercentage = 95;
			break;
		case 5:
			distancePercentage = 90;
			break;
		case 6:
			distancePercentage = 85;
			break;
		case 7:
		default:
			distancePercentage = 70;
			break;
	}

	if(g_user.sex)		//0:man , 1:woman
		distancePercentage = (distancePercentage * 95)/100;

	if(g_user.hight >= 166)
		stepDistanceInterval = (g_user.hight - g_user.hight/(7*3))/2;
	else if(g_user.hight >= 140 && g_user.hight < 166)
		stepDistanceInterval = (g_user.hight - g_user.hight/(7*2))/2;
	else
		stepDistanceInterval = (g_user.hight - (g_user.hight*2)/(7*3))/2;

	if(g_user.sex) // man
	{
		if(g_user.hight > 94)
			footLength = (g_user.hight*100 - 9355)/326;
		else
			footLength = 18;
	}
	else // woman
	{
		if(g_user.hight > 131)
			footLength = (g_user.hight*100 - 13053)/141;
		else
			footLength = 15;
	}

	if(stepDistanceInterval > footLength)
		stepDistanceInterval -= footLength;

	
	g_arthPersonalInfo.weight = g_user.weight;
	g_arthPersonalInfo.stepDisInterval = (unsigned char)((stepDistanceInterval * distancePercentage)/100);
	g_arthPersonalInfo.left_rifht_hand = g_user.left_right_hand;

	ARITH_INFO("init: hight %d, step_dis %d \r\n",g_user.hight,g_arthPersonalInfo.stepDisInterval);

}

#if 0
void receive_userInfo_from_app(unsigned char *data)
{
	unsigned char equal=0;
	user_info_struct btUserInfo;
	set_user_info(&btUserInfo,data);
	equal = cmp_user_info(&btUserInfo,&g_user);
	if(equal)
	{
		//if user name info is not equal,so clear all flash data
		if(equal == 2)
		{	
			#if DATA_MANGER_FEATURE_ENABLE
			flash_all_data_clear(); //not erase here , at connect pair
			#endif
		}
		set_user_info(&g_user,data);

		arth_init_person_info();
		
		#if DATA_MANGER_FEATURE_ENABLE
		write_infos_2_flash_first_page();
		#endif
	}
}
#endif
void receive_userInfo_from_app(unsigned char *data)
{
	unsigned char equal=0;
	user_info_struct btUserInfo;
	set_user_info(&btUserInfo,data);
	equal = cmp_user_info(&btUserInfo,&g_user);
	if(equal)
	{
		//if user name info is not equal,so clear all flash data
		set_user_info(&g_user,data);
		arth_init_person_info();
		write_infos_2_flash_first_page();
	}
}

void Get_userInfo_from_app(unsigned char *data)
{
	//unsigned char equal=0;
	//user_info_struct btUserInfo;
	//set_user_info(&btUserInfo,data);
	//equal = cmp_user_info(&btUserInfo,&g_user);
	memcpy(data,(unsigned char*)g_user.id_num,sizeof(user_info_struct)-1);
	//memcpy((unsigned char*)pInfo->id_num,pVal,sizeof(user_info_struct)-1);

}

#if FREE_NOTIC_ARITHMETIC
void receive_freeInfo_from_app(unsigned char *data)
{
	QPRINTF("receive_freeInfo_from_app\r\n");

	free_notic_info.flag = 0xAA;
	memcpy(&free_notic_info.free_enable,data,sizeof(free_notic_info)-1);
	free_notic_info.count_time = 0;
	
	#if DATA_MANGER_FEATURE_ENABLE
	write_infos_2_flash_first_page();
	#endif
}
#endif

/*g_sportInfoDMItem=p_sport_info 转换成BUFFER格式*/
static void sportInfo2DMDataItem(unsigned char type, walk_info_t *p_sport_info, unsigned int startSecs,unsigned short sustainSecs)
{
	memset(&g_sportInfoDMItem,0,sizeof(sport_info_struct));

	IntSetFourChar((char*)g_sportInfoDMItem.startSecs,startSecs);
	IntSetThreeChar((char*)g_sportInfoDMItem.walkSteps,p_sport_info->steps);
	IntSetThreeChar((char*)g_sportInfoDMItem.distance,p_sport_info->distance);
	IntSetFourChar((char*)g_sportInfoDMItem.calories,p_sport_info->calorie/100);
	ShortSetTwoChar((char*)g_sportInfoDMItem.sustainSecs,sustainSecs);
}

/*10 分钟强制保存一次数据sportInfoItem 为数据*/
static void send_sport_info2_datamanger(sport_info_struct* sportInfoItem)
{
#if DATA_MANGER_FEATURE_ENABLE
	write_data_item_2_mem_pool(DATA_TYPE_SPORT,(unsigned char*)sportInfoItem);
#endif
	g_arthStoreSec = g_arthCurSec;
}

/*存储当前运动数据到RAM   g_walkInfoStruct*/
static void app_arithmetic_save_sportinfo(const signed short steps, unsigned char sportMode, unsigned char flushFlag, signed int daltSec)
{
	volatile unsigned short distance = 0, calorie = 0;
	ARITH_INFO("app_arithmetic_save_sportinfo\r\n");
	//cal calorie and distance
	if(steps > 0)
	{
		distance = steps_2_distance(sportMode,steps);
		calorie = steps_2_calorie(sportMode,steps,daltSec);   

	#if SLEEP_RECORD_ARITHMETIC
		Sleep_Add_Interval_Steps(steps);
	#endif
		g_walkInfoStruct.steps += steps;
		g_walkInfoStruct.distance += distance;  // the number disp on lcd must div by 100
		g_walkInfoStruct.calorie += calorie;
		g_walkInfoStruct.sportMode = sportMode;
		g_walkInfoStruct.sustainSecs += daltSec;
		
		theSink.Step_Flag=1;//有STEP 标志
//		Step_can_Save=1;
		#if ARI_MODULE_LOG_EN
		unsigned char sportModeStr[3][12] = {"Walk","Run","HalfWalkRun"};
		ARITH_INFO("arith %s : add %d steps, total %d steps\r\n", sportModeStr[sportMode], steps,g_walkInfoStruct.steps);
		#endif

		sportInfo2DMDataItem(SPORT_TYPE_WALKING,&g_walkInfoStruct, 
									g_arthStoreSec,g_arthCurSec - g_arthStoreSec);	/*g_sportInfoDMItem=g_walkInfoStruct*/

		//memcpy((unsigned char*)&g_sportInfoDMItemSave,(unsigned char*)g_sportInfoDMItem.startSecs,sizeof(sport_info_struct));
	}
	if(flushFlag || (g_arthCurSec - g_arthStoreSec) >= ARTH_SPORT_SAVE2DATAMANGER_INTERVAL)/*10 分钟强制保存一次数据*/
	//if(1)
	{
		//ARITH_INFO("//////////////////////////////////////\r\n");
		//ARITH_INFO("ble disconnect sync %d steps, dalt time = %d,flushFlag = %d\r\n",g_walkInfoStruct.steps,(g_arthCurSec - g_arthStoreSec),flushFlag);
		//send_sport_info2_datamanger(&g_sportInfoDMItem);//kevin delete
	}
	

	#if 0
	#if DATA_TRANSFER_FEATURE_ENABLE   /*NICK 0*/
	if(get_ble_connect_status()&&(theSink.AutoSend_RealtimeDataFlg))//ble and APP is connection flag      /**/
	//if(get_ble_connect_status())//ble and APP is connection flag      /**/
	{
		//ARITH_INFO("//////////////////////////////////////\r\n");
		//ARITH_INFO("ble connect sync %d steps, daltSec = %d\r\n",g_walkInfoStruct.steps,daltSec);
		//send_sport_info2_datamanger(&g_sportInfoDMItem);

		/*************发送实时数据********************/
		//auto_send_sport_Live_to_app((UINT8 *)&g_sportInfoDMItem,14,SERVICE_STEP);
		/*************发送实时数据********************/
		if(flushFlag || (g_arthCurSec - g_arthStoreSec) >= ARTH_SPORT_SAVE2DATAMANGER_INTERVAL)/*10 分钟强制保存一次数据*/
		//if(flushFlag)
		{
			ARITH_INFO("//////////////////////////////////////\r\n");
			ARITH_INFO("ble disconnect sync %d steps, dalt time = %d,flushFlag = %d\r\n",g_walkInfoStruct.steps,(g_arthCurSec - g_arthStoreSec),flushFlag);
			send_sport_info2_datamanger(&g_sportInfoDMItem);
		}
	}
	else
	#endif
	{
		ARITH_INFO("%d  g_arthCurSec=%d,g_arthStoreSec=%d\r\n",g_arthCurSec-g_arthStoreSec,g_arthCurSec,g_arthStoreSec);
		//if(flushFlag || (g_arthCurSec - g_arthStoreSec) >= ARTH_SPORT_SAVE2DATAMANGER_INTERVAL)/*10 分钟强制保存一次数据*/
		if(flushFlag)
		{
			ARITH_INFO("//////////////////////////////////////\r\n");
			ARITH_INFO("ble disconnect sync %d steps, dalt time = %d,flushFlag = %d\r\n",g_walkInfoStruct.steps,(g_arthCurSec - g_arthStoreSec),flushFlag);
			send_sport_info2_datamanger(&g_sportInfoDMItem);
		}
	}
	#endif

}


signed short app_arithmetic_store_steps(unsigned char steps, unsigned char sportMode, unsigned char flushFlag,unsigned char pedometerFlg)
{

	static unsigned int lastStoreSec = 0;
	unsigned int curSec = system_sec_get();
	signed int daltSec = 0;
	ARITH_INFO("app_arithmetic_store_steps, steps = %d, sportMode = %d\r\n",steps,sportMode);
	if(lastStoreSec < g_arthStartSec)
		lastStoreSec = g_arthStartSec;

	daltSec = curSec - lastStoreSec;
	lastStoreSec = curSec;
	
	if(pedometerFlg)
	{
		app_arithmetic_save_sportinfo(steps,sportMode,flushFlag, daltSec);
	}

	return steps;
}

void app_arthmetic_clear_steps(unsigned char flag)
{
	if(g_walkInfoStruct.steps > 0)
	{
		sportInfo2DMDataItem(SPORT_TYPE_WALKING,&g_walkInfoStruct, 
								g_arthStoreSec,g_arthCurSec - g_arthStoreSec);
		//send_sport_info2_datamanger(&g_sportInfoDMItem);//kevin delete
	}

	memset(&g_walkInfoStruct,0,sizeof(walk_info_t));
	g_walkInfoStruct.sportMode = SPORT_MODE_MAX;
	memset(&g_sportInfoDMItem,0,sizeof(sport_info_struct));

	if(flag)
	{
		sportInfo2DMDataItem(SPORT_TYPE_WALKING,&g_walkInfoStruct, 
								g_arthStoreSec,g_arthCurSec - g_arthStoreSec);
		//send_sport_info2_datamanger(&g_sportInfoDMItem);//kevin delete
	}

}

void save_sport_info2_datamanger(void)
{
  if(Check_is_save_sportRecordTime()/*&&(Step_can_Save==1)*/)
  {
    unsigned int temp=0;
//		Step_can_Save=0;
	  send_sport_info2_datamanger(&g_sportInfoDMItem);//kevin delete
	  temp=system_sec_get();
	  g_sportInfoDMItem.startSecs[0]=temp&0xff;
		g_sportInfoDMItem.startSecs[1]=(temp>>8)&0xff;
		g_sportInfoDMItem.startSecs[2]=(temp>>16)&0xff;
		g_sportInfoDMItem.startSecs[3]=(temp>>24)&0xff;
#if DEBUG_UART_EN    
				DbgPrintf("save action time=%d:\r\n",temp);
#endif 

  }
}
#endif


