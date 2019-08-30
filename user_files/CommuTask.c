#include"Stc12c5a.h"
#include"Global.h"
#include"CommuTask.h"
#include"DS18B20.h"
#include <string.h>
#include"EEProm.h"
#include"IO.h"
#include"MC20.h"

#define BAUD_9600
//#define BAUD_115200

#ifdef BAUD_9600
#define MCU_BAUD   (256-((FOSC)/16/9600))
#define MC20_BAUD  9600
#elif defined BAUD_115200
#define MCU_BAUD   (256-((FOSC)/16/115200))
#define MC20_BAUD  115200
#else
#define MCU_BAUD   (256-((FOSC)/16/9600))
#define MC20_BAUD  9600
#endif

#define REG_NUM  ((EXTER_TX_MAX_SIZE-10)/2)  //reserve 10 byte for other usage

//#define BAUD_9600   (256-((FOSC)/16/9600))
//#define BAUD_115200 (256-((FOSC)/16/115200))

XINT8U InterRxBuff[INTER_RX_MAX_SIZE];
XINT8U BackUpRxBuf[INTER_RX_MAX_SIZE];
XINT8U InterTxBuff[INTER_TX_MAX_SIZE];
XINT8U ExterRxBuff[EXTER_RX_MAX_SIZE];
XINT8U ExterTxBuff[EXTER_TX_MAX_SIZE];

xdata INT8U tbExterTempBuf[15];

idata sInterRxManage sInterRx;
idata sRxManage sExterRx;
idata sTxManage sInterTx;
idata sTxManage sExterTx;

void UartConfig(INT8U Uartx,INT8U Baud) //notice that UART1 & UART2 baud need to be set to the same(use common baud degenerator),except some special ways
{
	if((Uartx&UART_1) == UART_1)
    {
		PCON |= 0x80;  //PCON.bit7/SMOD=1; baud degenerator double
		PCON &= 0xbf;  //PCON.bit6/SMOD0=0;
		SM0 = 0;       //Mode 1,8 bit 
		SM1 = 1;
		BRT = Baud;
        AUXR |= 0x04;  //AUXR.bit2/BRTx12=1;independent baud degenerator count every 1 sysclk cycle
		AUXR |= 0x01;  //AUXR.bit4/BRTR =1; allow independent baud degenerator
		AUXR |= 0x10;  //AUXR.bit0/S1BRS=1; activate independent baud degenerator,release T1;
	}
	
	if((Uartx&UART_2) == UART_2)
	{
		S2CON &= 0x7f; //S2CON.bit7/S2SM0=0; Mode 1,8 bit,baud changeable
		S2CON |= 0x40; //S2CON.bit6/S2SM1=1;
		BRT = Baud;
		AUXR |= 0x08;  //AUXR.bit3/S2SMOD=1; baud degenerator double
		AUXR |= 0x04;  //AUXR.bit2/BRTx12=1; independent baud degenerator count every 1 sysclk cycle
		AUXR1 &= 0xef; //AUXR1.bit4/S2_P4=0; URAT2 located in P1 Port
		//AUXR1 |= 0x10; //AUXR1.bit4/S2_P4=0; URAT2 located in P4 Port
	}
}

void ContrUart_RX(INT8U Uartx,bit Contr)
{
    if((Uartx&UART_1) == UART_1)
    {
		if(Contr == Enable)
		{
			REN = 1;
		}
		else if(Contr == Disable)
		{
			REN = 0;
		}
	}

	if((Uartx&UART_2) == UART_2)
	{
		if(Contr == Enable)
		{
			S2CON |= 0x10;  //bit4/S2REN
		}
		else if(Contr == Disable)
		{
			S2CON &= 0xef;  //bit4/S2REN
		}
	}
}

