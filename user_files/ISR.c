#include"Stc12c5a.h"
#include"Global.h"
#include"Timer.h"
#include"CommuTask.h"


static bit TickCntStart=0;
static INT16U bTickCnt=0;
static INT8U b1sCnt=0;

void delayN_Tick(INT16U bTickNum)
{
	bTickCnt = bTickNum;
	TickCntStart = 1;
	while(TickCntStart==1);
}

//*************************************************************************
//******************************  Timer  **********************************

void Timer0ISR() interrupt TIMER0_VEC   //5ms
{
	TaskManage *Temp;
	INT8U taskNum;
	
	SetTimerCnt(TIMER_0,T0_CNT);
	
	for(taskNum=0;taskNum<MAX_TASK_NUM;taskNum++)
	{
		Temp = &TaskManaTble[taskNum];
		if(Temp->TaskStatus == TASK_WAIT)
		{
			Temp->TimeCnt--;
			if(Temp->TimeCnt == 0)
			{
				Temp->TaskStatus = TASK_READY;
				Temp->TimeCnt = Temp->Period;
			}
		}
	}
	
	if(TickCntStart == 1)
	{
		if(bTickCnt>0)
		{
			bTickCnt--;
		}
		else
		{
			TickCntStart = 0;
		}
	}

	if(sInterRx.bRxStatus == RX_READY)
	{
		sInterRx.bRxTimeCnt++;
		if(sInterRx.bRxTimeCnt>RX_FINISH_TIME)
		{
			sInterRx.bRxStatus = RX_FINISH;
			sInterRx.bRxCnt = 0;
			sInterRx.bRxTimeCnt = 0;
		}
	}
}

void Timer1ISR() interrupt TIMER1_VEC using 1 //50ms
{
	SetTimerCnt(TIMER_1,T1_CNT);
	b1sCnt++;
	if(b1sCnt>=20)
	{
		b1sCnt = 0;
		GlabalTimeCnt++;
	}
}                                 

//********************************************************************************
//*********************************  Uart  ***************************************
#if 1
void InterComISR() interrupt UART2_VEC
{
	INT8U bTemp;
	
	if(UartReadFlg(UART_2,UART_RI_FLG) == SET)
	{
		UartClearFlg(UART_2,UART_RI_FLG);

		if(sInterRx.bRxCnt < sInterRx.bRxSize)
		{
			*(sInterRx.pRxBuffer+sInterRx.bRxCnt) = S2BUF;
			sInterRx.bRxCnt++;
		}
		else
		{
			bTemp = S2BUF;
		}
		sInterRx.bRxTimeCnt = 0;
		sInterRx.bRxStatus = RX_READY;
	}

	if(UartReadFlg(UART_2,UART_TI_FLG) == SET)
	{
		UartClearFlg(UART_2,UART_TI_FLG);
		
		if(sInterTx.bTxCnt<sInterTx.bSendLength)
		{
			S2BUF = *(sInterTx.pSendBuff+sInterTx.bTxCnt);  
			sInterTx.bTxCnt++;
		}
		else
		{
			sInterTx.bSendStatus = TX_READY;
		}
	}
}
#endif

#if 0
void InterComISR() interrupt UART1_VEC
{
	INT8U bTemp;
	if(UartReadFlg(UART_1,UART_RI_FLG) == SET)
	{
		UartClearFlg(UART_1,UART_RI_FLG);
		
		if(sInterRx.bRxCnt < sInterRx.bRxSize)
		{
			*(sInterRx.pRxBuffer+sInterRx.bRxCnt) = SBUF;
			sInterRx.bRxCnt++;
			sInterRx.bRxLength++;
		}
		else
		{
			bTemp = SBUF;
		}
		sInterRx.bRxStatus = RX_READY;
	}

	if(UartReadFlg(UART_1,UART_TI_FLG) == SET)
	{
		UartClearFlg(UART_1,UART_TI_FLG);
		
		if(sInterTx.bTxCnt<sInterTx.bSendLength)
		{
			SBUF = *(sInterTx.pSendBuff+sInterTx.bTxCnt);  
			sInterTx.bTxCnt++;
		}
		else
		{
			sInterTx.bSendStatus = TX_READY;
		}
	}
}
#endif

void ExterComISR() interrupt UART1_VEC
{
	INT8U bTemp;
	
	if(UartReadFlg(UART_1,UART_RI_FLG) == SET)
	{
		UartClearFlg(UART_1,UART_RI_FLG);
	    if(sExterRx.bRxCnt < sExterRx.bRxSize)
		{
			*(sExterRx.pRxBuffer+sExterRx.bRxCnt) = SBUF;
			sExterRx.bRxCnt++;
			sExterRx.bRxLength++;
		}
		else
		{
			bTemp = SBUF;
		}
		sExterRx.bRxStatus = RX_READY;
	}

	if(UartReadFlg(UART_1,UART_TI_FLG) == SET)
	{
		UartClearFlg(UART_1,UART_TI_FLG);
		
		if(sExterTx.bTxCnt<sExterTx.bSendLength)
		{
			SBUF = *(sExterTx.pSendBuff+sExterTx.bTxCnt);  //start first send
			sExterTx.bTxCnt++;
		}
		else
		{
			sExterTx.bSendStatus = TX_READY;
		}
	}
}
































