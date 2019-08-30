#include"Stc12c5a.h"
#include"Global.h"
#include"CommuTask.h"
#include <string.h>
#include"IO.h"
#include"MC20.h"
#include"EEProm.h"

#define USUAL_WAIT      10
#define MESSAGE_WAIT    400  //2s,when send 0x0a to start sending message,it need much time to response ok

#define START_DELAY     2400 //12s

#define WAIT_RESPONES_T  6   //total 5x10x3x5=750ms
#define RETRY_TIME       2

#define NO_MSG       0
#define RECIEV_MSG   1
#define READ_FAIL    2

#define REQUIR_AGAIN 0
#define NEED_SEND    1
#define SEND_FINISH  2

#define LATITUDE     3
#define LATI_DIR     4
#define LONGTITU     5
#define LONG_DIR     6
#define FIX_STAT     7
#define PRECISION    9
#define ATITUDE      10

#define DATAx100     100

#define STA_UP      0
#define STA_DOWN    1
#define STA_PARA    2
#define STA_UNKNOW  3

#define UNDER_LEVER_1  0x01   //150M
#define UNDER_LEVER_2  0x02   //500M
#define UNDER_LEVER_3  0x04   //100M
#define UNDER_LEVER_4  0x08
#define SEND_MSG_FLAG  0x10

#define HIGH_LEVER_1  100
#define HIGH_LEVER_2  600
#define HIGH_LEVER_3  1200
#define HIGH_LEVER_4  1600
#define HIGH_LEVER_5  500

#define UP_CNT            3
#define LOCAT_CNT         75
#define GET_DT_TIME       5   //1s
#define GO_BACK_CHK_TIME  20  //4s
#define CONTINOUS_TIME    (GET_DT_TIME*5)  //success 5 times

#define CNT_TIME        2

#define MSG_BUFF_LEN  20

xdata INT8U tReadMsg[13]="AT+CMGR=     ";
xdata INT8U tbMsgPhone[26]="AT+CMGS=\"           \"\r\n";
xdata INT8U tbSendMsgBuff[MSG_BUFF_LEN] = "                  ";  //initial value
xdata INT8U tbNumChk[4];

code INT8U tStartSend[2]={0x1A,0x00};

code INT8U CmdTable[3][20]=
{
	"get location",
	"get backup",
	"motor",
};
	
code INT8U SendTable[5][MSG_BUFF_LEN]=
{
	"Invalid command!",
	"Locate fail!",
	"GPS fail!",
	"start ok!",
	"start fail!",
};
	
xdata sGPSDtManage sGPSData;
xdata sGPSDtBkUp sGPSDtBackUp;
xdata sMsgStr sMsgManage;

static INT8U AtiSymbol=0;
static INT8U BalonStaRecor=STA_UNKNOW;  
static INT8U BalonAlreadyUp=0;


//*******************************************************************************
void ClrInterRxBuf()
{
	memset(InterRxBuff, 0, INTER_RX_MAX_SIZE);
	sInterRx.bRxCnt=0;
	sInterRx.bRxStatus = BUFF_EMPTY;
	sInterRx.bRxTimeCnt=0;
}

INT8U MC20SendCmd(INT8U *pSendBuff,INT8U *pResponseStr,INT16U bWaitTime)
{
	INT8U bSendLenCnt=0;
	INT8U i,m,n;
	
	for(i=0;i<INTER_TX_MAX_SIZE;i++)
	{
		if(*(pSendBuff+bSendLenCnt) !='\0')
		{
			bSendLenCnt++;
		}
		else
		{
			break;
		}
	}
	
	memset(BackUpRxBuf,0,INTER_RX_MAX_SIZE);
	
	for(i=0;i<RETRY_TIME;i++)
	{
		ClrInterRxBuf();
		while(UartSendData(INTER_TX,pSendBuff,bSendLenCnt) == TX_BUSY);
 
		for(m=0;m<WAIT_RESPONES_T;m++)
		{
			delayN_Tick(bWaitTime);

			#if 0
		    if(m<2)
		    {
				while(UartSendData(EXTER_TX,InterRxBuff,100) == TX_BUSY);
				delayN_Tick(20);
		    }
            #endif
			
			if (strstr(InterRxBuff, pResponseStr) != NULL)
			{
				for(n=0;n<INTER_RX_MAX_SIZE;n++)
				{
					BackUpRxBuf[n] = InterRxBuff[n];
				}
				
				ClrInterRxBuf();
				return SUCCESS;
			}
		}
	}

	return FAIL;
}

void MC20Start()
{
	INT8U i;
	INT8U SuccCnt=0;

	MC20PowerCtr(SHUT_DOWN);  //in case MCU reset unexpectly,in this case,mc20 should restart

	delayN_Tick(400);  //2s

	MC20PowerCtr(START);

	delayN_Tick(200);  //1s
	
	Set_IO(POWE_KEY_PORT,POWE_KEY_PIN);  //Pull down POW_KEY PIN
	
	delayN_Tick(START_DELAY);  //12s

	for(i=0;i<4;i++)
	{
		if(MC20SendCmd("AT\r\n","OK\r\n",USUAL_WAIT) == SUCCESS)  //check if AT cmd is executived
		{
			SuccCnt++;
		}
	}
	
	Reset_IO(POWE_KEY_PORT,POWE_KEY_PIN);  
	
    if(SuccCnt<=2)
    {
		SetFault(MC20_FAITAL_FAULT);  
	}
}