bit UartReadFlg(INT8U Uartx,INT8U UartFlg)  //Read a flag one time
{
	bit StatusTemp=RESET;
	
	if(Uartx == UART_1)
	{
		switch(UartFlg)
		{
			case UART_TI_FLG:StatusTemp = TI;break;
			case UART_RI_FLG:StatusTemp = RI;break;
		}
	}
	else if(Uartx == UART_2)
	{
		switch(UartFlg)
		{
			case UART_TI_FLG:StatusTemp = (bit)(S2CON&0x02);break;
			case UART_RI_FLG:StatusTemp = (bit)(S2CON&0x01);break;
		}
	}
	
	return StatusTemp;
}

void UartClearFlg(INT8U Uartx,INT8U UartFlg)
{
	if((Uartx&UART_1) == UART_1)
	{
		if((UartFlg&UART_TI_FLG) == UART_TI_FLG)
		{
			TI = 0;
		}

		if((UartFlg&UART_RI_FLG) == UART_RI_FLG)
		{
			RI = 0;
		}
	}

	if((Uartx&UART_2) == UART_2)
	{
		if((UartFlg&UART_TI_FLG) == UART_TI_FLG)
		{
			S2CON &= 0xfd;  //S2TI=0;
		}

		if((UartFlg&UART_RI_FLG) == UART_RI_FLG)
		{
			S2CON &= 0xfe;  //S2RI=0;
		}
	}
}

void RxManageInit(INT8U RxType)
{
    if((RxType&INTER_RX) == INTER_RX)
    {
		memset(BackUpRxBuf,0,INTER_RX_MAX_SIZE);
		memset(InterRxBuff,0,INTER_RX_MAX_SIZE);
		#if 1
		sInterRx.bRxCnt=0;
		sInterRx.bRxSize=INTER_RX_MAX_SIZE;
		sInterRx.bRxStatus=BUFF_EMPTY;
		sInterRx.bRxTimeCnt=0;
		sInterRx.bRxTimeOut=RX_FINISH_TIME;
		sInterRx.pRxBuffer = InterRxBuff;
		#endif
      #if 0
		sInterRx.bReadCnt = 0;
		sInterRx.bRxCnt = 0;
		sInterRx.bRxLength = 0;
		sInterRx.bRxSize = INTER_RX_MAX_SIZE;
		sInterRx.bRxStatus = BUFF_EMPTY;
		sInterRx.pRxBuffer = InterRxBuff;
      #endif
    }

	if((RxType&EXTER_RX) == EXTER_RX)
	{
		memset(ExterRxBuff,0,EXTER_RX_MAX_SIZE);
		sExterRx.bReadCnt = 0;
		sExterRx.bRxCnt = 0;
		sExterRx.bRxLength = 0;
		sExterRx.bRxSize = EXTER_RX_MAX_SIZE;
		sExterRx.bRxStatus = BUFF_EMPTY;
		sExterRx.pRxBuffer = ExterRxBuff;
	}
}

void TxManageInit(INT8U TxType)
{
	if((TxType&INTER_TX) == INTER_TX)
	{
		memset(InterTxBuff,0,INTER_TX_MAX_SIZE);
		sInterTx.bSendLength = 0;
		sInterTx.bSendStatus = TX_READY;
		sInterTx.bTxCnt = 0;
		sInterTx.pSendBuff = InterTxBuff;
	}
	if((TxType&EXTER_TX) == EXTER_TX)
	{
		memset(ExterTxBuff,0,EXTER_TX_MAX_SIZE);
		sExterTx.bSendLength = 0;
		sExterTx.bSendStatus = TX_READY;
		sExterTx.bTxCnt = 0;
		sExterTx.pSendBuff = ExterTxBuff;
	}
}

void UartInit()
{
	UartConfig(UART_1|UART_2,MCU_BAUD);
	ContrUart_RX(UART_1|UART_2,Enable);
	RxManageInit(INTER_RX|EXTER_RX);
	TxManageInit(INTER_TX|EXTER_TX);
}

