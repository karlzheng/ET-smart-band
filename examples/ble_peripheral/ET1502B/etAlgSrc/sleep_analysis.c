#include "sleep_analysis.h"
#if SLEEP_RECORD_ARITHMETIC
#include <string.h>
#include "common_arithmetic.h"
#include "et_debug.h"
#include "sleep_data.h"

#if TEST_SLEEP_ENABLE
#define SLEEP_END_TIME				(3)
#else
#define SLEEP_END_TIME				(2400)
#endif

#define ERROR_XAXISSAMEAVGDATA		(250)//(4)
#define SB_MAX_STEP_NUM				(10) // sleep begin case: step num
#define SE_MAX_STEP_NUM 			(15) // sleep end case: step num >

#define SAME_UNIT_MAX		(200)
#define SHORTEST_TIME		(4)
the_same_unit unit[SAME_UNIT_MAX];

enum
{
	SLEEP_BEGIN_FLG = 0,
	SLEEP_END_FLG,
	SLEEP_MAX
}sleep_analysis_enum;


unsigned short g_sleep_rocord_size = 0;
sleepRawDataStruct gSleepRawItem[SLEEP_ROCORD_SIZE] = {0};

unsigned char gNapArrayCount = 0;
CENapDataStruct	gNapArray[SLEEP_NAP_SIZE];

unsigned char g_sleeep_state_index= 0;
CESleepStateStruct g_pSleepStateItem[SLEEP_STATUS_SIZE];

unsigned char g_flash_sleeep_state_index= 0;
CESleepStateStruct g_flash_pSleepStateItem[SLEEP_STATUS_SIZE];

CESleepNapAnalysisCfgStruct g_SleepNapAnalysisCfg ={100,30};
gsensor_cfg_struct g_gsensro_cfg;

sleep_record_struct g_sleep_record;

unsigned char sleep_begin_flag = 0;

void judgeSleepTimeStamp(unsigned char hour)
{
	unsigned char gsensorEnum;
    if (hour <= 6)
        gsensorEnum =  GSENSOR_CFG_NIGHT;
    else if (hour >= 20)
        gsensorEnum =  GSENSOR_CFG_EVENING;
    else if (hour > 6 && hour <= 9)
        gsensorEnum =  GSENSOR_CFG_MORING;
    else
        gsensorEnum =  GSENSOR_CFG_DAY;
#if TEST_SLEEP_ENABLE	
	if (gsensorEnum == GSENSOR_CFG_DAY) {
		g_SleepNapAnalysisCfg.maxStepsBetweenNap = 10;
		g_SleepNapAnalysisCfg.maxMinDurationBetweenNap = 3;
	}
	else if (gsensorEnum == GSENSOR_CFG_MORING) {
		g_SleepNapAnalysisCfg.maxStepsBetweenNap = 10;
		g_SleepNapAnalysisCfg.maxMinDurationBetweenNap = 3;
	}
	else if (gsensorEnum == GSENSOR_CFG_EVENING) {
		g_SleepNapAnalysisCfg.maxStepsBetweenNap = 10;
		g_SleepNapAnalysisCfg.maxMinDurationBetweenNap = 3;
	}
	else if (gsensorEnum == GSENSOR_CFG_NIGHT) {
		g_SleepNapAnalysisCfg.maxStepsBetweenNap = 10;
		g_SleepNapAnalysisCfg.maxMinDurationBetweenNap = 1;
	} 
#else
	if (gsensorEnum == GSENSOR_CFG_DAY) {
        g_SleepNapAnalysisCfg.maxStepsBetweenNap = 100;
        g_SleepNapAnalysisCfg.maxMinDurationBetweenNap = 30;
    }
    else if (gsensorEnum == GSENSOR_CFG_MORING) {
        g_SleepNapAnalysisCfg.maxStepsBetweenNap = 100;
        g_SleepNapAnalysisCfg.maxMinDurationBetweenNap = 30;
    }
    else if (gsensorEnum == GSENSOR_CFG_EVENING) {
        g_SleepNapAnalysisCfg.maxStepsBetweenNap = 100;
        g_SleepNapAnalysisCfg.maxMinDurationBetweenNap = 30;
    }
    else if (gsensorEnum == GSENSOR_CFG_NIGHT) {
        g_SleepNapAnalysisCfg.maxStepsBetweenNap = 100;
        g_SleepNapAnalysisCfg.maxMinDurationBetweenNap = 10;
    }
#endif
}

unsigned char get_hour_from_sec(unsigned int sec)
{
	unsigned char hour = 0;
	hour = (sec % 86400)/(3600);
	return hour;
}

static unsigned char get_sleep_start_info_index(unsigned int curSec,unsigned char *index)
{
	unsigned char i = 0;
	for(i=0;i<g_sleeep_state_index;i++)
	{	
		if(curSec == g_pSleepStateItem[i].startSec)
		{
			*index = i; 
			return 1;
		}
	}
	return 0;
}

static void get_sleep_end_info_index(unsigned int curSec,unsigned char *index)
{
	unsigned char i = 0;
	for(i=0;i<g_sleeep_state_index;i++)
	{
		if(i == 0)
		{
			if(curSec == g_pSleepStateItem[i].startSec)
			{
				*index = 0; 
				break;
			}
		}
		else
		{
			if(curSec > g_pSleepStateItem[i-1].startSec && curSec <= g_pSleepStateItem[i].startSec)
			{
				*index = i; 
				break;
			}
		}
	}
}