void BasicInfoChk()
{
	if(MC20SendCmd("AT+CPIN?\r\n","READY",USUAL_WAIT) == SUCCESS)
	{
		if(MC20SendCmd("AT+CREG?\r\n", "1",USUAL_WAIT) == SUCCESS)
		{
			ClrFault(MC20_BASE_FAULT);
		}
		else if(MC20SendCmd("AT+CREG?\r\n", "5",USUAL_WAIT) == SUCCESS)
		{
			ClrFault(MC20_BASE_FAULT);
		}
        else
        {
			SetFault(MC20_BASE_FAULT);
		}
	}
	else
	{
		SetFault(MC20_BASE_FAULT);
	}
}

void MC20InitBaud()
{
	if(MC20SendCmd("AT+IPR=9600\r\n","OK\r\n",USUAL_WAIT) == SUCCESS)  //set baud 
	{
		MC20SendCmd("AT&W\r\n","OK\r\n",USUAL_WAIT);  //preserve setting
		ClrFault(MC20_BAUD_FAULT);
	}
	else
	{
		SetFault(MC20_BAUD_FAULT);
	}
}

void MC20StartGNSS()
{
	if(MC20SendCmd("AT+QGNSSC?\r\n","+QGNSSC: 0",USUAL_WAIT) == SUCCESS)  //if GNSS was not started
	{
		if(MC20SendCmd("AT+QGNSSC=1\r\n","OK\r\n",USUAL_WAIT) != SUCCESS)	//start GNSS
		{
			SetFault(MC20_FAITAL_FAULT);
		}
	}
}

void MC20InitMessage()
{
	sMsgManage.DealStatus = SEND_FINISH;
	sMsgManage.pMsgContent = SendTable[0];
	sMsgManage.tbPhoneNum = sDeviceInfoEE.tbPhoneNum;
	memcpy(sMsgManage.tbPhoneBuff,sDeviceInfoEE.tbPhoneNum,PHONE_SIZE);
	memset(sMsgManage.tbCommand,'0',MSGCMD_SEZE);
	
	ClrFault(MC20_MSG_FAULT);
	
	if(MC20SendCmd("AT+CSCS=\"GSM\"\r\n","OK\r\n",USUAL_WAIT) != SUCCESS)	 //set terminal equipment input char group is "GSM"
	{
		SetFault(MC20_MSG_FAULT);
	}
	if(MC20SendCmd("AT+CNMI=2,1\r\n","OK\r\n",USUAL_WAIT) != SUCCESS)  //message firstly store in SIM,then give a signal
	{
		SetFault(MC20_MSG_FAULT);			
	}
	if(MC20SendCmd("AT+CMGF=1\r\n","OK\r\n",USUAL_WAIT) != SUCCESS)	//message tex mode,not PDU mode
	{
		SetFault(MC20_MSG_FAULT);	  	
	}
	if(MC20SendCmd("AT+CSDH=0\r\n","OK\r\n",USUAL_WAIT) != SUCCESS)	//text parameter mode display
	{
		SetFault(MC20_MSG_FAULT);	  	
	}
#if 0
	if(MC20SendCmd("AT+CPMS=\"SM\",\"SM\",\"SM\"\r\n","OK\r\n",USUAL_WAIT) != SUCCESS)  //select message storage memory(in SIM card)
	{
		SetFault(MC20_MSG_FAULT);
	}
#endif
}

INT8U ChkIfMsgIsFull()
{
	INT8U i;
	static INT8U TimeCnt30s=0;

	if(TimeCnt30s<150)
	{
		TimeCnt30s++;
		return FAIL;
	}
	else
	{
		TimeCnt30s = 0;
	}

	if(MC20SendCmd("AT+CPMS?\r\n","OK\r\n",USUAL_WAIT) == SUCCESS)
	{
		for(i=0;i<INTER_RX_MAX_SIZE;i++)
	    {
			if((BackUpRxBuf[i]==0x0d)&&(BackUpRxBuf[i+1]==0x0a))
			{
				if((BackUpRxBuf[i+15] != ',')&&(((BackUpRxBuf[i+14] == 0x34)&&(BackUpRxBuf[i+15] > 0x35)) ||(BackUpRxBuf[i+14] == 0x35))) //if message num is >45
				{
					if(MC20SendCmd("AT+CMGD=1,4\r\n","OK\r\n",USUAL_WAIT) == SUCCESS)  //delete all message
					{
						return SUCCESS;
					}
					else
					{
						return FAIL;
					}
				}
				
				return SUCCESS;
			}
	    }
	}
	
	SetFault(MC20_MSG_FAULT);
	return FAIL;
}

INT8U SendMessage(INT8U *bContent,INT8U *Phone)
{
	INT8U i;
	
	for(i=0;i<3;i++)
	{
		if(MC20SendCmd(Phone,">",USUAL_WAIT) == SUCCESS)     //wait to edit message
		{
			if(MC20SendCmd(bContent,bContent,MESSAGE_WAIT) == SUCCESS)
			{
				if(MC20SendCmd(tStartSend,"OK\r\n",MESSAGE_WAIT) == SUCCESS)  //send 0x1A to start sending
				{
					ClrFault(MC20_MSG_FAULT);
					return SUCCESS;
				}
			}
		}
	}
	
	SetFault(MC20_MSG_FAULT);
	
	return FAIL;
}