INT8U ReadRxData(INT8U* bDataTep,INT8U RxType)
{
    #if 0
	if(RxType == INTER_RX)
	{

		if(sInterRx.bRxStatus == BUFF_EMPTY)
		{
			return BUFF_EMPTY;
		}
		else
		{
			if(sInterRx.bReadCnt<sInterRx.bRxSize)
			{
				*bDataTep = *(sInterRx.pRxBuffer+sInterRx.bReadCnt);
				sInterRx.bReadCnt++;
				
				if(sInterRx.bRxLength > 0)
				{
			       sInterRx.bRxLength--;
				}
				if(sInterRx.bRxLength == 0)
				{
					sInterRx.bRxStatus = BUFF_EMPTY;
					sInterRx.bRxCnt = 0;
					sInterRx.bReadCnt = 0;
				}
				
				return RX_READY;
			}
			else
			{
				sInterRx.bRxStatus = BUFF_EMPTY;
				sInterRx.bRxCnt = 0;
				sInterRx.bReadCnt = 0;
				sInterRx.bRxLength = 0;
			}
		}
	}
	#endif

	if(RxType == EXTER_RX)
	{
		if(sExterRx.bRxStatus == BUFF_EMPTY)
		{
			return BUFF_EMPTY;
		}
		else
		{
			if(sExterRx.bReadCnt<sExterRx.bRxSize)
			{
				*bDataTep = *(sExterRx.pRxBuffer+sExterRx.bReadCnt);
				sExterRx.bReadCnt++;
				
				if(sExterRx.bRxLength > 0)
				{
			       sExterRx.bRxLength--;
				}
				if(sExterRx.bRxLength == 0)
				{
					sExterRx.bRxStatus = BUFF_EMPTY;
					sExterRx.bRxCnt = 0;
					sExterRx.bReadCnt = 0;
				}

				return RX_READY;
			}
			else
			{
				sExterRx.bRxStatus = BUFF_EMPTY;
				sExterRx.bRxCnt = 0;
				sExterRx.bReadCnt = 0;
				sExterRx.bRxLength = 0;
			}
		}
	}

	return BUFF_EMPTY;
}