void CESleepGSensorCfg(unsigned char gsensorEnum)
{
	static unsigned char preEnum = GSENSOR_CFG_MAX;
	if(gsensorEnum == preEnum)
		return;
	#if TEST_SLEEP_ENABLE == 0
	if (gsensorEnum == GSENSOR_CFG_DAY) 
	{
        g_gsensro_cfg.slopeThreshold 	= 2;
        g_gsensro_cfg.slopeDuration 	= 0;
        g_gsensro_cfg.rcdMinInterval 	= 5;
        g_gsensro_cfg.slopeDebounceSec 	= 1;
		g_gsensro_cfg.stepArithmeticYMin= 1000;

        g_gsensro_cfg.middle_sb_nomotion_ratio 	= DAY_MIDDLE_SB_NOMOTION_RATIO;
        g_gsensro_cfg.middle_sb_max_slope_num 	= DAY_MIDDLE_SB_MAX_SLOPE_NUM;
        g_gsensro_cfg.middle_se_nomotion_ratio 	= DAY_MIDDLE_SE_NOMOTION_RATIO;
        g_gsensro_cfg.middle_se_max_slope_num 	= DAY_MIDDLE_SE_MAX_SLOPE_NUM;
        g_gsensro_cfg.middle_ss_nomotion_ratio 	= 80;//0.8;
        g_gsensro_cfg.middle_ss_max_slope_num 	= 4;
        g_gsensro_cfg.middle_sd_nomotion_ratio 	= 85;//0.85;
        g_gsensro_cfg.middle_sd_max_slope_num 	= 1;
        g_gsensro_cfg.begin_samp_num	= 12;
		g_gsensro_cfg.end_samp_num 		= 6;
     }
     else if (gsensorEnum == GSENSOR_CFG_MORING) 
	 {
        g_gsensro_cfg.slopeThreshold 	= 4;//6;//7;
        g_gsensro_cfg.slopeDuration 	= 0;
        g_gsensro_cfg.rcdMinInterval 	= 5;
        g_gsensro_cfg.slopeDebounceSec 	= 2;
		g_gsensro_cfg.stepArithmeticYMin= 1000;

        g_gsensro_cfg.middle_sb_nomotion_ratio 	= MORING_MIDDLE_SB_NOMOTION_RATIO;
        g_gsensro_cfg.middle_sb_max_slope_num 	= MORING_MIDDLE_SB_MAX_SLOPE_NUM;
        g_gsensro_cfg.middle_se_nomotion_ratio 	= MORING_MIDDLE_SE_NOMOTION_RATIO;
        g_gsensro_cfg.middle_se_max_slope_num 	= MORING_MIDDLE_SE_MAX_SLOPE_NUM;
        g_gsensro_cfg.middle_ss_nomotion_ratio 	= 80;//0.8;
        g_gsensro_cfg.middle_ss_max_slope_num 	= 4;
        g_gsensro_cfg.middle_sd_nomotion_ratio 	= 85;//0.85;
        g_gsensro_cfg.middle_sd_max_slope_num 	= 1;
        g_gsensro_cfg.begin_samp_num 	= 6;
		g_gsensro_cfg.end_samp_num 		= 6;
     }
     else if (gsensorEnum == GSENSOR_CFG_EVENING) 
	 {
        g_gsensro_cfg.slopeThreshold 	= 4;//9;//10;
        g_gsensro_cfg.slopeDuration 	= 0;
        g_gsensro_cfg.rcdMinInterval 	= 5;
        g_gsensro_cfg.slopeDebounceSec 	= 2;
		g_gsensro_cfg.stepArithmeticYMin= 1000;

        g_gsensro_cfg.middle_sb_nomotion_ratio 	= EVENING_MIDDLE_SB_NOMOTION_RATIO;
        g_gsensro_cfg.middle_sb_max_slope_num 	= EVENING_MIDDLE_SB_MAX_SLOPE_NUM;
        g_gsensro_cfg.middle_se_nomotion_ratio 	= EVENING_MIDDLE_SE_NOMOTION_RATIO;
        g_gsensro_cfg.middle_se_max_slope_num 	= EVENING_MIDDLE_SE_MAX_SLOPE_NUM;
        g_gsensro_cfg.middle_ss_nomotion_ratio 	= 80;//0.8;
        g_gsensro_cfg.middle_ss_max_slope_num 	= 3;
        g_gsensro_cfg.middle_sd_nomotion_ratio 	= 85;//0.85;
        g_gsensro_cfg.middle_sd_max_slope_num 	= 1;
        g_gsensro_cfg.begin_samp_num 	= 12;
		g_gsensro_cfg.end_samp_num 		= 6;
     }
     else if (gsensorEnum == GSENSOR_CFG_NIGHT) 
	 {
        g_gsensro_cfg.slopeThreshold 	= 6;//7;//15;//20;
        g_gsensro_cfg.slopeDuration 	= 0;
        g_gsensro_cfg.rcdMinInterval 	= 5;
        g_gsensro_cfg.slopeDebounceSec 	= 3;
		g_gsensro_cfg.stepArithmeticYMin= 1000;

        g_gsensro_cfg.middle_sb_nomotion_ratio 	= NIGHT_MIDDLE_SB_NOMOTION_RATIO;
        g_gsensro_cfg.middle_sb_max_slope_num 	= NIGHT_MIDDLE_SB_MAX_SLOPE_NUM;
        g_gsensro_cfg.middle_se_nomotion_ratio 	= NIGHT_MIDDLE_SE_NOMOTION_RATIO;
        g_gsensro_cfg.middle_se_max_slope_num 	= NIGHT_MIDDLE_SE_MAX_SLOPE_NUM;
        g_gsensro_cfg.middle_ss_nomotion_ratio 	= 80;//0.8;
        g_gsensro_cfg.middle_ss_max_slope_num 	= 4;
        g_gsensro_cfg.middle_sd_nomotion_ratio 	= 90;//80;//0.85;
        g_gsensro_cfg.middle_sd_max_slope_num 	= 1;
        g_gsensro_cfg.begin_samp_num 	= 12;
		g_gsensro_cfg.end_samp_num 		= 6;
      }
	#else
	
	if (gsensorEnum == GSENSOR_CFG_DAY) 
	{
        g_gsensro_cfg.slopeThreshold 	= 3;//2;
        g_gsensro_cfg.slopeDuration 	= 0;
        g_gsensro_cfg.rcdMinInterval 	= 1;//5;
        g_gsensro_cfg.slopeDebounceSec 	= 1;
		g_gsensro_cfg.stepArithmeticYMin= 1000;

        g_gsensro_cfg.middle_sb_nomotion_ratio = DAY_MIDDLE_SB_NOMOTION_RATIO;
        g_gsensro_cfg.middle_sb_max_slope_num = DAY_MIDDLE_SB_MAX_SLOPE_NUM;
        g_gsensro_cfg.middle_se_nomotion_ratio = DAY_MIDDLE_SE_NOMOTION_RATIO;
        g_gsensro_cfg.middle_se_max_slope_num = DAY_MIDDLE_SE_MAX_SLOPE_NUM;
        g_gsensro_cfg.middle_ss_nomotion_ratio = 80;//0.8;
        g_gsensro_cfg.middle_ss_max_slope_num = 4;
        g_gsensro_cfg.middle_sd_nomotion_ratio = 85;//0.85;
        g_gsensro_cfg.middle_sd_max_slope_num = 1;
        g_gsensro_cfg.begin_samp_num= 2;//12;
		g_gsensro_cfg.end_samp_num = 2;//6;
     }
     else if (gsensorEnum == GSENSOR_CFG_MORING) 
	 {
        g_gsensro_cfg.slopeThreshold 	= 10;//15;
        g_gsensro_cfg.slopeDuration 	= 0;
        g_gsensro_cfg.rcdMinInterval 	= 1;
        g_gsensro_cfg.slopeDebounceSec 	= 2;
		g_gsensro_cfg.stepArithmeticYMin= 1000;

        g_gsensro_cfg.middle_sb_nomotion_ratio = MORING_MIDDLE_SB_NOMOTION_RATIO;
        g_gsensro_cfg.middle_sb_max_slope_num = MORING_MIDDLE_SB_MAX_SLOPE_NUM;
        g_gsensro_cfg.middle_se_nomotion_ratio = MORING_MIDDLE_SE_NOMOTION_RATIO;
        g_gsensro_cfg.middle_se_max_slope_num = MORING_MIDDLE_SE_MAX_SLOPE_NUM;
        g_gsensro_cfg.middle_ss_nomotion_ratio = 80;//0.8;
        g_gsensro_cfg.middle_ss_max_slope_num = 4;
        g_gsensro_cfg.middle_sd_nomotion_ratio = 85;//0.85;
        g_gsensro_cfg.middle_sd_max_slope_num = 1;
        g_gsensro_cfg.begin_samp_num = 2;
		g_gsensro_cfg.end_samp_num = 2;
     }
     else if (gsensorEnum == GSENSOR_CFG_EVENING) 
	 {
        g_gsensro_cfg.slopeThreshold 	= 20;
        g_gsensro_cfg.slopeDuration 	= 0;
        g_gsensro_cfg.rcdMinInterval 	= 1;
        g_gsensro_cfg.slopeDebounceSec 	= 2;
		g_gsensro_cfg.stepArithmeticYMin= 1000;

        g_gsensro_cfg.middle_sb_nomotion_ratio = EVENING_MIDDLE_SB_NOMOTION_RATIO;
        g_gsensro_cfg.middle_sb_max_slope_num = EVENING_MIDDLE_SB_MAX_SLOPE_NUM;
        g_gsensro_cfg.middle_se_nomotion_ratio = EVENING_MIDDLE_SE_NOMOTION_RATIO;
        g_gsensro_cfg.middle_se_max_slope_num = EVENING_MIDDLE_SE_MAX_SLOPE_NUM;
        g_gsensro_cfg.middle_ss_nomotion_ratio = 80;//0.8;
        g_gsensro_cfg.middle_ss_max_slope_num = 3;
        g_gsensro_cfg.middle_sd_nomotion_ratio = 85;//0.85;
        g_gsensro_cfg.middle_sd_max_slope_num = 1;
        g_gsensro_cfg.begin_samp_num = 2;
		g_gsensro_cfg.end_samp_num = 2;
     }
     else if (gsensorEnum == GSENSOR_CFG_NIGHT) 
	 {
        g_gsensro_cfg.slopeThreshold 	= 35;
        g_gsensro_cfg.slopeDuration 	= 0;
        g_gsensro_cfg.rcdMinInterval 	= 1;
        g_gsensro_cfg.slopeDebounceSec 	= 3;
		g_gsensro_cfg.stepArithmeticYMin= 1000;

        g_gsensro_cfg.middle_sb_nomotion_ratio = NIGHT_MIDDLE_SB_NOMOTION_RATIO;
        g_gsensro_cfg.middle_sb_max_slope_num = NIGHT_MIDDLE_SB_MAX_SLOPE_NUM;
        g_gsensro_cfg.middle_se_nomotion_ratio = NIGHT_MIDDLE_SE_NOMOTION_RATIO;
        g_gsensro_cfg.middle_se_max_slope_num = NIGHT_MIDDLE_SE_MAX_SLOPE_NUM;
        g_gsensro_cfg.middle_ss_nomotion_ratio = 80;//0.8;
        g_gsensro_cfg.middle_ss_max_slope_num = 4;
        g_gsensro_cfg.middle_sd_nomotion_ratio = 80;//0.85;
        g_gsensro_cfg.middle_sd_max_slope_num = 1;
        g_gsensro_cfg.begin_samp_num = 2;
		g_gsensro_cfg.end_samp_num = 2;
      }
	#endif
	 preEnum = gsensorEnum;
}