INT8U ReadMessage()
{
	INT8U m,n=0;
	INT8U bPositionCnt=0;

	if(sInterRx.bRxStatus != RX_FINISH)
	{
		return NO_MSG;
	}

	if((InterRxBuff[2]=='+')&&(InterRxBuff[3]=='C')&&(InterRxBuff[4]=='M')&&(InterRxBuff[5]=='T')&&(InterRxBuff[6]=='I'))
	{
		if((InterRxBuff[15] == 0x0d) &&(InterRxBuff[16] == 0x0a))
		{
			tReadMsg[8] = InterRxBuff[14];
			tReadMsg[9] = '\r';
			tReadMsg[10] = '\n';
			tReadMsg[11] = '\0';
		}
		else if((InterRxBuff[16] == 0x0d) &&(InterRxBuff[17] == 0x0a))
		{
			tReadMsg[8] = InterRxBuff[14];
			tReadMsg[9] = InterRxBuff[15];
			tReadMsg[10] = '\r';
			tReadMsg[11] = '\n';
			tReadMsg[12] = '\0';
		}
		
		if(MC20SendCmd(tReadMsg,"OK\r\n",USUAL_WAIT) == SUCCESS)
		{
			memset(sMsgManage.tbCommand,0,MSGCMD_SEZE);
			memset(sMsgManage.tbPhoneBuff,0,PHONE_SIZE);
			
			for(m=0;m<(INTER_RX_MAX_SIZE);m++)  
			{
				if((BackUpRxBuf[m]=='+')&&(BackUpRxBuf[m+1]=='8')&&(BackUpRxBuf[m+2]=='6')) //china +86
				{
					memcpy(sMsgManage.tbPhoneBuff,&BackUpRxBuf[m+3],PHONE_SIZE);
				}
				
				if((BackUpRxBuf[m]==0x0d)&&(BackUpRxBuf[m+1]==0x0a))
				{
					bPositionCnt++;
				    if(bPositionCnt == 2)
					{
					    memcpy(sMsgManage.tbCommand,&BackUpRxBuf[m+2],MSGCMD_SEZE);
						sMsgManage.tbCommand[MSGCMD_SEZE-1] = '\0';
						return RECIEV_MSG;
					}
				}
			}
		}
		
		return READ_FAIL;
	}
	
	return NO_MSG;
}
void GPSDtInit()
{
	sGPSData.sLatitude.bMark[0] = '0';
	sGPSData.sLatitude.bMark[1] = ':';
	memset(sGPSData.sLatitude.bGrade,'0',3);
	sGPSData.sLatitude.bGrade[2] = '.';
	memset(sGPSData.sLatitude.dwMinute,'0',6);
	sGPSData.sLatitude.tbEnter = '\n';

	sGPSData.sLongitude.bMark[0] = '0';
	sGPSData.sLongitude.bMark[1] = ':';	
	memset(sGPSData.sLongitude.bGrade,'0',4);
	sGPSData.sLongitude.bGrade[3] = '.';
	memset(sGPSData.sLongitude.dwMinute,'0',6);
	sGPSData.sLongitude.tbEnter = '\n';

	sGPSData.sAtitude.bMark[0] = 'H';
	sGPSData.sAtitude.bMark[1] = ':';	
	memset(sGPSData.sAtitude.tbAtitu,'0',GPS_ATITU_LEN);
	sGPSData.sAtitude.tbEnter = '\n';
	
	sGPSData.sFixStat.bMark[0] = 'S';
	sGPSData.sFixStat.bMark[1] = ':';	
	sGPSData.sFixStat.bFixStatus = '0';
	sGPSData.sFixStat.tbEnter = '\n';

	sGPSData.sPrecision.bMark[0] = 'P';
	sGPSData.sPrecision.bMark[1] = ':';	
	memset(sGPSData.sPrecision.tbPrecision,'0',GPS_PRECI_LEN);
	sGPSData.sPrecision.tbEnter = '\n';

	sGPSData.sTemperSta.bMark[0] = 'T';
	sGPSData.sTemperSta.bMark[1] = ':';	
	memset(sGPSData.sTemperSta.tbTemper,'0',TEMPER_LEN);
	sGPSData.sTemperSta.tbEnter = '\n';
	
	sGPSData.sBalloonSta.bMark[0] = 'B';
	sGPSData.sBalloonSta.bMark[1] = ':';
	memset(sGPSData.sBalloonSta.tbBloStat,'0',BALLOON_STA_LEN);
	//sGPSData.sBalloonSta.tbEnter = '\n';
	
	sGPSData.bEndChar = '\0';
	
    sGPSData.dwLatiMinut = 0;
	sGPSData.dwLontiMinut = 0;
	sGPSData.dwAtitude = 0;
	memset(sGPSData.tbDataBuf,0,GPS_BUF_LEN);

	memcpy(sGPSDtBackUp.sLatitude.bMark,sGPSData.sLatitude.bMark,12);
	memcpy(sGPSDtBackUp.sLongitude.bMark,sGPSData.sLongitude.bMark,13);
	memcpy(sGPSDtBackUp.sFixStat.bMark,sGPSData.sFixStat.bMark,4);
	memcpy(sGPSDtBackUp.sAtitude.bMark,sGPSData.sAtitude.bMark,10);
	memcpy(sGPSDtBackUp.sTemperSta.bMark,sGPSData.sTemperSta.bMark,7);
	memcpy(sGPSDtBackUp.sBalloonSta.bMark,sGPSData.sBalloonSta.bMark,6);
	sGPSDtBackUp.dwAtitude = sGPSData.dwAtitude;
	sGPSDtBackUp.bEndChar = sGPSData.bEndChar;

}

