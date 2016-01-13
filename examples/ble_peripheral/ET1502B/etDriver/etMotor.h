#ifndef _ETMOTOR_H_
#define _ETMOTOR_H_
#include <stdint.h>

#define MOTOR_PWM_HI_DELAY 3////4
#define MOTOR_PWM_LO_DELAY 1

#define MOTOR_ENABLE 1
#define MOTOR_DISABLE 0

#define MOTOR_TYPE_ALARM 0
#define MOTOR_TYPE_INCOMING 1
#define MOTOR_TYPE_TEST 2

#define MOTOR_TYPE_ALARM_DELAY 2//400
#define MOTOR_TYPE_NoShake_ALARM_DELAY 2//400

#define MOTOR_TYPE_INCOMING_DELAY 2//250
#define MOTOR_TYPE_NoShake_INCOMING_DELAY 2//250
 
#define MOTOR_TYPE_TEST_DELAY 2//250
#define MOTOR_TYPE_NoShake_TEST_DELAY 2//250
typedef struct
{
  unsigned Motor_enalbe:8;
//  unsigned Motor_HiLevelTime:8;
//  unsigned Motor_LoLevelTime:8;
  unsigned Motor_dispAlarm_flag:8;
  
	unsigned Motor_shake_type:8;
  unsigned Motor_shake_count:8;
  unsigned Motor_shake_delay:16;
  
  
  unsigned Motor_NoShake_delay:16;
//  unsigned Motor_no_use2:16;
}Motor_stru;
extern Motor_stru Motor_str;
extern void MotorInit(void);
extern void MotorVibrate(void);
extern void MotorCtr(uint8_t enable,unsigned char type);
#endif