void sleep_gsensor_cfg_judge(unsigned char hour)
{
   unsigned char paramIndex = GSENSOR_CFG_MAX;
   
   if (hour <= 6)
	   paramIndex = GSENSOR_CFG_NIGHT;
   else if (hour >= 20)
	   paramIndex = GSENSOR_CFG_EVENING;
   else if (hour > 6 && hour <= 9)
	   paramIndex = GSENSOR_CFG_MORING;
   else
	   paramIndex = GSENSOR_CFG_DAY;

   CESleepGSensorCfg(paramIndex);
}

bool containsCharge(CENapDataStruct *src)
{
	unsigned short i=0;
	int curTime  = 0;
	for(i=0;i<g_sleep_rocord_size;i++)
	{
		curTime = FourCharGetInt((char*)gSleepRawItem[i].startSecs);
		if(curTime >= src->startSec && curTime < src->endSec)
		{
            if(gSleepRawItem[i].devStatus.bits.chargerFlag)
                return true;
        }
	}
    return false;
}

unsigned int getStepsFromRawDataByDate(unsigned int begin,unsigned int end)
{
	unsigned int sum = 0;
	unsigned short i=0;
	int curTime  = 0;

	if(begin == end || begin < end)
		return 0;

	for(i=0;i<g_sleep_rocord_size;i++)
	{
		curTime = FourCharGetInt((char*)gSleepRawItem[i].startSecs);
		if(curTime >= begin && curTime <= end)
			sum += gSleepRawItem[i].stepsTotalNum;
	}
	return sum;
}