void GPSDtNumToChar(INT32U Data,INT8U *SavePosition)
{
	SavePosition[0] = ASCIIConvert(Data/100000,NUM_TO_CHAR);
	Data = Data%100000;

	SavePosition[1] = ASCIIConvert(Data/10000,NUM_TO_CHAR);
	Data = Data%10000;

	SavePosition[2] = ASCIIConvert(Data/1000,NUM_TO_CHAR);
	Data = Data%1000;

	SavePosition[3] = ASCIIConvert(Data/100,NUM_TO_CHAR);
	Data = Data%100;

	SavePosition[4] = ASCIIConvert(Data/10,NUM_TO_CHAR);
	Data = Data%10;
	
	SavePosition[5] = ASCIIConvert(Data,NUM_TO_CHAR);
}

static INT8U GPSNumCharConv(INT8U symbol,INT8U Type)
{
	if(Type == CHAR_TO_NUM)
	{
		if((symbol>=0x30)&&(symbol<=0x39))
		{
			return (symbol-0x30);
		}
		else if(Type == '-')
		{
			AtiSymbol = 1;
			return 0;
		}
	}
	else if(Type == NUM_TO_CHAR)
	{
		if((symbol>=0)&&(symbol<=9))
		{
			return (symbol+0x30);
		}
	}
	
	return SPACE;
}

