#include"Stc12c5a.h"
#include"Global.h"
#include"EEProm.h"
#include"intrins.h"
#include"MC20.h"

#define IAP_CMD_STANDBY  0x00
#define IAP_CMD_READ     0x01
#define IAP_CMD_PROGRAM  0x02
#define IAP_CMD_ERASE    0x03

#define IAP_FORBID       0x00
#define IAP_ENABLE       0x82  //sysclk<20M
//#define IAP_ENABLE       0x83  //sysclk<12
//#define IAP_ENABLE       0x84  //sysclk<6

#define IAP_SECTION1_ADDR   0x0000
#define IAP_SECTION2_ADDR   0x0200
#define IAP_INVALID_ADDR    0x8000  //this address IAP fuc is invalid,so it can prevent misuse

#define ERASE_FAIL          0x00
#define ERASE_OK            0x01
#define IAP_DELAY           200

#define DEVICE_BLOCK        0x01
#define USER_BLOCK          0x02
#define DEVCE_BL_STRADDR    IAP_SECTION1_ADDR
#define USER_BL_STRADDR     IAP_SECTION2_ADDR

#define EEPROM_OK           0x00
#define BL1_READ_FL         0x01
#define BL1_WRIT_FL         0x02
#define BL2_READ_FL         0x04
#define BL2_WRIT_FL         0x08
#define BL1_ERASE_FL        0x10
#define BL2_ERASE_FL        0x20

#define MAX_BLOCK_SIZE      512

INT8U EprNeedWrt=0;

xdata sDeviceInf sDeviceInfoEE;
xdata sUserInf   sUserInfoEE;

void IAP_Exit()
{
	IAP_CONTR = IAP_FORBID;
	IAP_CMD = IAP_CMD_STANDBY;
	IAP_TRIG = 0x00;
	IAP_ADDRH = (IAP_INVALID_ADDR>>8);  
	IAP_ADDRL = 0x00;
}

INT8U IAP_ReadByte(INT16U Addr)
{
	INT8U DataTemp;

	IAP_CONTR = IAP_ENABLE;
	IAP_CMD = IAP_CMD_READ;
	IAP_ADDRL = Addr;
	IAP_ADDRH = Addr>>8;
	IAP_TRIG = 0x5a;
	IAP_TRIG = 0xa5;
	_nop_();
	DataTemp = IAP_DATA;
	IAP_Exit();
	return DataTemp;
}

void IAP_ProgramByte(INT16U Addr,INT8U Dat)
{
	IAP_CONTR = IAP_ENABLE;
	IAP_CMD = IAP_CMD_PROGRAM;
	IAP_ADDRL = Addr;
	IAP_ADDRH = Addr>>8;
	IAP_DATA = Dat;
	IAP_TRIG = 0x5a;
	IAP_TRIG = 0xa5;
	_nop_();
	IAP_Exit();
}

INT8U IAP_EraseSector(INT16U SectorAddr)  
{
	INT16U i;
	
	IAP_CONTR = IAP_ENABLE;
	IAP_CMD = IAP_CMD_ERASE;
	IAP_ADDRL = SectorAddr;
	IAP_ADDRH = SectorAddr>>8;
	IAP_TRIG = 0x5a;
	IAP_TRIG = 0xa5;
	_nop_();
	_nop_();
	_nop_();
	IAP_Exit();

	for(i=0;i<MAX_BLOCK_SIZE;i++)
	{
		if(IAP_ReadByte(SectorAddr+i) != 0xff)
		{
			return ERASE_FAIL;
		}

	}
	
	return ERASE_OK;
}

INT8U ReadEEPrData(INT8U Block)
{
	INT8U *PStruct;
	INT8U bResult=EEPROM_OK;
	INT16U i;
	INT16U wCrcChk=0;

    bResult=EEPROM_OK;
	
	if((Block&DEVICE_BLOCK) == DEVICE_BLOCK)
	{
		ClrFault(EEPROM_READ_FALI);
		
		PStruct = (INT8U *)&sDeviceInfoEE;
		for(i=0;i<DEVICE_INFO_SIZE+2;i++)
		{
			tbComUseBuf[i] = IAP_ReadByte(DEVCE_BL_STRADDR+i);
		}
		wCrcChk = tbComUseBuf[DEVICE_INFO_SIZE]*256 + tbComUseBuf[DEVICE_INFO_SIZE+1];
		if(usMBCRC16(tbComUseBuf,DEVICE_INFO_SIZE) != wCrcChk)
		{
			bResult |= BL1_READ_FL;
		}
		else
		{
			for(i=0;i<DEVICE_INFO_SIZE;i++)
			{
				*((INT8U*)(PStruct+i)) = tbComUseBuf[i];
			}
		}

		if((bResult & BL1_READ_FL) != 0)  // user block is not important data for the moment,so,do not care its data is correct or not
		{
			SetFault(EEPROM_READ_FALI);
		}
	}
	
	if((Block&USER_BLOCK) == USER_BLOCK)
	{
		PStruct = (INT8U *)&sUserInfoEE;
		for(i=0;i<USER_INFO_SEZE+2;i++)
		{
			tbComUseBuf[i] = IAP_ReadByte(USER_BL_STRADDR+i);
		}
		wCrcChk = tbComUseBuf[USER_INFO_SEZE]*256 + tbComUseBuf[USER_INFO_SEZE+1];
		if(usMBCRC16(tbComUseBuf,USER_INFO_SEZE) != wCrcChk)
		{
			bResult |= BL2_READ_FL;
		}
		else
		{
		    for(i=0;i<USER_INFO_SEZE;i++)
			{
				*((INT8U*)(PStruct+i)) = tbComUseBuf[i];
			}
		}
	}
	
	return bResult;
}