unsigned short getStaticFromList(sleepRawDataStruct sleep_list[],unsigned short list_size)
{
	unsigned short i =0,begin = 0, end = 0, cash =1000,value =0,xAxisAvg = 0;
	unsigned short same_unit_index = 0;

	memset((unsigned char*)(unit),0,SAME_UNIT_MAX * sizeof(the_same_unit));
	
	for(i=0;i<list_size;i++)
	{
		if(!(sleep_list[i].xAxisSameTotalNum == 0 && sleep_list[i].xAxisSameOnceMaxNum == 0))
		{
			xAxisAvg = TwoCharGetUShort(sleep_list[i].xAxisSameAvgData);
			if(cash == 1000)
			{
				begin = i;
				cash = xAxisAvg;
			}
			else
			{
				value = cash > xAxisAvg ? cash - xAxisAvg : xAxisAvg - cash; 
				QPRINTF("gsensor diff value=%d\r\n",value);
				if(value < ERROR_XAXISSAMEAVGDATA)
				{
					end = i;
              		if (end == list_size - 1) 
					{
                		unit[same_unit_index].begin = begin;
                		unit[same_unit_index].end 	= end;
               		 	unit[same_unit_index].value = cash;
						same_unit_index++;
              		}
				}
				else
				{
					if (end > begin) 
					{
                		unit[same_unit_index].begin = begin;
                		unit[same_unit_index].end 	= end;
               		 	unit[same_unit_index].value = cash;
						same_unit_index++;
              		}

              		begin = i;
              		cash = xAxisAvg;
				}
			}
		}
		else
		{
			if (end > begin) 
			{
	            unit[same_unit_index].begin = begin;
	            unit[same_unit_index].end = end;
	            unit[same_unit_index].value = cash;
				same_unit_index++;
          	}
          	begin = i;
          	end = i;
		}
	}
	return same_unit_index;
}

void findAllStatic(sleepRawDataStruct sleep_list[],unsigned short list_size)
{
	unsigned short i = 0,j = 0,k = 0,same_unit_count=0;

	same_unit_count = getStaticFromList(sleep_list,list_size);
	if (same_unit_count > 1) 
	{
		for(i=0;i<same_unit_count;i++)	
		{
			if (unit[i].end - unit[i].begin < SHORTEST_TIME) 
				continue;
			
			if (j == 0) 
			{
				unit[j].begin 	= unit[i].begin;
	            unit[j].end 	= unit[i].end;
	            unit[j].value 	= unit[i].value;
				j++;
				continue;
			}
			
			if (unit[j].end == unit[i].begin - 1) //kai.ter do not ????
			{
				unit[j].end = unit[i].end;
			} 
			else 
			{
				unit[j].begin 	= unit[i].begin;
	            unit[j].end 	= unit[i].end;
	            unit[j].value 	= unit[i].value;
				j++;
			}
        }
      }
	else if (same_unit_count == 1) 
	{
	 	if(unit[0].end - unit[0].begin + 1 > SHORTEST_TIME)
	 	{
	 		unit[0].begin 	= unit[0].begin;
            unit[0].end 	= unit[0].end;
            unit[0].value 	= unit[0].value;
			j++;
	 	}
    }
	else
		return;

	// 去掉小于12个点的 即少于1小时的静止
	for (i=0;i<j;i++)
	{
		if (unit[i].end - unit[i].begin > 16) 
		{
	  		unit[k].begin 	= unit[i].begin;
	        unit[k].end 	= unit[i].end;
	        unit[k].value 	= unit[i].value;
			k++;
		}
	}

	for (i=0;i<k;i++)
	{
		for(j = unit[i].begin; j<=unit[i].end; j++)
			sleep_list[j].devStatus.bits.chargerFlag = 1;
	}
}