INT8U UartSendData(INT8U TxType,INT8U *OutBuff,INT8U DataLen)
{
	INT8U i;
	
	if(TxType == INTER_TX)
	{
		if(sInterTx.bSendStatus == TX_BUSY)
		{
			return TX_BUSY;
		}
		sInterTx.bSendStatus = TX_BUSY;
		sInterTx.bSendLength = DataLen;
		sInterTx.bTxCnt = 0;
		for(i=0;i<DataLen;i++)
		{
			*(sInterTx.pSendBuff+i) = *(OutBuff+i);
		}
		if(sInterTx.bTxCnt<sInterTx.bSendLength)
		{
			S2BUF = *(sInterTx.pSendBuff+sInterTx.bTxCnt);  //start first send
			sInterTx.bTxCnt++;
		}
		else
		{
			sInterTx.bSendStatus = TX_READY;
		}
	}

	if(TxType == EXTER_TX)
	{
		if(sExterTx.bSendStatus == TX_BUSY)
		{
			return TX_BUSY;
		}
		sExterTx.bSendStatus = TX_BUSY;
		sExterTx.bSendLength = DataLen;
		sExterTx.bTxCnt = 0;
		for(i=0;i<DataLen;i++)
		{
			*(sExterTx.pSendBuff+i) = *(OutBuff+i);
		}
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

	return TX_READY;
}

void ReadRegister(INT16U wReadIndex,INT8U *pSendBuf)
{
	INT16U wReadDataTemp;

	wReadDataTemp = sDeviceInfoEE.ComAddr;                           //00 
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	
	if(TemperType == TEMPER_MINUS)
	{
		wReadDataTemp = wTemperCal + 10000;  //make MSB is 1
	}
	else
	{
		wReadDataTemp = wTemperCal;
	}
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);             //01 temperature
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = sDeviceInfoEE.DeviceID;                          //02
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)(sDeviceInfoEE.TimerGrp1>>16);           //03
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)(sDeviceInfoEE.TimerGrp1&0x0000ffff);    //04
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = sDeviceInfoEE.TimerGrp1_Time;                    //05
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8); 
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = sDeviceInfoEE.sDevInfBit.TimerGrp1En;            //06
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)(sDeviceInfoEE.TimerGrp2>>16);           //07
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)(sDeviceInfoEE.TimerGrp2&0x0000ffff);    //08
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = sDeviceInfoEE.TimerGrp2_Time;                    //09
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8); 
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = sDeviceInfoEE.sDevInfBit.TimerGrp2En;            //10
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)(sDeviceInfoEE.TimerGrp3>>16);           //11
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)(sDeviceInfoEE.TimerGrp3&0x0000ffff);    //12
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = sDeviceInfoEE.TimerGrp3_Time;                    //13
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8); 
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = sDeviceInfoEE.sDevInfBit.TimerGrp3En;            //14
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = sDeviceInfoEE.HighGrp1;                          //15
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = sDeviceInfoEE.HighGrp1_Time;                     //16
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = sDeviceInfoEE.sDevInfBit.HighGrp1En;             //17
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8); 
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = sDeviceInfoEE.HighGrp2;                          //18
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = sDeviceInfoEE.HighGrp2_Time;                     //19
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = sDeviceInfoEE.sDevInfBit.HighGrp2En;             //20
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = sDeviceInfoEE.HighGrp3;                          //21
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8); 
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = sDeviceInfoEE.HighGrp3_Time;                     //22
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = sDeviceInfoEE.sDevInfBit.HighGrp3En;             //23
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)(GlabalTimeCnt>>16);			         //24
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)(GlabalTimeCnt&0x0000ffff);	             //25
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)(sGPSDtBackUp.dwAtitude>>16);			 //26
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)(sGPSDtBackUp.dwAtitude&0x0000ffff);	 //27
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)(ASCIIConvert(sDeviceInfoEE.tbPhoneNum[0],CHAR_TO_NUM));  //28
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)(ASCIIConvert(sDeviceInfoEE.tbPhoneNum[1],CHAR_TO_NUM));  //29
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)(ASCIIConvert(sDeviceInfoEE.tbPhoneNum[2],CHAR_TO_NUM));  //30
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)(ASCIIConvert(sDeviceInfoEE.tbPhoneNum[3],CHAR_TO_NUM)); //31
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)(ASCIIConvert(sDeviceInfoEE.tbPhoneNum[4],CHAR_TO_NUM));  //32
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)(ASCIIConvert(sDeviceInfoEE.tbPhoneNum[5],CHAR_TO_NUM));  //33
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)(ASCIIConvert(sDeviceInfoEE.tbPhoneNum[6],CHAR_TO_NUM));  //34
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)(ASCIIConvert(sDeviceInfoEE.tbPhoneNum[7],CHAR_TO_NUM));  //35
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)(ASCIIConvert(sDeviceInfoEE.tbPhoneNum[8],CHAR_TO_NUM));  //36
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)(ASCIIConvert(sDeviceInfoEE.tbPhoneNum[9],CHAR_TO_NUM));  //37
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)(ASCIIConvert(sDeviceInfoEE.tbPhoneNum[10],CHAR_TO_NUM));  //38
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = 0;                                                                 //39 //recover eeprom initial data
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
    wReadDataTemp = sUserInfoEE.wFaultRecord;                                          //40
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)(sUserInfoEE.GlobalTimeRecor>>16);			               //41
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)(sUserInfoEE.GlobalTimeRecor&0x0000ffff);	               //42
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)((INT32U)sUserInfoEE.MostAtitu>>16);			           //43
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
	wReadDataTemp = (INT16U)((INT32U)sUserInfoEE.MostAtitu&0x0000ffff);	               //44
	pSendBuf[wReadIndex++] = (INT8U)(wReadDataTemp>>8);
	pSendBuf[wReadIndex++] = (INT8U)wReadDataTemp;
}