INT8U WriteEEPrData(INT8U Block)
{
	INT8U *PStruct;
	INT8U bTimes=0;
	INT8U bResult=EEPROM_OK;
	INT16U i;
	INT16U wCrcChk=0;

	bResult=EEPROM_OK;
	
	if((Block&DEVICE_BLOCK) == DEVICE_BLOCK)
	{
		ClrFault(EEPROM_WRITE_FAIL);
		
		PStruct = (INT8U *)&sDeviceInfoEE;
		for(i=0;i<DEVICE_INFO_SIZE;i++)
		{
			tbComUseBuf[i] = *((INT8U*)(PStruct+i));
		}
		wCrcChk = usMBCRC16(tbComUseBuf,DEVICE_INFO_SIZE);
		tbComUseBuf[DEVICE_INFO_SIZE] = (INT8U)(wCrcChk>>8);
		tbComUseBuf[DEVICE_INFO_SIZE+1] = (INT8U)wCrcChk;

		for(bTimes=0;bTimes<3;bTimes++)  //erase 3 times,if erase fail
		{
			if(IAP_EraseSector(IAP_SECTION1_ADDR) == ERASE_OK)
			{
				bResult &= ~BL1_ERASE_FL;
				break;
			}
			else
			{
				bResult |= BL1_ERASE_FL;
			}
		}

		if((bResult&BL1_ERASE_FL) == 0)
		{
			for(bTimes=0;bTimes<3;bTimes++)
			{
				for(i=0;i<DEVICE_INFO_SIZE+2;i++)
				{
					IAP_ProgramByte(DEVCE_BL_STRADDR+i,tbComUseBuf[i]);
				}
				
				for(i=0;i<DEVICE_INFO_SIZE+2;i++)  //check if write success
				{
					if(IAP_ReadByte(DEVCE_BL_STRADDR+i) != tbComUseBuf[i])
					{
						bResult |= BL1_WRIT_FL;
						break;
					}
					else
					{
						bResult &= ~BL1_WRIT_FL;
					}
				}
				if((bResult&BL1_WRIT_FL) == 0)
				{
					break;
				}
			}
		}

		if(((bResult&BL1_WRIT_FL) != 0) || ((bResult&BL1_ERASE_FL) != 0))  // user block is not important data for the moment,so,do not care its data is correct or not
		{
			SetFault(EEPROM_WRITE_FAIL);
		}
	}

	if((Block&USER_BLOCK) == USER_BLOCK)
	{
		PStruct = (INT8U *)&sUserInfoEE;
		for(i=0;i<USER_INFO_SEZE;i++)
		{
			tbComUseBuf[i] = *((INT8U*)(PStruct+i));
		}
		wCrcChk = usMBCRC16(tbComUseBuf,USER_INFO_SEZE);
		tbComUseBuf[USER_INFO_SEZE] = (INT8U)(wCrcChk>>8);
		tbComUseBuf[USER_INFO_SEZE+1] = (INT8U)wCrcChk;

		for(bTimes=0;bTimes<3;bTimes++)  //erase 3 times,if erase fail
		{
			if(IAP_EraseSector(IAP_SECTION2_ADDR) == ERASE_OK)
			{
				bResult &= ~BL2_ERASE_FL;
				break;
			}
			else
			{
				bResult |= BL2_ERASE_FL;
			}
		}

		if((bResult&BL2_ERASE_FL) == 0)
		{
			for(bTimes=0;bTimes<3;bTimes++)
			{
				for(i=0;i<USER_INFO_SEZE+2;i++)
				{
					IAP_ProgramByte(USER_BL_STRADDR+i,tbComUseBuf[i]);
				}
				
				for(i=0;i<USER_INFO_SEZE+2;i++)
				{
					if(IAP_ReadByte(USER_BL_STRADDR+i) != tbComUseBuf[i])
					{
						bResult |= BL2_WRIT_FL;
						break;
					}
					else
					{
						bResult &= ~BL2_WRIT_FL;
					}
				}
				if((bResult&BL2_WRIT_FL) == 0)
				{
					break;
				}
			}
		}
	}
	
	return bResult;
}

