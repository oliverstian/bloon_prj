#include"Stc12c5a.h"
#include"Global.h"
#include"IO.h"
#include"DS18B20.h"
#include"EEProm.h"
#include"MC20.h"

void AtituCtrMotor()
{
	static INT8U High1MotorFlg=0;
	static INT8U High2MotorFlg=0;
	static INT8U High3MotorFlg=0;
	static xdata INT32S Grp1HighOld=0;
	static xdata INT32S Grp2HighOld=0;
	static xdata INT32S Grp3HighOld=0;
	static xdata INT32U Grp1HTimeTemp=0;
	static xdata INT32U Grp2HTimeTemp=0;
	static xdata INT32U Grp3HTimeTemp=0;
	INT32S HighTemp=0;

	if(sDeviceInfoEE.sDevInfBit.HighGrp1En == Enable)
    {
		HighTemp = (INT32S)sDeviceInfoEE.HighGrp1;
		
        if(High1MotorFlg < 3)
        {
			if((sGPSDtBackUp.dwAtitude + MOTOR_PREPAER) >= HighTemp)
			{
				if(High1MotorFlg == 0)
				{
					High1MotorFlg = 1;
					Grp1HTimeTemp = GlabalTimeCnt;
				}
				else if(High1MotorFlg >= 1)
				{
					if(GlabalTimeCnt < (Grp1HTimeTemp + 10))
					{
						if(sGPSDtBackUp.dwAtitude!= Grp1HighOld)
						{
							High1MotorFlg++;
								
							if(High1MotorFlg > 2)
							{
								Grp1HTimeTemp = GlabalTimeCnt;
							}
						}
					}
					else
					{
						High1MotorFlg = 0;
					}
				}
			}
			else
			{
				High1MotorFlg = 0;
			}
        }

        if(High1MotorFlg == 3)
        {
			if(GlabalTimeCnt < (Grp1HTimeTemp + sDeviceInfoEE.HighGrp1_Time))
			{
				Set_IO(MOTER_PORT,MOTER_PIN);
			}
			else
			{
				High1MotorFlg = 4;
				Reset_IO(MOTER_PORT,MOTER_PIN);
			}
		}
		
		Grp1HighOld = sGPSDtBackUp.dwAtitude;
	}

	if(sDeviceInfoEE.sDevInfBit.HighGrp2En == Enable)
    {
		HighTemp = (INT32S)sDeviceInfoEE.HighGrp2;
		
        if(High2MotorFlg < 3)
        {
			if((sGPSDtBackUp.dwAtitude + MOTOR_PREPAER) >= HighTemp)
			{
				if(High2MotorFlg == 0)
				{
					High2MotorFlg = 1;
					Grp2HTimeTemp = GlabalTimeCnt;
				}
				else if(High2MotorFlg >= 1)
				{
					if(GlabalTimeCnt < (Grp2HTimeTemp + 10))
					{
						if(sGPSDtBackUp.dwAtitude!= Grp2HighOld)
						{
							High2MotorFlg++;
								
							if(High2MotorFlg > 2)
							{
								Grp2HTimeTemp = GlabalTimeCnt;
							}
						}
					}
					else
					{
						High2MotorFlg = 0;
					}
				}
			}
			else
			{
				High2MotorFlg = 0;
			}
        }

        if(High2MotorFlg == 3)
        {
			if(GlabalTimeCnt < (Grp2HTimeTemp + sDeviceInfoEE.HighGrp2_Time))
			{
				Set_IO(MOTER_PORT,MOTER_PIN);
			}
			else
			{
				High2MotorFlg = 4;
				Reset_IO(MOTER_PORT,MOTER_PIN);
			}
		}
		
		Grp2HighOld = sGPSDtBackUp.dwAtitude;
	}

	if(sDeviceInfoEE.sDevInfBit.HighGrp3En == Enable)
    {
		HighTemp = (INT32S)sDeviceInfoEE.HighGrp3;
		
        if(High3MotorFlg < 3)
        {
			if((sGPSDtBackUp.dwAtitude + MOTOR_PREPAER) >= HighTemp)
			{
				if(High3MotorFlg == 0)
				{
					High3MotorFlg = 1;
					Grp3HTimeTemp = GlabalTimeCnt;
				}
				else if(High3MotorFlg >= 1)
				{
					if(GlabalTimeCnt < (Grp3HTimeTemp + 10))
					{
						if(sGPSDtBackUp.dwAtitude!= Grp3HighOld)
						{
							High3MotorFlg++;
								
							if(High3MotorFlg > 2)
							{
								Grp3HTimeTemp = GlabalTimeCnt;
							}
						}
					}
					else
					{
						High3MotorFlg = 0;
					}
				}
			}
			else
			{
				High3MotorFlg = 0;
			}
        }

        if(High3MotorFlg == 3)
        {
			if(GlabalTimeCnt < (Grp3HTimeTemp + sDeviceInfoEE.HighGrp3_Time))
			{
				Set_IO(MOTER_PORT,MOTER_PIN);
			}
			else
			{
				High3MotorFlg = 4;
				Reset_IO(MOTER_PORT,MOTER_PIN);
			}
		}
		
		Grp3HighOld = sGPSDtBackUp.dwAtitude;
	}
}

