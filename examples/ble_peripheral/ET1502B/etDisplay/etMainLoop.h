#ifndef _MAIN_LOOP_H_
#define _MAIN_LOOP_H_



//extern void LDO_PW_CTR(void);
extern void dev_init(void);
extern void dev_init_from_Timer4_to_WakeUp(void);
extern void dev_init_from_GPIOINT_to_WakeUp(void);

extern void dev_init_from_ButtonDown_to_WakeUp(void);
extern void main_loop(void);



#endif