void GPSDtParse(INT8U Type,INT8U* Adrr,INT8U DtLen)
{
	INT8U m;

	if((DtLen <= 0)||(DtLen>GPS_BUF_LEN))  //if DtLen>GPS_BUF_LEN,it will cause data cover problem
	{
		return;
	}
	
    memset(sGPSData.tbDataBuf,0,GPS_BUF_LEN);
	memcpy(sGPSData.tbDataBuf,Adrr,DtLen);
	
	switch(Type)
	{
		case LATITUDE:
		{
			sGPSData.dwLatiMinut = 0;
			for(m=0;m<DtLen;m++)
			{
				switch(m)
				{
					case 0:sGPSData.sLatitude.bGrade[m] = sGPSData.tbDataBuf[m];break;
					case 1:sGPSData.sLatitude.bGrade[m] = sGPSData.tbDataBuf[m];break;
					case 2:sGPSData.dwLatiMinut += ASCIIConvert(sGPSData.tbDataBuf[m],CHAR_TO_NUM)*100000;break;
					case 3:sGPSData.dwLatiMinut += ASCIIConvert(sGPSData.tbDataBuf[m],CHAR_TO_NUM)*10000;break;
					case 5:sGPSData.dwLatiMinut += ASCIIConvert(sGPSData.tbDataBuf[m],CHAR_TO_NUM)*1000;break;
					case 6:sGPSData.dwLatiMinut += ASCIIConvert(sGPSData.tbDataBuf[m],CHAR_TO_NUM)*100;break;
					case 7:sGPSData.dwLatiMinut += ASCIIConvert(sGPSData.tbDataBuf[m],CHAR_TO_NUM)*10;break;
					case 8:
					{
						sGPSData.dwLatiMinut += ASCIIConvert(sGPSData.tbDataBuf[m],CHAR_TO_NUM)*1;
						sGPSData.dwLatiMinut = sGPSData.dwLatiMinut*DATAx100/60;                    //1 degree = 60 minute
						GPSDtNumToChar(sGPSData.dwLatiMinut,sGPSData.sLatitude.dwMinute);
						break;
					}
				}
			}
			break;
		}
		case LATI_DIR:sGPSData.sLatitude.bMark[0] = sGPSData.tbDataBuf[0];break;
		case LONGTITU:
		{
			sGPSData.dwLontiMinut = 0;
			for(m=0;m<DtLen;m++)
			{
				switch(m)
				{
					case 0:sGPSData.sLongitude.bGrade[m] = sGPSData.tbDataBuf[m];break;
					case 1:sGPSData.sLongitude.bGrade[m] = sGPSData.tbDataBuf[m];break;
					case 2:sGPSData.sLongitude.bGrade[m] = sGPSData.tbDataBuf[m];break;
					case 3:sGPSData.dwLontiMinut += ASCIIConvert(sGPSData.tbDataBuf[m],CHAR_TO_NUM)*100000;break;
					case 4:sGPSData.dwLontiMinut += ASCIIConvert(sGPSData.tbDataBuf[m],CHAR_TO_NUM)*10000;break;
					case 6:sGPSData.dwLontiMinut += ASCIIConvert(sGPSData.tbDataBuf[m],CHAR_TO_NUM)*1000;break;
					case 7:sGPSData.dwLontiMinut += ASCIIConvert(sGPSData.tbDataBuf[m],CHAR_TO_NUM)*100;break;
					case 8:sGPSData.dwLontiMinut += ASCIIConvert(sGPSData.tbDataBuf[m],CHAR_TO_NUM)*10;break;
					case 9:
					{
						sGPSData.dwLontiMinut += ASCIIConvert(sGPSData.tbDataBuf[m],CHAR_TO_NUM)*1;
						sGPSData.dwLontiMinut = sGPSData.dwLontiMinut*DATAx100/60;                  //1 degree = 60 minute
						GPSDtNumToChar(sGPSData.dwLontiMinut,sGPSData.sLongitude.dwMinute);
						break;
					}
				}
			}
			break;
		}
		case LONG_DIR:sGPSData.sLongitude.bMark[0] = sGPSData.tbDataBuf[0];break;
		case FIX_STAT:sGPSData.sFixStat.bFixStatus = sGPSData.tbDataBuf[0];break;
		case PRECISION:
		{
			if(DtLen<=GPS_PRECI_LEN)
			{
				memcpy(sGPSData.sPrecision.tbPrecision,sGPSData.tbDataBuf,DtLen);
			}
			else
			{
				memcpy(sGPSData.sPrecision.tbPrecision,sGPSData.tbDataBuf,GPS_PRECI_LEN);
			}
			break;
		}
		case ATITUDE:
		{
			memset(sGPSData.sAtitude.tbAtitu,'0',GPS_ATITU_LEN);
			
			if(DtLen>GPS_ATITU_LEN)
			{
				break;
			}
	
			sGPSData.dwAtitude = 0;
			
			for(m=0;m<DtLen;m++)
			{
                sGPSData.sAtitude.tbAtitu[GPS_ATITU_LEN-m-1] = sGPSData.tbDataBuf[DtLen-m-1];
			}
			sGPSData.dwAtitude+= GPSNumCharConv(sGPSData.sAtitude.tbAtitu[0],CHAR_TO_NUM)*10000;
			sGPSData.dwAtitude+= GPSNumCharConv(sGPSData.sAtitude.tbAtitu[1],CHAR_TO_NUM)*1000;
            sGPSData.dwAtitude+= GPSNumCharConv(sGPSData.sAtitude.tbAtitu[2],CHAR_TO_NUM)*100;
			sGPSData.dwAtitude+= GPSNumCharConv(sGPSData.sAtitude.tbAtitu[3],CHAR_TO_NUM)*10;
			sGPSData.dwAtitude+= GPSNumCharConv(sGPSData.sAtitude.tbAtitu[4],CHAR_TO_NUM);
			if(AtiSymbol == 1)
			{
				AtiSymbol = 0;
				sGPSData.dwAtitude = -sGPSData.dwAtitude;
			}
			
			break;
		}
	}
}
INT8U GetGPSData()
{
	INT8U i;
	INT8U *pStringAddr;
	INT8U *PStrAddrLast;
	INT8U bMarkCnt=0;
	INT8U bDataLen=0;
	static xdata INT32S AtitudTab[3];
	
	if(MC20SendCmd("AT+QGNSSRD=\"NMEA/GGA\"\r\n","OK\r\n",USUAL_WAIT) == SUCCESS)
	{
		if (strstr(BackUpRxBuf, ",") == NULL)
		{
			return FAIL;
		}
		else
		{
			pStringAddr = BackUpRxBuf;
			PStrAddrLast = pStringAddr;
			
			for(i=0;i<INTER_RX_MAX_SIZE;i++)
			{
				if(BackUpRxBuf[i] == ',')
				{
					PStrAddrLast = pStringAddr;
					pStringAddr = &BackUpRxBuf[i]+1;
					bDataLen = (INT8U)(pStringAddr-PStrAddrLast-1);
					
					bMarkCnt++;
					if(bMarkCnt>ATITUDE)
					{
						break;
					}
					
					GPSDtParse(bMarkCnt,PStrAddrLast,bDataLen);
				}
			}

			if(sGPSData.sFixStat.bFixStatus == '0')    //if GPS data is invalid
			{
				return DT_INVALID;
		    }
			else   
			{
				AtitudTab[0] = AtitudTab[1];
				AtitudTab[1] = AtitudTab[2];
				AtitudTab[2] = sGPSData.dwAtitude;

				if((AtitudTab[0]>=AtitudTab[1]) && (AtitudTab[0]>=AtitudTab[2]))
				{
					if(AtitudTab[1] > AtitudTab[2])
					{
						sGPSData.dwAtitude = AtitudTab[1];  //select middle one
					}
					else
					{
						sGPSData.dwAtitude = AtitudTab[2];
					}
				}
				else if((AtitudTab[1]>=AtitudTab[0]) && (AtitudTab[1]>=AtitudTab[2]))
				{
					if(AtitudTab[0] > AtitudTab[2])
					{
						sGPSData.dwAtitude = AtitudTab[0];  //select middle one
					}
					else
					{
						sGPSData.dwAtitude = AtitudTab[2];
					}
				}
		        else if((AtitudTab[2]>=AtitudTab[0]) && (AtitudTab[2]>=AtitudTab[1]))
				{
					if(AtitudTab[0] > AtitudTab[1])
					{
						sGPSData.dwAtitude = AtitudTab[0];  //select middle one
					}
					else
					{
						sGPSData.dwAtitude = AtitudTab[1];
					}
				}
				
				return SUCCESS;
			}
		}
	}
	
	return FAIL;
}

