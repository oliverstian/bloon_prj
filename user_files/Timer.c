#include"Stc12c5a.h"
#include"Global.h"
#include"Timer.h"

void SetTimerCnt(INT8U Tnum,INT16U Vlaue)
{
	if(Tnum == TIMER_0)
	{
		TL0 = Vlaue;
		TH0 = Vlaue >> 8;
	}
	else if(Tnum == TIMER_1)
	{
		TL1 = Vlaue;
		TH1 = Vlaue >> 8;
	}
}

void TimerConfig(INT8U Tnum)
{
	if((Tnum&TIMER_0) == TIMER_0)
	{
		TMOD &= 0xf1;  // timer0 mode 1,16bit timer
		TMOD |= 0x01;
		AUXR &= 0x7f;  //sysclk/12
		SetTimerCnt(TIMER_0,T0_CNT);
	}
	if((Tnum&TIMER_1) == TIMER_1)
	{
		TMOD &= 0x1f;  // timer1 mode 1,16bit timer
		TMOD |= 0x10;  
	    AUXR &= 0xbf;  // sysclk/12
	    SetTimerCnt(TIMER_1,T1_CNT);
	}
}

void TimerStart(INT8U Tnum,INT8U contr)
{
	if((Tnum&TIMER_0) == TIMER_0)
	{
		if(contr == Enable)
		{
			TCON |= 0x10;
		}
		else if(contr == Disable)
		{
			TCON &= 0xef;
		}
	}
	
	if((Tnum&TIMER_1) == TIMER_1)
	{
		if(contr == Enable)
		{
			TCON |= 0x40;
		}
		else if(contr == Disable)
		{
			TCON &= 0xbf;
		}
	}
}

void InitTimer()
{
	TimerConfig(TIMER_0|TIMER_1);
	TimerStart(TIMER_0|TIMER_1,Enable);
	
}




