bool isSleepBegin(sleepRawDataStruct sleep_list[],unsigned short list_size,unsigned char flg)
{
	unsigned short i=0;
	int avg_noMotionTotalNum = 0;
    unsigned short avg_slopeMaxNum = 0;
    unsigned short avg_stepNum = 0;
	unsigned int preTime = 0,curTime=0,timeLongDif=0;
	unsigned short slopeMaxNum = 0;
	unsigned short noMotionMaxNum = 0;
	
	curTime = FourCharGetInt((char*)sleep_list[0].startSecs);
	
	for(i=0;i<list_size;i++)
	{
		if(preTime == 0)
		{
			preTime = curTime;
		}
		else
		{
			curTime = FourCharGetInt((char*)sleep_list[i].startSecs);
			timeLongDif = curTime > preTime ? curTime - preTime : preTime - curTime;
		  	if(timeLongDif > 30*60)
		  	{
		  		if(flg == SLEEP_BEGIN_FLG)
		     		return false;//表示中间有两条数据之间的时间间隔要大于30分钟，也表示两条数据间有超过5条数据的丢包
		     	else if(flg == SLEEP_END_FLG)
					return true;
			}
			preTime = curTime;
		}
		
		if(sleep_list[i].devStatus.bits.chargerFlag)
		{
			QPRINTF("charge.......\r\n");
			if(flg == SLEEP_BEGIN_FLG)
				return false;// 若有充电一律不算睡眠
			else if(flg == SLEEP_END_FLG)
				return true;
		}
			
        avg_noMotionTotalNum 	+= sleep_list[i].noMotionTotalNum;
        avg_slopeMaxNum 		+= sleep_list[i].slopeOnceMaxNum;
        avg_stepNum 			+= sleep_list[i].stepsTotalNum;
    }

	avg_noMotionTotalNum 	/= list_size;
	avg_slopeMaxNum 		/= list_size;
	avg_stepNum 			/= list_size;
	QPRINTF("avg_noMotionTotalNum=%d,avg_slopeMaxNum=%d,avg_stepNum=%d,\r\n",avg_noMotionTotalNum,avg_slopeMaxNum,avg_stepNum);
	
	
	if(flg == SLEEP_BEGIN_FLG)
	{
		noMotionMaxNum = (g_gsensro_cfg.rcdMinInterval * 4) * g_gsensro_cfg.middle_sb_nomotion_ratio/100;
		slopeMaxNum = g_gsensro_cfg.middle_sb_max_slope_num * g_gsensro_cfg.rcdMinInterval;
		QPRINTF("SLEEP_BEGIN_FLG:slopeMaxNum=%d,noMotionMaxNum=%d,\r\n",slopeMaxNum,noMotionMaxNum);
		if (avg_noMotionTotalNum > noMotionMaxNum && 
			avg_slopeMaxNum < slopeMaxNum && 
			avg_stepNum < SB_MAX_STEP_NUM ) 
	      return true;
		else 
	      return false;
	}
	else if(flg == SLEEP_END_FLG)
	{
		noMotionMaxNum = (g_gsensro_cfg.rcdMinInterval * 4) * g_gsensro_cfg.middle_se_nomotion_ratio/100;
		slopeMaxNum = g_gsensro_cfg.middle_se_max_slope_num * g_gsensro_cfg.rcdMinInterval;
		QPRINTF("SLEEP_END_FLG:slopeMaxNum=%d,noMotionMaxNum=%d,\r\n",slopeMaxNum,noMotionMaxNum);
		if ((avg_noMotionTotalNum < noMotionMaxNum ||
            avg_slopeMaxNum > slopeMaxNum )||
            avg_stepNum  >= SE_MAX_STEP_NUM) 
          return true;
        else 
          return false;
	}
	else
		return true;
}

unsigned short sleep_Add_2_State(CESleepStateStruct *pSleepStateItem, unsigned short stateEnum,sleepRawDataStruct *pCurRawDataItem,unsigned int startTime)
{
	pSleepStateItem->sleepState |= stateEnum;

	if (pCurRawDataItem->devStatus.bits.chargerFlag) // charging flag is // setting
	  pSleepStateItem->sleepState |= SLEEP_STATE_INVALID_CHARGING;

	pSleepStateItem->startSec = startTime;

	return pSleepStateItem->sleepState;
}