INT8U SetRegister(INT8U bSetAddr,INT16U wSetData,INT8U *pReturnBuf)
{
	INT8U bSetMessage=0;
	INT16U CrcTemp=0;
	
	switch(bSetAddr)
	{
		case 0:
		{
			sDeviceInfoEE.ComAddr = wSetData;
			EprNeedWrt|=DEVICE_BLOCK;
			
			pReturnBuf[4] = (INT8U)0;
			pReturnBuf[5] = (INT8U)(sDeviceInfoEE.ComAddr);
			break;
		}
		case 2:
		{
			sDeviceInfoEE.DeviceID = wSetData;
			EprNeedWrt|=DEVICE_BLOCK;
			
			pReturnBuf[4] = (INT8U)(sDeviceInfoEE.DeviceID>>8);
			pReturnBuf[5] = (INT8U)(sDeviceInfoEE.DeviceID);
			break;
		}
		case 3:
		{
			sDeviceInfoEE.TimerGrp1 &= 0x0000ffff;
			sDeviceInfoEE.TimerGrp1 |= (INT32U)wSetData<<16;
			EprNeedWrt|=DEVICE_BLOCK;
			
			pReturnBuf[4] = (INT8U)(sDeviceInfoEE.TimerGrp1>>24);
			pReturnBuf[5] = (INT8U)(sDeviceInfoEE.TimerGrp1>>16);
			break;
		}
		case 4:
		{
			sDeviceInfoEE.TimerGrp1 &= 0xffff0000;
			sDeviceInfoEE.TimerGrp1 |= (INT32U)wSetData;
			EprNeedWrt|=DEVICE_BLOCK;
			
			pReturnBuf[4] = (INT8U)(sDeviceInfoEE.TimerGrp1 >> 8);
			pReturnBuf[5] = (INT8U)(sDeviceInfoEE.TimerGrp1);
			break;
		}
		case 5:
		{
			if(wSetData<3000)
			{
				sDeviceInfoEE.TimerGrp1_Time = wSetData;
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)(sDeviceInfoEE.TimerGrp1_Time >> 8);
			pReturnBuf[5] = (INT8U)(sDeviceInfoEE.TimerGrp1_Time);
			break;
		}
        case 6:
		{
			if(wSetData<=1)
			{
				sDeviceInfoEE.sDevInfBit.TimerGrp1En = wSetData;
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)0;
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.sDevInfBit.TimerGrp1En;
			break;
		}
        case 7:
		{
			sDeviceInfoEE.TimerGrp2 &= 0x0000ffff;
			sDeviceInfoEE.TimerGrp2 |= (INT32U)wSetData<<16;
			EprNeedWrt|=DEVICE_BLOCK;
			
			pReturnBuf[4] = (INT8U)(sDeviceInfoEE.TimerGrp2>>24);
			pReturnBuf[5] = (INT8U)(sDeviceInfoEE.TimerGrp2>>16);
			break;
		}
		case 8:
		{
			sDeviceInfoEE.TimerGrp2 &= 0xffff0000;
			sDeviceInfoEE.TimerGrp2 |= (INT32U)wSetData;
			EprNeedWrt|=DEVICE_BLOCK;
			
			pReturnBuf[4] = (INT8U)(sDeviceInfoEE.TimerGrp2 >> 8);
			pReturnBuf[5] = (INT8U)(sDeviceInfoEE.TimerGrp2);
			break;
		}
		case 9:
		{
			if(wSetData<3000)
			{
				sDeviceInfoEE.TimerGrp2_Time = wSetData;
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)(sDeviceInfoEE.TimerGrp2_Time >> 8);
			pReturnBuf[5] = (INT8U)(sDeviceInfoEE.TimerGrp2_Time);
			break;
		}
        case 10:
		{
			if(wSetData<=1)
			{
				sDeviceInfoEE.sDevInfBit.TimerGrp2En = wSetData;
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)0;
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.sDevInfBit.TimerGrp2En;
			break;
		}
		case 11:
		{
			sDeviceInfoEE.TimerGrp3 &= 0x0000ffff;
			sDeviceInfoEE.TimerGrp3 |= (INT32U)wSetData<<16;
			EprNeedWrt|=DEVICE_BLOCK;
			
			pReturnBuf[4] = (INT8U)(sDeviceInfoEE.TimerGrp3>>24);
			pReturnBuf[5] = (INT8U)(sDeviceInfoEE.TimerGrp3>>16);
			break;
		}
		case 12:
		{
			sDeviceInfoEE.TimerGrp3 &= 0xffff0000;
			sDeviceInfoEE.TimerGrp3 |= (INT32U)wSetData;
			EprNeedWrt|=DEVICE_BLOCK;
			
			pReturnBuf[4] = (INT8U)(sDeviceInfoEE.TimerGrp3 >> 8);
			pReturnBuf[5] = (INT8U)(sDeviceInfoEE.TimerGrp3);
			break;
		}
		case 13:
		{
			if(wSetData<3000)
			{
				sDeviceInfoEE.TimerGrp3_Time = wSetData;
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)(sDeviceInfoEE.TimerGrp3_Time >> 8);
			pReturnBuf[5] = (INT8U)(sDeviceInfoEE.TimerGrp3_Time);
			break;
		}
        case 14:
		{
			if(wSetData<=1)
			{
				sDeviceInfoEE.sDevInfBit.TimerGrp3En = wSetData;
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)0;
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.sDevInfBit.TimerGrp3En;
			break;
		}
        case 15:
		{
			if(wSetData<=50000)
			{
				sDeviceInfoEE.HighGrp1 = wSetData;
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)(sDeviceInfoEE.HighGrp1>>8);
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.HighGrp1;
			break;
		}
		case 16:
		{
			if(wSetData<=3000)
			{
				sDeviceInfoEE.HighGrp1_Time = wSetData;
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)(sDeviceInfoEE.HighGrp1_Time>>8);
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.HighGrp1_Time;
			break;
		}
        case 17:
		{
			if(wSetData<=1)
			{
				sDeviceInfoEE.sDevInfBit.HighGrp1En = wSetData;
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)0;
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.sDevInfBit.HighGrp1En;
			break;
        }
		case 18:
		{
			if(wSetData<=50000)
			{
				sDeviceInfoEE.HighGrp2 = wSetData;
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)(sDeviceInfoEE.HighGrp2>>8);
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.HighGrp2;
			break;
		}
		case 19:
		{
			if(wSetData<=3000)
			{
				sDeviceInfoEE.HighGrp2_Time = wSetData;
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)(sDeviceInfoEE.HighGrp2_Time>>8);
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.HighGrp2_Time;
			break;
		}
        case 20:
		{
			if(wSetData<=1)
			{
				sDeviceInfoEE.sDevInfBit.HighGrp2En = wSetData;
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)0;
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.sDevInfBit.HighGrp2En;
			break;
        }
		case 21:
		{
			if(wSetData<=50000)
			{
				sDeviceInfoEE.HighGrp3 = wSetData;
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)(sDeviceInfoEE.HighGrp3>>8);
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.HighGrp3;
			break;
		}
		case 22:
		{
			if(wSetData<=3000)
			{
				sDeviceInfoEE.HighGrp3_Time = wSetData;
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)(sDeviceInfoEE.HighGrp3_Time>>8);
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.HighGrp3_Time;
			break;
		}
        case 23:
		{
			if(wSetData<=1)
			{
				sDeviceInfoEE.sDevInfBit.HighGrp3En = wSetData;
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)0;
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.sDevInfBit.HighGrp3En;
			break;
        }
		case 28:
		{
			if(wSetData<=9)
			{
				sDeviceInfoEE.tbPhoneNum[0] = ASCIIConvert(wSetData,NUM_TO_CHAR);
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)0;
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.tbPhoneNum[0];
			break;
        }
		case 29:
		{
			if(wSetData<=9)
			{
				sDeviceInfoEE.tbPhoneNum[1] = ASCIIConvert(wSetData,NUM_TO_CHAR);
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)0;
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.tbPhoneNum[1];
			break;
        }
		case 30:
		{
			if(wSetData<=9)
			{
				sDeviceInfoEE.tbPhoneNum[2] = ASCIIConvert(wSetData,NUM_TO_CHAR);
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)0;
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.tbPhoneNum[2];
			break;
        }
		case 31:
		{
			if(wSetData<=9)
			{
				sDeviceInfoEE.tbPhoneNum[3] = ASCIIConvert(wSetData,NUM_TO_CHAR);
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)0;
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.tbPhoneNum[3];
			break;
        }
		case 32:
		{
			if(wSetData<=9)
			{
				sDeviceInfoEE.tbPhoneNum[4] = ASCIIConvert(wSetData,NUM_TO_CHAR);
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)0;
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.tbPhoneNum[4];
			break;
        }
		case 33:
		{
			if(wSetData<=9)
			{
				sDeviceInfoEE.tbPhoneNum[5] = ASCIIConvert(wSetData,NUM_TO_CHAR);
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)0;
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.tbPhoneNum[5];
			break;
        }
		case 34:
		{
			if(wSetData<=9)
			{
				sDeviceInfoEE.tbPhoneNum[6] = ASCIIConvert(wSetData,NUM_TO_CHAR);
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)0;
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.tbPhoneNum[6];
			break;
        }
		case 35:
		{
			if(wSetData<=9)
			{
				sDeviceInfoEE.tbPhoneNum[7] = ASCIIConvert(wSetData,NUM_TO_CHAR);
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)0;
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.tbPhoneNum[7];
			break;
        }
		case 36:
		{
			if(wSetData<=9)
			{
				sDeviceInfoEE.tbPhoneNum[8] = ASCIIConvert(wSetData,NUM_TO_CHAR);
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)0;
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.tbPhoneNum[8];
			break;
        }
		case 37:
		{
			if(wSetData<=9)
			{
				sDeviceInfoEE.tbPhoneNum[9] = ASCIIConvert(wSetData,NUM_TO_CHAR);
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)0;
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.tbPhoneNum[9];
			break;
        }
		case 38:
		{
			if(wSetData<=9)
			{
				sDeviceInfoEE.tbPhoneNum[10] = ASCIIConvert(wSetData,NUM_TO_CHAR);
				EprNeedWrt|=DEVICE_BLOCK;
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)0;
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.tbPhoneNum[10];
			break;
        }
		case 39:
		{
			if(wSetData == 1)
			{
				EEprDtInit();
				EprNeedWrt|=(DEVICE_BLOCK|USER_BLOCK);
			}
			else
			{
				bSetMessage = 1;
			}
			pReturnBuf[4] = (INT8U)0;
			pReturnBuf[5] = (INT8U)sDeviceInfoEE.tbPhoneNum[10];
			break;
        }
		default:
		{
			bSetMessage = 1;
			pReturnBuf[4] = 0;
			pReturnBuf[5] = 0;
			break;
		}
	}

	if(bSetMessage == 0)
	{
		CrcTemp = usMBCRC16(ExterTxBuff,6);
		ExterTxBuff[6] = (INT8U)CrcTemp;
		ExterTxBuff[7] = (INT8U)(CrcTemp>>8);
		return 8;   //length = 8
	}
	else
	{
		ExterTxBuff[1] = 0x06|0x08;
		ExterTxBuff[2] = 0x01;
		CrcTemp = usMBCRC16(ExterTxBuff,3);
		ExterTxBuff[3] = (INT8U)CrcTemp;
		ExterTxBuff[4] = (INT8U)(CrcTemp>>8);
		return 5;   //length = 5
	}
}

