#ifndef EEPROM_H
#define EEPROM_H

#define DEVICE_BLOCK        0x01
#define USER_BLOCK          0x02

#define DEVICE_INFO_SIZE    45  //take care of tbComUseBuf[60],can not less than DEVICE_INFO_SIZE
#define USER_INFO_SEZE      10

typedef struct 
		{
			INT8U  ComAddr;                    //1
			INT16U DeviceID;                   //3
			
			INT32U TimerGrp1;                  //7
			INT16U TimerGrp1_Time;             //9
			INT32U TimerGrp2;                  //13
			INT16U TimerGrp2_Time;             //15
			INT32U TimerGrp3;                  //19
			INT16U TimerGrp3_Time;             //21

			INT16U HighGrp1;                   //23
			INT16U HighGrp1_Time;              //25
			INT16U HighGrp2;                   //27
			INT16U HighGrp2_Time;              //29
			INT16U HighGrp3;                   //31
			INT16U HighGrp3_Time;              //33
			INT8U tbPhoneNum[11];              //44
			
			struct
			{
				INT8U TimerGrp1En:1;
				INT8U TimerGrp2En:1;
				INT8U TimerGrp3En:1;
				INT8U HighGrp1En:1;
				INT8U HighGrp2En:1;
				INT8U HighGrp3En:1;
				INT8U Reserved:2;
			} sDevInfBit;                      //45
		} sDeviceInf;
			
typedef struct 
		{
			INT32S MostAtitu;
			INT32U GlobalTimeRecor;
			INT16U wFaultRecord;
		} sUserInf;

extern xdata sDeviceInf sDeviceInfoEE;
extern xdata sUserInf   sUserInfoEE;

extern INT8U EprNeedWrt;

extern void PowerOnReadEE();
extern void ChkIfNeedWrtEpr();
extern void EEprDtInit();

#endif