void sleepOneNapAnalysis(CENapDataStruct *pCurNap)
{
	unsigned int startTime = 0;
	int xDataSameCount = 0;
	unsigned short preState = 0;
	unsigned char listSize = 0,i=0;
    unsigned short curPos = 0;
	sleepRawDataStruct *pCurRawDataItem;
	CESleepStateStruct *pSleepStateItem;
	
	pSleepStateItem = &g_pSleepStateItem[g_sleeep_state_index++];
	QPRINTF("beginRawDataPos=%d,endRawDataPos=%d,\r\n",pCurNap->beginRawDataPos,pCurNap->endRawDataPos);
	curPos = pCurNap->beginRawDataPos;
	while(curPos <= pCurNap->endRawDataPos)
	{
		pCurRawDataItem = &gSleepRawItem[curPos];
		startTime = FourCharGetInt((char*)pCurRawDataItem->startSecs);
		sleep_gsensor_cfg_judge(get_hour_from_sec(startTime));

		if (curPos == pCurNap->beginRawDataPos)
		{
			QPRINTF("sleep begin\r\n");
			preState = sleep_Add_2_State(pSleepStateItem, SLEEP_STATE_BEGIN,pCurRawDataItem,startTime);
		}
		else if (curPos == pCurNap->endRawDataPos)
		{
			QPRINTF("sleep end\r\n");
			pSleepStateItem = &g_pSleepStateItem[g_sleeep_state_index++];
          	sleep_Add_2_State(pSleepStateItem, SLEEP_STATE_END, pCurRawDataItem,startTime);
		}
		else
		{
			unsigned short slopeDeepMaxNum 		= 0;
          	unsigned short slopeShallowMaxNum 	= 0;
          	bool deepXDataSame2DiffFlag 		= false;
			unsigned short avg_NoMotion = 0,avg_SlopeTotal = 0,avg_StepNum = 0;

			if(++listSize < 3)
			{
	            curPos++;
	            continue;
          	}

			for(i=0;i<listSize;i++)
			{
	            avg_NoMotion 	+= gSleepRawItem[curPos-i].noMotionTotalNum;
	            avg_SlopeTotal 	+= gSleepRawItem[curPos-i].slopeTotalNum;
	            avg_StepNum 	+= gSleepRawItem[curPos-i].stepsTotalNum;
          	}
			QPRINTF("curPos=%d,\r\n",curPos);
			avg_NoMotion 	/= listSize;
			avg_SlopeTotal 	/= listSize;
			avg_StepNum		/= listSize;
			QPRINTF("*******avg_NoMotion=%d,avg_SlopeTotal=%d,avg_StepNum=%d\r\n",avg_NoMotion,avg_SlopeTotal,avg_StepNum);
			
			slopeDeepMaxNum 	= g_gsensro_cfg.rcdMinInterval * g_gsensro_cfg.middle_sd_max_slope_num/2;	//1,1,1,1 50%
          	slopeShallowMaxNum 	= g_gsensro_cfg.rcdMinInterval * g_gsensro_cfg.middle_ss_max_slope_num/2;	//4,4,3,4 200% 200% 150% 200%
			QPRINTF("slopeDeepMaxNum=%d,slopeShallowMaxNum=%d,\r\n",slopeDeepMaxNum,slopeShallowMaxNum);

			
			if ((avg_NoMotion > g_gsensro_cfg.rcdMinInterval * 4 * g_gsensro_cfg.middle_sd_nomotion_ratio/100 )
              &&(avg_SlopeTotal < slopeDeepMaxNum)&&(avg_StepNum < 2))
          	{ 
          		QPRINTF("deep sleep 111111111111111\r\n");
				// deep sleep

				if (((preState & SLEEP_STATE_DEEP) != 0) && // last
                (((pCurRawDataItem->devStatus.bits.chargerFlag) ? 1 : 0) == (((preState & SLEEP_STATE_INVALID_CHARGING) != 0) ? 1 : 0)) &&
                deepXDataSame2DiffFlag == false) // last state and cur
	            {
	            	//pSleepStateItem->stateKeepSec +=  g_gsensro_cfg.rcdMinInterval * 60;
	            }
            	else
	            {
					pSleepStateItem = &g_pSleepStateItem[g_sleeep_state_index++];
					preState = sleep_Add_2_State(pSleepStateItem,SLEEP_STATE_DEEP, pCurRawDataItem,startTime);
	            }
			}
			else if ((avg_NoMotion > g_gsensro_cfg.rcdMinInterval * 4 * g_gsensro_cfg.middle_ss_nomotion_ratio/100)
              &&(avg_SlopeTotal < slopeShallowMaxNum)&&(avg_StepNum < 10))
          	{ 
          		QPRINTF("shallow sleep 222222222222\r\n");
				// shallow sleep
				if ((preState & SLEEP_STATE_DEEP) != 0 && (xDataSameCount > 4))
				{
					QPRINTF("pSleepStateItem->sleepState |= SLEEP_STATE_INVALID_DROPOFF\r\n");
              		pSleepStateItem->sleepState |= SLEEP_STATE_INVALID_DROPOFF;
				}

				if ((preState & SLEEP_STATE_SHALLOW) != 0 && // last
                (((pCurRawDataItem->devStatus.bits.chargerFlag != 0) ? 1 : 0) == (((preState & SLEEP_STATE_INVALID_CHARGING) != 0) ? 1 : 0))) // last state and cur state, charging
                           // flag is the same
	            {
	            	QPRINTF("// flag is the same\r\n");
	            }
            	else
	            {
	              	if ((preState & SLEEP_STATE_DEEP) != 0 && (xDataSameCount > 2))
					{
						QPRINTF("(preState & SLEEP_STATE_DEEP) != 0 && (xDataSameCount > 2)\r\n");
						//deepXDataSameEnd = startTime - 1;
						pSleepStateItem->sleepState |= SLEEP_STATE_INVALID_DROPOFF;
						xDataSameCount = 0;
					}

					pSleepStateItem = &g_pSleepStateItem[g_sleeep_state_index++];
					preState = sleep_Add_2_State(pSleepStateItem,SLEEP_STATE_SHALLOW, pCurRawDataItem,startTime);
	            }
			}
			else 
          	{
          		QPRINTF("AWAKE SLEEP 3333333333333\r\n");
				// awake sleep
				if ((preState & SLEEP_STATE_DEEP) != 0 && (xDataSameCount > 2))
	            {
	            	QPRINTF("(preState & SLEEP_STATE_DEEP) != 0 && (xDataSameCount > 2)\r\n");
					//deepXDataSameEnd = startTime- 1;
					pSleepStateItem->sleepState |= SLEEP_STATE_INVALID_DROPOFF;
					xDataSameCount = 0;
	            }

				if ((preState & SLEEP_STATE_AWAKE) != 0 && // last
                (((pCurRawDataItem->devStatus.bits.chargerFlag != 0) ? 1 : 0) == (((preState & SLEEP_STATE_INVALID_CHARGING) != 0) ? 1: 0))) // last state and cur state, charging
                           // flag is the same
	            {
	            	QPRINTF("// flag is the same\r\n");
	            }
            	else
	            {
	            	QPRINTF("else\r\n");
					pSleepStateItem = &g_pSleepStateItem[g_sleeep_state_index++];
	              	preState = sleep_Add_2_State(pSleepStateItem,SLEEP_STATE_AWAKE, pCurRawDataItem,startTime);
	            }
			}
		}

		curPos++;
		listSize = 0;
	}
}

