#ifndef _ETTIMER_H_
#define _ETTIMER_H_

//#include "TimerDrv.h"
//#include "etTypeDefine.h"

//#define TIMER_T0_IRQ_EN   TimerIrqEn(T0)
//#define TIMER_T1_IRQ_EN   TimerIrqEn(T1)
//#define TIMER_T2_IRQ_EN   TimerIrqEn(T2)
//#define TIMER_T3_IRQ_EN   TimerIrqEn(T3)
//#define TIMER_T4_IRQ_EN   TimerIrqEn(T4)

//#define TIMER_T0_IRQ_DIS  TimerIrqDis(T0)
//#define TIMER_T1_IRQ_DIS  TimerIrqDis(T1)
//#define TIMER_T2_IRQ_DIS  TimerIrqDis(T2)
//#define TIMER_T3_IRQ_DIS  TimerIrqDis(T3)
//#define TIMER_T4_IRQ_DIS  TimerIrqDis(T4)

//#define TIMER0_VALUE  0x2710 //(10000)=0x2710//0x00FFFFFF //(8000+1600)//0x9530//
#define TIMER1_VALUE  0x2710
#define TIMER1_1MS_LOAD_VALUE (8000+1600)

//extern unsigned char time_flag;
extern BOOL  g_Timer0flag;

extern void ETTimer0Init(void);
extern void ETTimer0Stop(void);
extern void ETTimer0Start(void);

extern void ET_Timer1Init(void);
extern void ET_Timer1Start(void);
//extern void ET_Timer1Stop(void);

extern unsigned int  ETGetTime0Tick(void);
extern void  ETSetTime0Tick(void);
//extern void ET_delay(uint16_t delay_count);
#endif