void MC20Init()
{
	if(FaultChk(MC20_FAITAL_FAULT) == NO_FAULT)  //if GSM is started ok
	{
		MC20InitBaud();
		
		BasicInfoChk();

		GPSDtInit();
		
		MC20StartGNSS();
		
		MC20InitMessage();

		//ChkIfMsgIsFull();
	}
}
	
INT8U* GetPhoneInfo(INT8U *PhoneNum)
{
	INT8U i;
	
	for(i=0;i<PHONE_SIZE;i++)
	{
		tbMsgPhone[i+9] = PhoneNum[i];
	}
	
	return tbMsgPhone;
}

void MsgCmdSparse()
{
	INT8U bTemp=0;
	INT16U MotorTimTemp=0;

	if(strstr(sMsgManage.tbCommand,&CmdTable[0]) != NULL)
	{
		bTemp = GetGPSData();
		if(bTemp == SUCCESS)
		{
			sMsgManage.pMsgContent = sGPSData.sLatitude.bMark;
		}
		else if(bTemp == DT_INVALID)
		{
			sMsgManage.pMsgContent = &SendTable[1];  //
		}
		else
		{
			sMsgManage.pMsgContent = &SendTable[2];
		}
	}
	else if(strstr(sMsgManage.tbCommand,&CmdTable[1]) != NULL)  //get backup gps data
	{
		sMsgManage.pMsgContent = sGPSDtBackUp.sLatitude.bMark;
	}
	else if(strstr(sMsgManage.tbCommand,&CmdTable[2]) != NULL)  //start motor
	{
		if(sMsgManage.tbCommand[5] == ' ')
		{
			if(((ASCIIConvert(sMsgManage.tbCommand[9],CHAR_TO_NUM)) != SPACE) && ((ASCIIConvert(sMsgManage.tbCommand[8],CHAR_TO_NUM)) != SPACE)) //if only [9] is num,maybe it is wrong condition,so should check [8] and [9]
			{
				tbNumChk[0] = ASCIIConvert(sMsgManage.tbCommand[6],CHAR_TO_NUM);
				tbNumChk[1] = ASCIIConvert(sMsgManage.tbCommand[7],CHAR_TO_NUM);
				tbNumChk[2] = ASCIIConvert(sMsgManage.tbCommand[8],CHAR_TO_NUM);
				tbNumChk[3] = ASCIIConvert(sMsgManage.tbCommand[9],CHAR_TO_NUM);				
			}
			else if((ASCIIConvert(sMsgManage.tbCommand[8],CHAR_TO_NUM) != SPACE) && (ASCIIConvert(sMsgManage.tbCommand[7],CHAR_TO_NUM) != SPACE))
			{
				tbNumChk[0] = 0;
				tbNumChk[1] = ASCIIConvert(sMsgManage.tbCommand[6],CHAR_TO_NUM);
				tbNumChk[2] = ASCIIConvert(sMsgManage.tbCommand[7],CHAR_TO_NUM);
				tbNumChk[3] = ASCIIConvert(sMsgManage.tbCommand[8],CHAR_TO_NUM);
				
			}
			else if((ASCIIConvert(sMsgManage.tbCommand[7],CHAR_TO_NUM) != SPACE) && (ASCIIConvert(sMsgManage.tbCommand[6],CHAR_TO_NUM) != SPACE))
			{
				tbNumChk[0] = 0;
				tbNumChk[1] = 0;
				tbNumChk[2] = ASCIIConvert(sMsgManage.tbCommand[6],CHAR_TO_NUM);
				tbNumChk[3] = ASCIIConvert(sMsgManage.tbCommand[7],CHAR_TO_NUM);
			}
			else if(ASCIIConvert(sMsgManage.tbCommand[6],CHAR_TO_NUM) != SPACE)
			{
				tbNumChk[0] = 0;
				tbNumChk[1] = 0;
				tbNumChk[2] = 0;
				tbNumChk[3] = ASCIIConvert(sMsgManage.tbCommand[6],CHAR_TO_NUM);
			}
			else
			{
				tbNumChk[0] = 0;
				tbNumChk[1] = 0;
				tbNumChk[2] = 0;
				tbNumChk[3] = 0;
			}

            StartMotorTime = tbNumChk[0]*1000;
			StartMotorTime += tbNumChk[1]*100;
			StartMotorTime += tbNumChk[2]*10;
			StartMotorTime += tbNumChk[3];

			if(StartMotorTime<=600)
			{
				memcpy(tbSendMsgBuff,SendTable[3],MSG_BUFF_LEN);
				tbSendMsgBuff[9] = ' ';
				MotorTimTemp = StartMotorTime;
				tbSendMsgBuff[10] = ASCIIConvert((MotorTimTemp/100),NUM_TO_CHAR);
				MotorTimTemp = MotorTimTemp%100;
				tbSendMsgBuff[11] = ASCIIConvert((MotorTimTemp/10),NUM_TO_CHAR);
				MotorTimTemp = MotorTimTemp%10;
				tbSendMsgBuff[12] = ASCIIConvert(MotorTimTemp,NUM_TO_CHAR);
				tbSendMsgBuff[13] = 's';
				tbSendMsgBuff[14] = '\0';

				NeedStarMotor = 1;
				sMsgManage.pMsgContent = tbSendMsgBuff;
			}
			else
			{
				sMsgManage.pMsgContent = &SendTable[4];
				StartMotorTime = 0;
				NeedStarMotor = 0;
				GlabalTimeTemp = 0;
			}
		}
		else
		{
			sMsgManage.pMsgContent = &SendTable[4];
			StartMotorTime = 0;
			NeedStarMotor = 0;
			GlabalTimeTemp = 0;
		}
	}
	else
	{
		sMsgManage.pMsgContent = &SendTable[0];
	}
	
	sMsgManage.tbPhoneNum = sMsgManage.tbPhoneBuff;
	sMsgManage.DealStatus = NEED_SEND;
}
void BalloonStaChk()
{
    static INT8U UpCnt=0;
	static INT8U DownCnt=0;
	static INT8U UpRecordCnt=0;
	static INT32S AtitudeOld=0;

	if(GetGPSData() == SUCCESS)
	{
		if(sGPSData.dwAtitude > (AtitudeOld + COMP_HIGH))
		{
			UpCnt++;
			if(UpCnt>=CNT_TIME)
			{
				UpCnt = CNT_TIME;
				sGPSData.sBalloonSta.tbBloStat[0] = 'U';
				sGPSData.sBalloonSta.tbBloStat[1] = 'p';
				sGPSData.sBalloonSta.tbBloStat[2] = ' ';
				sGPSData.sBalloonSta.tbBloStat[3] = ' ';
				BalonStaRecor = STA_UP;
				
				if(UpRecordCnt<UP_CNT)  //this can indicate that balloon was ever in up condition 
				{
					UpRecordCnt++;
				}
				else
				{
					BalonAlreadyUp = 1;
				}
			}
			DownCnt = 0;
		}
		else if(AtitudeOld > (sGPSData.dwAtitude + COMP_HIGH))
		{
			DownCnt++;
			if(DownCnt>=CNT_TIME)
			{
				DownCnt = CNT_TIME;
				sGPSData.sBalloonSta.tbBloStat[0] = 'D';
				sGPSData.sBalloonSta.tbBloStat[1] = 'o';
				sGPSData.sBalloonSta.tbBloStat[2] = 'w';
				sGPSData.sBalloonSta.tbBloStat[3] = 'n';
				BalonStaRecor = STA_DOWN;
			}
			UpCnt = 0;
		}
		else 
		{
			sGPSData.sBalloonSta.tbBloStat[0] = 'P';
			sGPSData.sBalloonSta.tbBloStat[1] = 'a';
			sGPSData.sBalloonSta.tbBloStat[2] = 'r';
			sGPSData.sBalloonSta.tbBloStat[3] = 'a';
			BalonStaRecor = STA_PARA;
			UpCnt = 0;
			DownCnt = 0;
		}

		AtitudeOld = sGPSData.dwAtitude;
	}
	else 
	{
		sGPSData.sBalloonSta.tbBloStat[0] = '?';
		sGPSData.sBalloonSta.tbBloStat[1] = '?';
		sGPSData.sBalloonSta.tbBloStat[2] = '?';
		sGPSData.sBalloonSta.tbBloStat[3] = '?';
		BalonStaRecor = STA_UNKNOW;
		UpCnt = 0;
		DownCnt = 0;
		AtitudeOld = sGPSDtBackUp.dwAtitude;
	}
}