void EEprDtInit()
{
	sDeviceInfoEE.ComAddr = 0x01;
	sDeviceInfoEE.DeviceID = 0x007d;
	
    sDeviceInfoEE.TimerGrp1 = 90000;          //if read eeprom fail,do some thing to prevent malfunction
	sDeviceInfoEE.TimerGrp1_Time = 0;    //3 min
	sDeviceInfoEE.TimerGrp2 = 90000;
	sDeviceInfoEE.TimerGrp2_Time = 0;
	sDeviceInfoEE.TimerGrp3 = 90000;
	sDeviceInfoEE.TimerGrp3_Time = 0;
	
	sDeviceInfoEE.HighGrp1 = 50000;
	sDeviceInfoEE.HighGrp1_Time = 0;
	sDeviceInfoEE.HighGrp2 = 50000;
	sDeviceInfoEE.HighGrp2_Time = 0;
	sDeviceInfoEE.HighGrp3 = 50000;
	sDeviceInfoEE.HighGrp3_Time = 0;
	
	sDeviceInfoEE.sDevInfBit.TimerGrp1En = 0;
	sDeviceInfoEE.sDevInfBit.TimerGrp2En = 0;
    sDeviceInfoEE.sDevInfBit.TimerGrp3En = 0;
    sDeviceInfoEE.sDevInfBit.HighGrp1En = 0;
    sDeviceInfoEE.sDevInfBit.HighGrp2En = 0;
    sDeviceInfoEE.sDevInfBit.HighGrp3En = 0;
	sDeviceInfoEE.sDevInfBit.Reserved = 0;

	sDeviceInfoEE.tbPhoneNum[0] = '1';
	sDeviceInfoEE.tbPhoneNum[1] = '3';
	sDeviceInfoEE.tbPhoneNum[2] = '0';
	sDeviceInfoEE.tbPhoneNum[3] = '6';
	sDeviceInfoEE.tbPhoneNum[4] = '6';
	sDeviceInfoEE.tbPhoneNum[5] = '8';
	sDeviceInfoEE.tbPhoneNum[6] = '8';
	sDeviceInfoEE.tbPhoneNum[7] = '2';
	sDeviceInfoEE.tbPhoneNum[8] = '8';
	sDeviceInfoEE.tbPhoneNum[9] = '6';
	sDeviceInfoEE.tbPhoneNum[10] = '0';
	
	sUserInfoEE.GlobalTimeRecor = 0;
	sUserInfoEE.MostAtitu = 0;
	sUserInfoEE.wFaultRecord = 0;
}

void CheckIfNewMcu()
{
	INT8U i;
	INT8U DevicBlkCnt=0;
	INT8U UserBlkCnt=0;

    if(ReadEEPrData(DEVICE_BLOCK) != EEPROM_OK)
	{
		WriteEEPrData(DEVICE_BLOCK);
	}
	else  // in case new mcu eeprom data is all 0xff,but crc is check ok
	{
		for(i=0;i<DEVICE_INFO_SIZE;i++)
		{
			if(IAP_ReadByte(IAP_SECTION1_ADDR+i) == 0xff)
			{
				DevicBlkCnt++;
			}
			
			if(DevicBlkCnt>=DEVICE_INFO_SIZE)
			{
				WriteEEPrData(DEVICE_BLOCK);
			}
		}
	}
	
	if(ReadEEPrData(USER_BLOCK) != EEPROM_OK)
	{
		WriteEEPrData(USER_BLOCK);
	}
	else  // in case new mcu eeprom data is all 0xff,but crc is check ok
	{
		for(i=0;i<USER_INFO_SEZE;i++)
		{
			if(IAP_ReadByte(IAP_SECTION2_ADDR+i) == 0xff)
			{
				UserBlkCnt++;
			}
		}
		if(UserBlkCnt>=USER_INFO_SEZE)
		{
			WriteEEPrData(USER_BLOCK);
		}
	}
}

void PowerOnReadEE()
{
	EEprDtInit();
	CheckIfNewMcu();
}

void ChkIfNeedWrtEpr()
{
	static INT8U CycleCnt=0;

	if(CycleCnt<50)
	{
		CycleCnt++;
		return;
	}
	else
	{
		CycleCnt = 0;
	}
	
	if((EprNeedWrt & DEVICE_BLOCK) == DEVICE_BLOCK)
	{
		EprNeedWrt &= ~DEVICE_BLOCK;
		WriteEEPrData(DEVICE_BLOCK);
	}
	else if((EprNeedWrt & USER_BLOCK) == USER_BLOCK)
	{
		EprNeedWrt &= ~USER_BLOCK;
		WriteEEPrData(USER_BLOCK);
	}
}



