void ProtocolParse(INT8U *bDataBuf)
{
	INT16U wStartAdrr;
	INT16U wNumOfReg;
	INT16U wSetData;
	INT8U bSendLenth;
	INT16U CrcTemp;
	INT8U i=0;

	memset(ExterTxBuff,0,EXTER_TX_MAX_SIZE);

	if((bDataBuf[0] != sDeviceInfoEE.ComAddr) && (bDataBuf[0] != 0))
	{
		return;
	}
	
    ExterTxBuff[0] = sDeviceInfoEE.ComAddr;  //device addrress
    ExterTxBuff[1] = bDataBuf[1];  
	
	if(bDataBuf[1] == 0x03)
	{
		wStartAdrr = bDataBuf[2]*256 + bDataBuf[3];
		wNumOfReg = bDataBuf[4]*256 + bDataBuf[5];
		
		if((wNumOfReg<(REG_NUM-wStartAdrr))&&(wStartAdrr<REG_NUM))
		{
			ReadRegister(3,ExterTxBuff);

			ExterTxBuff[2] = wNumOfReg*2;

			for(i=0;i<(wNumOfReg*2);i++)
			{
				ExterTxBuff[i+3] = ExterTxBuff[3+i+wStartAdrr*2];
			}
			
            bSendLenth = 3+wNumOfReg*2;  //head 3 byte + wNumOfReg*2
            
			CrcTemp = usMBCRC16(ExterTxBuff,bSendLenth);
			ExterTxBuff[3+wNumOfReg*2] = (INT8U)CrcTemp;  //CRC
			ExterTxBuff[4+wNumOfReg*2] = (INT8U)(CrcTemp>>8);
			bSendLenth = bSendLenth+2;  // + crc two byte
		}
		else
		{
			ExterTxBuff[1] = 0x03|0x08;
			ExterTxBuff[2] = 0x01;
			CrcTemp = usMBCRC16(ExterTxBuff,3);
			ExterTxBuff[3] = (INT8U)CrcTemp;
			ExterTxBuff[4] = (INT8U)(CrcTemp>>8);
			bSendLenth = 5;
		}
		
		while(UartSendData(EXTER_TX,ExterTxBuff,bSendLenth) == TX_BUSY);
	}
	else if(bDataBuf[1] == 0x06)
	{
		wStartAdrr = bDataBuf[2]*256 + bDataBuf[3];
		wSetData = bDataBuf[4]*256 + bDataBuf[5];
		
		ExterTxBuff[2] = bDataBuf[2];
		ExterTxBuff[3] = bDataBuf[3];
		if(wStartAdrr>=REG_NUM)
		{
			ExterTxBuff[1] = 0x06|0x08;
			ExterTxBuff[2] = 0x01;
			CrcTemp = usMBCRC16(ExterTxBuff,3);
			ExterTxBuff[3] = (INT8U)CrcTemp;
			ExterTxBuff[4] = (INT8U)(CrcTemp>>8);
			bSendLenth = 5;
		}
		else
		{
			bSendLenth = SetRegister(wStartAdrr,wSetData,ExterTxBuff);
		}
		
		while(UartSendData(EXTER_TX,ExterTxBuff,bSendLenth) == TX_BUSY);
	}
}