void TimeCtrMotor()
{
	if(sDeviceInfoEE.sDevInfBit.TimerGrp1En == Enable)
	{
		if((GlabalTimeCnt>sDeviceInfoEE.TimerGrp1) && (GlabalTimeCnt<(sDeviceInfoEE.TimerGrp1_Time+sDeviceInfoEE.TimerGrp1)))
		{
			Set_IO(MOTER_PORT,MOTER_PIN);
		}
		else
		{
			Reset_IO(MOTER_PORT,MOTER_PIN);
		}
	}
	if(sDeviceInfoEE.sDevInfBit.TimerGrp2En == Enable)
	{
		if((GlabalTimeCnt>sDeviceInfoEE.TimerGrp2) && (GlabalTimeCnt<(sDeviceInfoEE.TimerGrp2_Time+sDeviceInfoEE.TimerGrp2)))
		{
			Set_IO(MOTER_PORT,MOTER_PIN);
		}
		else
		{
			Reset_IO(MOTER_PORT,MOTER_PIN);
		}
	}
	if(sDeviceInfoEE.sDevInfBit.TimerGrp3En == Enable)
	{
		if((GlabalTimeCnt>sDeviceInfoEE.TimerGrp3) && (GlabalTimeCnt<(sDeviceInfoEE.TimerGrp3_Time+sDeviceInfoEE.TimerGrp3)))
		{
			Set_IO(MOTER_PORT,MOTER_PIN);
		}
		else
		{
			Reset_IO(MOTER_PORT,MOTER_PIN);
		}
	}
}

void MoterContr()
{
	static bit MotorStarFlg = 0;
	static xdata INT16U FaultStrMotor = 0;
	
	AtituCtrMotor();

	TimeCtrMotor();
    
    if(FaultChk(EEPROM_READ_FALI) != NO_FAULT)  //if eeprom read fail,40s later start motor
    {
		FaultStrMotor++;

		if((FaultStrMotor > 300) && (FaultStrMotor < 900))
		{
			Set_IO(MOTER_PORT,MOTER_PIN);
		}
		else if(FaultStrMotor >= 900)
		{
			FaultStrMotor  = 900;
			Reset_IO(MOTER_PORT,MOTER_PIN);
		}
	}
	
	if(NeedStarMotor == 1)
	{
		MotorStarFlg = 1;
		NeedStarMotor = 0;
		GlabalTimeTemp = GlabalTimeCnt;
	}

	if(MotorStarFlg == 1)
	{
		if(GlabalTimeCnt < (GlabalTimeTemp + StartMotorTime))
		{
			Set_IO(MOTER_PORT,MOTER_PIN);
		}
		else
		{
			MotorStarFlg = 0;
			GlabalTimeTemp = 0;
			StartMotorTime = 0;
			Reset_IO(MOTER_PORT,MOTER_PIN);
		}
	}
}
void MCU_LED_Contr()
{
	static INT8U LedCnt=0;

	if(FaultChk(ANY_FLAULT) == NO_FAULT)
	{
		LedCnt++;
		if(LedCnt==10)
		{
	    	Reset_IO(MCU_LED_PORT,MCU_LED_PIN);
		}
		else if(LedCnt>=20)
		{
			LedCnt=0;
			Set_IO(MCU_LED_PORT,MCU_LED_PIN);
		}
	}
	else
	{
		LedCnt++;
		if(LedCnt==2)
		{
	    	Reset_IO(MCU_LED_PORT,MCU_LED_PIN);
		}
		else if(LedCnt>=4)
		{
			LedCnt=0;
			Set_IO(MCU_LED_PORT,MCU_LED_PIN);
		}
	}
}
void MostAtiJuge()
{
	static xdata INT8U AtiCompCnt=0;
	static xdata INT8U WrtUseBlkCnt=0;
	static xdata INT32S AtiRecorOld=0;
	static xdata INT32U GlobalTimeTem=0;
	static bit WriteUseBlk=0;
	static bit flag=0;

	if(sGPSDtBackUp.dwAtitude > sUserInfoEE.MostAtitu)
	{
		if(flag == 0)
		{
			flag = 1;
			AtiCompCnt = 0;
			GlobalTimeTem = GlabalTimeCnt;
		}
        else if(flag == 1)
		{
			if(GlabalTimeCnt < (GlobalTimeTem + 20))
			{
				if(AtiRecorOld != sGPSDtBackUp.dwAtitude)
				{
					AtiCompCnt++;
					if(AtiCompCnt > 3)
					{
						flag = 0;
						AtiCompCnt = 0;
						sUserInfoEE.MostAtitu = sGPSDtBackUp.dwAtitude;
						WrtUseBlkCnt = 0;
						WriteUseBlk = 1;
					}
				}
			}
			else
			{
				flag = 0;
				AtiCompCnt = 0;
			}
		}
	}
	else
	{
		flag = 0;
		AtiCompCnt = 0;
	}
	
	if(WriteUseBlk == 1)
	{
		WrtUseBlkCnt++;
		if(WrtUseBlkCnt > 90)  //9s
		{
			WriteUseBlk = 0;
			WrtUseBlkCnt = 0;
			EprNeedWrt |= USER_BLOCK;
		}
	}
	
	AtiRecorOld = sGPSDtBackUp.dwAtitude;
}
void UserBlWriteChk()
{
	static xdata INT16U TenMinCnt=0; 
	static xdata INT16U FaultOld = 0;

	MostAtiJuge();
	
	if(TenMinCnt >= 6000)  //>600s
	{
		TenMinCnt = 0;
		sUserInfoEE.GlobalTimeRecor = GlabalTimeCnt;
		EprNeedWrt |= USER_BLOCK;
	}
	else
	{
		TenMinCnt++;
	}

	if((wFaultRecord != FaultOld) && (wFaultRecord != 0))
	{
		sUserInfoEE.wFaultRecord |= wFaultRecord;
		FaultOld = wFaultRecord;
		EprNeedWrt |= USER_BLOCK;
	}
}
void UserTasks(INT8U TaskNum)
{
	TaskStaChk(TaskNum);

	TemperCtr();

	MoterContr();

	MCU_LED_Contr();

	UserBlWriteChk();
	
	ChkIfNeedWrtEpr();  //5s

	TaskPend(TaskNum);
}

