void getRealSleepData(void)
{
	unsigned char i=0,index = 0,max_index =0,left_index=0,right_index=0,last_end_flg = 0;;
	unsigned short minTime = 0,all_time=0;
	unsigned int stepBetweenNaps = 0;
	sleep_struct g_sleep_info ={0,0,0,0,0,0,0,0};
	
	max_index = 0;
	if(gNapArrayCount == 0)
		return;

	for(i=1;i<gNapArrayCount;i++)
	{
		if(gNapArray[i-1].sleepTotalMin < gNapArray[i].sleepTotalMin )
		{
			max_index = i;
		}
	}
	
    //最长的前一个和自己的间隔是不是小于30分钟,不是的话就把前一段个丢弃掉
	index = max_index;
	left_index = max_index;
	while(index >0 )
	{
		//找出左边和最长睡眠小于30分钟的睡眠
        minTime = (gNapArray[index].startSec - gNapArray[index-1].endSec)/60;
        judgeSleepTimeStamp(get_hour_from_sec(gNapArray[index-1].endSec));
        stepBetweenNaps = getStepsFromRawDataByDate(gNapArray[index-1].endSec,gNapArray[index].startSec);

        if(minTime > g_SleepNapAnalysisCfg.maxMinDurationBetweenNap ||
			stepBetweenNaps > g_SleepNapAnalysisCfg.maxStepsBetweenNap ||
			containsCharge(&gNapArray[index-1]))
		{
			//如果间隔时间大于30分钟且时间不是在凌晨，两个nap之间的步数超过，包含充电的元数据，则抛弃最长时间以前的睡眠数据
            break;  
        }
		left_index = index-1;
        index--;
	}

	index = max_index;
	right_index = max_index;
    while(index < gNapArrayCount-1)
	{
		//找出右边和最长睡眠小于30分钟的睡眠
        minTime = (gNapArray[index+1].startSec - gNapArray[index].endSec)/60;
        judgeSleepTimeStamp(get_hour_from_sec(gNapArray[index+1].startSec));
        stepBetweenNaps = getStepsFromRawDataByDate(gNapArray[index].endSec,gNapArray[index+1].startSec);
		
	    if(minTime > g_SleepNapAnalysisCfg.maxMinDurationBetweenNap ||
			stepBetweenNaps > g_SleepNapAnalysisCfg.maxStepsBetweenNap ||
			containsCharge(&gNapArray[index+1]) )
	   	{
	   		last_end_flg = 1;
          	break;  
        }
		right_index = index+1;
        index++;
    }

	g_sleep_info.sleep_start 	= gNapArray[left_index].startSec;
	g_sleep_info.sleep_end 		= gNapArray[right_index].endSec;
	for(i=left_index;i<=right_index;i++)
		all_time += gNapArray[i].sleepTotalMin;

#if TEST_SLEEP_ENABLE == 0	
	if(all_time < 180)
        return;
	else
#endif
	{
		unsigned short time =0;

		sleep_begin_flag = 1;
		if(get_sleep_start_info_index(gNapArray[left_index].startSec,&g_sleep_info.state_item_start))
		{
			get_sleep_end_info_index(gNapArray[right_index].endSec,&g_sleep_info.state_item_end);
			for(i=g_sleep_info.state_item_start; i<g_sleep_info.state_item_end; i++)
			{
				time = (g_pSleepStateItem[i+1].startSec - g_pSleepStateItem[i].startSec)/60;
			
				if((g_pSleepStateItem[i].sleepState & SLEEP_STATE_BEGIN) != 0 || 
					(g_pSleepStateItem[i].sleepState & SLEEP_STATE_END) != 0)
				{
					g_sleep_info.shall_time += time;
				}
				else if((g_pSleepStateItem[i].sleepState & SLEEP_STATE_AWAKE) != 0)
				{
					g_sleep_info.awake_time += time;
				}
				else if((g_pSleepStateItem[i].sleepState & SLEEP_STATE_SHALLOW) != 0)
				{
					g_sleep_info.shall_time += time;
				}
				else if((g_pSleepStateItem[i].sleepState & SLEEP_STATE_DEEP) != 0)
				{
					g_sleep_info.deep_time += time;
				}
			}
			g_sleep_info.sleep_start 		= g_pSleepStateItem[g_sleep_info.state_item_start].startSec;
			g_sleep_info.sleep_end 			= g_pSleepStateItem[g_sleep_info.state_item_end].startSec;
			g_sleep_info.sleep_real_time 	= (g_pSleepStateItem[g_sleep_info.state_item_end].startSec - g_pSleepStateItem[g_sleep_info.state_item_start].startSec)/60;

			g_sleep_record.sleep_begin_time = g_sleep_info.sleep_start;
			g_sleep_record.sleep_end_time 	= g_sleep_info.sleep_end;
			g_sleep_record.awake_sleep_time	= g_sleep_info.awake_time;
			g_sleep_record.shall_sleep_time = g_sleep_info.shall_time;
			g_sleep_record.deep_sleep_time  = g_sleep_info.deep_time;
			g_sleep_record.sleep_total_time	= g_sleep_info.sleep_real_time;
			g_sleep_record.state_item_start = g_sleep_info.state_item_start;
			g_sleep_record.state_item_end 	= g_sleep_info.state_item_end; 
			g_sleep_record.sleep_end_flg 	= last_end_flg; 

			
			g_flash_sleeep_state_index = 0;
			for(i=g_sleep_record.state_item_start;i<=g_sleep_record.state_item_end;i++)
			{
				g_flash_pSleepStateItem[g_flash_sleeep_state_index].sleepState 	= g_pSleepStateItem[i].sleepState;
				g_flash_pSleepStateItem[g_flash_sleeep_state_index].startSec	= g_pSleepStateItem[i].startSec;
				g_flash_sleeep_state_index++;
			}

			QPRINTF("g_awake_time=%d,g_shall_time=%d,g_deep_time=%d,sleep_real_time=%d,sleep_end_flg=%d,g_flash_sleeep_state_index=%d,\r\n",
				g_sleep_info.awake_time,
				g_sleep_info.shall_time,
				g_sleep_info.deep_time,
				g_sleep_info.sleep_real_time,
				last_end_flg,
				g_flash_sleeep_state_index);
		}
	}
}


