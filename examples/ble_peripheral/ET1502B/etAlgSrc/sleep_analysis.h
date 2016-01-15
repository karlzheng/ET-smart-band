#ifndef _SLEEP_ANALYSIS_H_
#define _SLEEP_ANALYSIS_H_
#include "ET1502B.h"
#if SLEEP_RECORD_ARITHMETIC
#include "sleep_record.h"

#define TEST_SLEEP_ENABLE					(0)

#define SLEEP_ROCORD_SIZE	(12*24)
#define SLEEP_NAP_SIZE		(96)
#define SLEEP_STATUS_SIZE		SLEEP_NAP_SIZE

#define DAY_MIDDLE_SB_NOMOTION_RATIO 		(60)//(0.6)
#define DAY_MIDDLE_SB_MAX_SLOPE_NUM         (5)
#define DAY_MIDDLE_SE_NOMOTION_RATIO 		(50)//(0.5)
#define DAY_MIDDLE_SE_MAX_SLOPE_NUM 		(8)

#define MORING_MIDDLE_SB_NOMOTION_RATIO     (60)//(0.6)
#define MORING_MIDDLE_SB_MAX_SLOPE_NUM      (5)
#define MORING_MIDDLE_SE_NOMOTION_RATIO 	(50)//(0.5)
#define MORING_MIDDLE_SE_MAX_SLOPE_NUM 		(8)

#define EVENING_MIDDLE_SB_NOMOTION_RATIO	(80)//(90)//(0.8)
#define EVENING_MIDDLE_SB_MAX_SLOPE_NUM		(4)//(2)
#define EVENING_MIDDLE_SE_NOMOTION_RATIO 	(65)//(0.65)
#define EVENING_MIDDLE_SE_MAX_SLOPE_NUM 	(5)//(4)

#define NIGHT_MIDDLE_SB_NOMOTION_RATIO 		(80)//(90)//(0.8)
#define NIGHT_MIDDLE_SB_MAX_SLOPE_NUM 		(5)
#define NIGHT_MIDDLE_SE_NOMOTION_RATIO 		(50)//(0.5)
#define NIGHT_MIDDLE_SE_MAX_SLOPE_NUM 		(8)

#define SLEEP_STATE_BEGIN  					(1<<0)//0x01
#define SLEEP_STATE_AWAKE 					(1<<1)//0x02
#define SLEEP_STATE_SHALLOW 				(1<<2)//0x04
#define SLEEP_STATE_DEEP					(1<<3)//0x08
#define SLEEP_STATE_END						(1<<4)//0x10
#define SLEEP_STATE_INVALID_CHARGING		(1<<5)//0x20
#define SLEEP_STATE_INVALID_DROPOFF			(1<<6)//0x40
#define SLEEP_STATE_INVALID_MAX				(1<<7)//0x80

typedef struct
{ 
  unsigned char slopeThreshold;		// �����ļ���gsensor��ֵ
  unsigned char slopeDuration;		// ������ֵ���ظ�����
  unsigned char rcdMinInterval; 	// ���һ����¼
  unsigned char slopeDebounceSec;  	// �����жϵ�����ʱ��
  unsigned short stepArithmeticYMin;	//Y���ֵ

  unsigned char middle_sb_nomotion_ratio;	// ����˯�߿�ʼ��nomotion����
  unsigned char middle_sb_max_slope_num;	// ����˯�߿�ʼ��ÿ����motion����
  unsigned char middle_se_nomotion_ratio;	// ˯�߽�����nomotion����
  unsigned char middle_se_max_slope_num;	// ˯�߽�����ÿ����motion����
  unsigned char middle_ss_nomotion_ratio;	// ˯�߿�ʼ��nomotion����
  unsigned char middle_ss_max_slope_num;	// ˯�߿�ʼ��ÿ����motion����
  unsigned char middle_sd_nomotion_ratio;
  unsigned char middle_sd_max_slope_num;

  unsigned char begin_samp_num;
  unsigned char end_samp_num;
}gsensor_cfg_struct;

typedef struct
{
    unsigned char sleepState;
    unsigned int startSec;
}CESleepStateStruct;

typedef struct
{
	unsigned char sleepState;
	unsigned int startTime;
}sleep_data_struct;

typedef struct
{
	unsigned int sleep_start;
	unsigned int sleep_end;
	unsigned short awake_time;
	unsigned short shall_time;
	unsigned short deep_time;
	unsigned short sleep_real_time;
	unsigned char state_item_start;
	unsigned char state_item_end;
}sleep_struct;

typedef struct
{
	unsigned int sleep_begin_time;
	unsigned int sleep_end_time;
	unsigned short awake_sleep_time;
	unsigned short shall_sleep_time;
	unsigned short deep_sleep_time;
	unsigned short sleep_total_time;
	unsigned char state_item_start;
	unsigned char state_item_end;
	unsigned char sleep_end_flg;
}sleep_record_struct;

typedef struct
{
    unsigned char napBeginFlag;		//��ʼ��־
    unsigned char napEndFlag;		//������־
    unsigned int startSec;			//��ʼʱ���
    unsigned int endSec;			//����ʱ���
    unsigned short beginRawDataPos;	//��ʼλ��
    unsigned short endRawDataPos;	//����λ��
    unsigned short sleepTotalMin;	//˯����ʱ��
}CENapDataStruct;

typedef struct
{
	unsigned char maxStepsBetweenNap;
    unsigned char maxMinDurationBetweenNap;
}CESleepNapAnalysisCfgStruct;

typedef struct{
	unsigned short begin;
	unsigned short end;
	unsigned short value;
}the_same_unit;

void sleep_gsensor_cfg_judge(unsigned char hour);
void write_sleep_rocrd_to_raw(unsigned char* pData);

unsigned int app_get_sleep_begin(void);
unsigned int app_get_sleep_end(void);
unsigned int app_get_sleep_all_time(void);
unsigned short app_get_sleep_awake_time(void);
unsigned short app_get_sleep_shall_time(void);
unsigned short app_get_sleep_deep_time(void);


#endif



#endif