void ExterComTask(INT8U TaskNum)
{
	static INT8U bReadIndex=0;
	INT8U bDataTemp;
	INT16U CrcTemp=0;
	
	TaskStaChk(TaskNum);
	
	bReadIndex = 0;
	while(ReadRxData(&bDataTemp,EXTER_RX) == RX_READY)
    {
		switch(bReadIndex)
		{
			case 0:
			{
				tbExterTempBuf[bReadIndex] = bDataTemp;
				bReadIndex++;
				break;
			}
			case 1:
			{
				if((bDataTemp == 0x03) || (bDataTemp == 0x06))
				{
					tbExterTempBuf[bReadIndex] = bDataTemp;
					bReadIndex++;
				}
				else
				{
					bReadIndex = 0;
				}
				break;
			}
			default:
			{
				if(bReadIndex<8)
				{
					tbExterTempBuf[bReadIndex] = bDataTemp;
					bReadIndex++;
					if(bReadIndex >= 8)
					{
						bReadIndex = 0;
						CrcTemp = usMBCRC16(tbExterTempBuf,6);
						if((tbExterTempBuf[6]==(CrcTemp&0xff))&&(tbExterTempBuf[7]==(CrcTemp>>8)))  //crc check
						{
						   ProtocolParse(tbExterTempBuf);
						}
					}
				}
		
				break;
			}
		}
	}
	
	TaskPend(TaskNum);
}