void sleepDataAnalysis(void)
{
	unsigned short rawItemNum =0;
	unsigned int curTime =0;
	unsigned char hour = 0,begin_analysis_flg = 0;
	sleepRawDataStruct *pCurRawDataItem;
	CENapDataStruct pCurNap;

	gNapArrayCount = 0;
	memset((unsigned char*)(gNapArray),0,SLEEP_NAP_SIZE * sizeof(CENapDataStruct));

	g_sleeep_state_index = 0;
	memset((unsigned char*)(g_pSleepStateItem),0,SLEEP_STATUS_SIZE * sizeof(CESleepStateStruct));

	memset((unsigned char*)&pCurNap,0,sizeof(CENapDataStruct));
	findAllStatic(gSleepRawItem,g_sleep_rocord_size);

	QPRINTF("begin sleep analysis:g_sleep_rocord_size=%d,\r\n",g_sleep_rocord_size);
	for(rawItemNum= 0; rawItemNum < g_sleep_rocord_size; rawItemNum++)
	{
		pCurRawDataItem = &gSleepRawItem[rawItemNum];
		curTime 		= FourCharGetInt((char*)pCurRawDataItem->startSecs);
		hour			= get_hour_from_sec(curTime);
		#if TEST_SLEEP_ENABLE == 0
		if(hour >= 16 || begin_analysis_flg)//from 16 hour start analysis
		#endif
		{
			begin_analysis_flg = 1;

			sleep_gsensor_cfg_judge(hour);

			if (pCurNap.napBeginFlag != 1) // search for begin point
			{
				if (g_sleep_rocord_size - rawItemNum < g_gsensro_cfg.begin_samp_num)  
	            	break;

				QPRINTF("IN isBegin analysis  .rawItemNum=%d,g_gsensro_cfg.begin_samp_num=%d,\r\n",rawItemNum,g_gsensro_cfg.begin_samp_num);
				if(isSleepBegin(pCurRawDataItem,g_gsensro_cfg.begin_samp_num,SLEEP_BEGIN_FLG))
				{
					QPRINTF("sleep start.......\r\n");
					pCurNap.napBeginFlag 	= 1;
					pCurNap.beginRawDataPos = rawItemNum;
					pCurNap.startSec 		= curTime;
				}
	          	else
	            	continue;
			}
			else // search for end point
	        {
	        	QPRINTF("sleep is begin time=%d,rawItemNum=%d,g_gsensro_cfg.end_samp_num=%d,\r\n\r\n",pCurNap.startSec,rawItemNum,g_gsensro_cfg.end_samp_num);
				if (g_sleep_rocord_size - rawItemNum >= g_gsensro_cfg.end_samp_num)
				{
					//如果剩余的采样点大于6个点，则走正常流程，否则算他直接结束
					if (isSleepBegin(pCurRawDataItem,g_gsensro_cfg.end_samp_num,SLEEP_END_FLG))
		            {
		            	QPRINTF("sleep end.......\r\n");
		                rawItemNum = rawItemNum + g_gsensro_cfg.end_samp_num - 1;
		                pCurRawDataItem = &gSleepRawItem[rawItemNum];
		                
						// if get currect raw data is charing, change the last raw data to
						// end
						// state; don't get charging status to nap;
						pCurNap.endRawDataPos = rawItemNum;

						if (pCurRawDataItem->devStatus.bits.chargerFlag)
						{
							if (rawItemNum + 1 < g_sleep_rocord_size - 1) 
							{
								pCurRawDataItem = &gSleepRawItem[rawItemNum + 1];
								curTime 		= FourCharGetInt((char*)pCurRawDataItem->startSecs);
							}
							pCurNap.endRawDataPos = rawItemNum + 1;
						}
					  	pCurNap.napEndFlag		= 1;
						pCurNap.endSec			= curTime;
						pCurNap.sleepTotalMin 	= (pCurNap.endSec - pCurNap.startSec)/60;
		              	
		              	if(pCurNap.sleepTotalMin >= g_gsensro_cfg.rcdMinInterval * 3)
		              	{
		              		QPRINTF("sleep is END start:%d ,end:%d, total time=%d,\r\n",pCurNap.startSec,pCurNap.endSec,pCurNap.sleepTotalMin);
							sleepOneNapAnalysis(&pCurNap);
							memcpy(&gNapArray[gNapArrayCount++],&pCurNap,sizeof(CENapDataStruct));
		              	}
		                
		              	memset((unsigned char*)&pCurNap,0,sizeof(CENapDataStruct));
		            }
		            else 
	            		continue;
				}
				else
				{
					pCurNap.endRawDataPos = rawItemNum;
								
					if (pCurRawDataItem->devStatus.bits.chargerFlag)
					{
					  	if (rawItemNum + 1 < g_sleep_rocord_size - 1) 
						{
							pCurRawDataItem = &gSleepRawItem[rawItemNum + 1];
							curTime 		= FourCharGetInt((char*)pCurRawDataItem->startSecs);
						}
						pCurNap.endRawDataPos = rawItemNum + 1;
					}
					pCurNap.napEndFlag		= 1;
					pCurNap.endSec			= curTime;
					pCurNap.sleepTotalMin 	= (pCurNap.endSec - pCurNap.startSec)/60;
	              	if(pCurNap.sleepTotalMin >= g_gsensro_cfg.rcdMinInterval * 3) 
	                {
						QPRINTF("sleep is not end but size < 2 start:%d ,end:%d , total time=%d,\r\n",pCurNap.startSec,pCurNap.endSec,pCurNap.sleepTotalMin);
						sleepOneNapAnalysis(&pCurNap);
		                memcpy(&gNapArray[gNapArrayCount++],&pCurNap,sizeof(CENapDataStruct));
		            }
	                
	              	memset((unsigned char*)&pCurNap,0,sizeof(CENapDataStruct));
				}
			}
		}
		
	}

	getRealSleepData();
	QPRINTF("************************\r\n\r\n");
}

void write_sleep_rocrd_to_raw(unsigned char* pData)
{
	unsigned char *pDataPos = NULL;
	unsigned short i =0;
	unsigned char record_buf[16];

	if(g_sleep_rocord_size >= SLEEP_ROCORD_SIZE)
	{
		g_sleep_rocord_size = SLEEP_ROCORD_SIZE-1;
		pDataPos = (unsigned char *)&gSleepRawItem[0];
		for(i=0;i<(SLEEP_ROCORD_SIZE-1)*16;i++)
			*(pDataPos+i) = *(pDataPos+i+16);
	}	
	pDataPos = (unsigned char *)&gSleepRawItem[g_sleep_rocord_size++];
	
	memcpy(pDataPos,pData,16);

	sleepDataAnalysis();

	if(sleep_begin_flag)
	{
		if(g_sleep_record.sleep_end_time + SLEEP_END_TIME <= FourCharGetInt((char*)gSleepRawItem[g_sleep_rocord_size-1].startSecs) || //睡眠结束已经大于2个小时
			g_sleep_record.sleep_end_flg )
		{
			sleep_begin_flag = 0;
			g_sleep_record.sleep_end_flg =0;

			memcpy(record_buf,(unsigned char*)&g_sleep_record,16);
			
		#if SLEEP_RECORD_DATA_ENABLE
			write_sleep_record_to_flash(record_buf,16);
		#endif
			
			g_sleep_rocord_size = 0;
			memset((unsigned char*)(gSleepRawItem),0,SLEEP_ROCORD_SIZE * sizeof(sleepRawDataStruct));
		}
	}
}

unsigned int app_get_sleep_begin(void)
{
	return g_sleep_record.sleep_begin_time;
}

unsigned int app_get_sleep_end(void)
{
	return g_sleep_record.sleep_end_time;
}

unsigned int app_get_sleep_all_time(void)
{
	return g_sleep_record.sleep_total_time;
}

unsigned short app_get_sleep_awake_time(void)
{
	return g_sleep_record.awake_sleep_time;
}

unsigned short app_get_sleep_shall_time(void)
{
	return g_sleep_record.shall_sleep_time;
}

unsigned short app_get_sleep_deep_time(void)
{
	return g_sleep_record.deep_sleep_time;
}



#endif



