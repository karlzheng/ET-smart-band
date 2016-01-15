#ifndef _ARITHMETIC_DATA_MANGER_H_
#define _ARITHMETIC_DATA_MANGER_H_
#include "ET1502B.h"
#if WALK_STEP_RECORD_ARITHMETIC

#define ARTH_SPORT_SAVE2DATAMANGER_INTERVAL		600//(10*60) // 10 min



typedef enum
{
	SPORT_MODE_WALK = 0,
	SPORT_MODE_RUN,
	SPORT_MODE_HALF_WALK_RUN,
	SPORT_MODE_MAX
}SPORT_MODE_TYPE;

typedef enum
{
	SPORT_TYPE_WALKING = 0,
	SPORT_TYPE_RUNNING,
	SPORT_TYPE_UP_STARING,
	SPORT_TYPE_CLIMBING,
	SPORT_TYPE_BASKETBALL,
	SPORT_TYPE_GOLF,
	SPORT_TYPE_MAX
}sport_type_enum;


typedef struct 
{
	unsigned int distance;   // unit : cm
	unsigned int calorie;	 // 
	signed int steps;
	unsigned short sustainSecs;
	unsigned short speed;
	unsigned char sportMode;
}walk_info_t;

typedef struct
{
	unsigned char flag;
	unsigned char id_num[4];
	unsigned char sex;		//0:man , 1:woman
	unsigned char age;	
	unsigned char hight;
	unsigned char weight;
	unsigned char stepMax0;
	unsigned char stepMax1;
	unsigned char stepMax2;
	unsigned char stepMax3;
	unsigned char left_right_hand;//0:left, 1:rifht

}user_info_struct;

typedef struct
{
	// sport start time
	unsigned char startSecs[4];
	unsigned char walkSteps[3];
	unsigned char distance[3];
	unsigned char calories[4];
	unsigned char sustainSecs[2];//walkSteps 的运动间隔
}sport_info_struct;

typedef struct
{
	unsigned char weight; // kg
	unsigned char stepDisInterval;
	unsigned char left_rifht_hand;
}arth_personal_info;

extern user_info_struct g_user;
extern sport_info_struct g_sportInfoDMItem;
extern sport_info_struct g_sportInfoDMItemSave;

extern walk_info_t g_walkInfoStruct;
//extern unsigned char SportFlg;

extern unsigned int app_arthmetic_get_total_steps(void);
extern unsigned int app_arthmetic_get_total_calorie(void);
extern unsigned int app_arthmetic_get_total_distance(void);
extern unsigned char app_arithmetic_get_left_right_hand(void);

extern signed short app_arithmetic_store_steps(unsigned char steps, unsigned char sportMode, unsigned char flushFlag,unsigned char pedometerFlg);

extern void app_arthmetic_clear_steps(unsigned char flag);
extern void receive_userInfo_from_app(unsigned char *data);
extern void Get_userInfo_from_app(unsigned char *data);
extern void save_sport_info2_datamanger(void);
#if FREE_NOTIC_ARITHMETIC
extern void receive_freeInfo_from_app(unsigned char *data);
#endif
#endif


#endif