void BalonGoBackChk()
{
	static INT8U MsgSendRecor=0;
	static INT8U StaticCnt=0;
	static INT8U BackChkCnt=0;
	static bit Flag=0;
    
	if(BackChkCnt<20)  //4s
	{
		BackChkCnt++;
		return;
	}
	else
	{
		BackChkCnt = 0;
	}
	
	if((sGPSDtBackUp.dwAtitude<HIGH_LEVER_3)&&(sGPSDtBackUp.dwAtitude>(HIGH_LEVER_2+BACK_CMP_HIGH)))  //when balloon is under HIGH_LEVER_3,send a msg
	{
		if((MsgSendRecor&UNDER_LEVER_3) != UNDER_LEVER_3)
		{
			MsgSendRecor |= UNDER_LEVER_3;
			
			MsgSendRecor &= ~UNDER_LEVER_1;
			MsgSendRecor &= ~UNDER_LEVER_2;
			MsgSendRecor &= ~UNDER_LEVER_4;
			MsgSendRecor |= SEND_MSG_FLAG;
		}
	}
    else if((sGPSDtBackUp.dwAtitude<HIGH_LEVER_2)&&(sGPSDtBackUp.dwAtitude>(HIGH_LEVER_1+BACK_CMP_HIGH)))  //when balloon is under HIGH_LEVER_2,send a msg
	{
		if((MsgSendRecor&UNDER_LEVER_2) != UNDER_LEVER_2)
		{
			MsgSendRecor |= UNDER_LEVER_2;

			MsgSendRecor &= ~UNDER_LEVER_1;
			MsgSendRecor &= ~UNDER_LEVER_3;
			MsgSendRecor &= ~UNDER_LEVER_4;
			MsgSendRecor |= SEND_MSG_FLAG;
		}
	}
	else if(sGPSDtBackUp.dwAtitude<HIGH_LEVER_1)  //when balloon is under HIGH_LEVER_1,send a msg
	{
		if((MsgSendRecor&UNDER_LEVER_1) != UNDER_LEVER_1)
		{
			MsgSendRecor |= UNDER_LEVER_1;

			MsgSendRecor &= ~UNDER_LEVER_2;
			MsgSendRecor &= ~UNDER_LEVER_3;
			MsgSendRecor &= ~UNDER_LEVER_4;
			MsgSendRecor |= SEND_MSG_FLAG;
		}
	}
	else if((sGPSDtBackUp.dwAtitude<HIGH_LEVER_5) && (BalonAlreadyUp == 1) && (BalonStaRecor != STA_UP))
	{
		if(Flag == 0)
		{
			Flag = 1;
			MsgSendRecor |= SEND_MSG_FLAG;
		}
	}

	if((sGPSDtBackUp.dwAtitude<HIGH_LEVER_4) && (BalonStaRecor == STA_PARA) && ((MsgSendRecor&UNDER_LEVER_4) != UNDER_LEVER_4) && (BalonAlreadyUp == 1))  // if balloon is under HIGH_LEVER_4 and stay parallel for 2min,consider it is arived ground,then,should send a msg
	{
		StaticCnt++;
		if(StaticCnt>225)    //900s
		{
			StaticCnt = 0;
			MsgSendRecor |= UNDER_LEVER_4;
			MsgSendRecor |= SEND_MSG_FLAG;
		}
	}
	else
	{
		StaticCnt = 0;
	}

	if(sGPSDtBackUp.dwAtitude>(HIGH_LEVER_4+BACK_CMP_HIGH))
    {
		MsgSendRecor = 0;
	}
	
	if((MsgSendRecor&SEND_MSG_FLAG) == SEND_MSG_FLAG)
    {
		MsgSendRecor &= ~SEND_MSG_FLAG;
		
		if(BalonStaRecor != STA_UP)
		{
			if((BalonStaRecor == STA_DOWN)||(BalonStaRecor == STA_PARA))  //when power on,MC20 need time to locate,so,the first state will be STA_UNKNOW
			{
				sMsgManage.pMsgContent = sGPSData.sLatitude.bMark;
				sMsgManage.tbPhoneNum = sDeviceInfoEE.tbPhoneNum;
				sMsgManage.DealStatus = NEED_SEND;
			}
			else if((BalonStaRecor == STA_UNKNOW) && (BalonAlreadyUp == 1)) //when GPS without signal,UpRecordCnt can prevent send message when power on
			{
				sMsgManage.pMsgContent = sGPSDtBackUp.sLatitude.bMark;
				sMsgManage.tbPhoneNum = sDeviceInfoEE.tbPhoneNum;
				sMsgManage.DealStatus = NEED_SEND;
			}
		}
	}
}
void MC20PowerCtr(INT8U Cmd)
{
	if(Cmd == START)
	{
		Set_IO(MC_PW_CONTR_PORT,MC_PW_CONTR_PIN);
	}
	else if(Cmd == SHUT_DOWN)
	{
		Reset_IO(MC_PW_CONTR_PORT,MC_PW_CONTR_PIN);
    }
}
void MC20Task(INT8U TaskNum)
{
	static INT8U CycleCnt=0;
	
	TaskStaChk(TaskNum);

	if(ReadMessage()==RECIEV_MSG)
	{
		MsgCmdSparse();
	}
	else
	{
        CycleCnt++;

        if(CycleCnt>=LOCAT_CNT)
		{
			CycleCnt = 0;
			BalloonStaChk();
		}
		else if((CycleCnt%GET_DT_TIME) == 0)
		{
			if(GetGPSData() != SUCCESS)
			{
				CycleCnt = 0;
				sGPSData.sBalloonSta.tbBloStat[0] = '?';
				sGPSData.sBalloonSta.tbBloStat[1] = '?';
				sGPSData.sBalloonSta.tbBloStat[2] = '?';
				sGPSData.sBalloonSta.tbBloStat[3] = '?';
				BalonStaRecor = STA_UNKNOW;
			}
			else 
			{
				if(CycleCnt > CONTINOUS_TIME)  //if get data successfull continuously for 5 times
				{
					memcpy(sGPSDtBackUp.sLatitude.bMark,sGPSData.sLatitude.bMark,12);  //if data is valid for 5 times,it indicate that data is credible,so,back up 
					memcpy(sGPSDtBackUp.sLongitude.bMark,sGPSData.sLongitude.bMark,13);
					memcpy(sGPSDtBackUp.sFixStat.bMark,sGPSData.sFixStat.bMark,4);
					memcpy(sGPSDtBackUp.sAtitude.bMark,sGPSData.sAtitude.bMark,10);
					memcpy(sGPSDtBackUp.sBalloonSta.bMark,sGPSData.sBalloonSta.bMark,6);
					sGPSDtBackUp.dwAtitude = sGPSData.dwAtitude;
				}
			}
		}

		 BalonGoBackChk();  //every 4s
	}
    
	if(sMsgManage.DealStatus == NEED_SEND)  
    {
		SendMessage(sMsgManage.pMsgContent,GetPhoneInfo(sMsgManage.tbPhoneNum));
		sMsgManage.DealStatus = SEND_FINISH;
	}
	
	ChkIfMsgIsFull(); //30s
	
	TaskPend(TaskNum);
}












