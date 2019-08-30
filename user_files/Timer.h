#ifndef TIMER_H
#define TIMER_H

#define TIMER_0    0x01
#define TIMER_1    0x02

#define TIME_MS_T0   5      //5ms  no more than 71
#define TIME_MS_T1   50    //50ms  no more than 71
#define T0_CNT   (65536-(TIME_MS_T0*(FOSC/12/1000)))  //every count cost 1000/(FOSC/12) ms,so,1ms need (FOSC/12)/1000 count
#define T1_CNT   (65536-(TIME_MS_T1*(FOSC/12/1000))) //take care of overflow,i mean count value >65536

extern void InitTimer();
extern void SetTimerCnt(INT8U Tnum,INT16U Vlaue);

#endif






































