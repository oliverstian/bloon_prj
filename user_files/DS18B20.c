#include"Stc12c5a.h"
#include"Global.h"
#include"IO.h"
#include"DS18B20.h"
#include <string.h>
#include"MC20.h"


#define DS18B20_PORT  IO_0
#define DS18B20_PIN   PIN_3 
#define DS18B20_DATA_PIN  P03


#define IGNOR_ROM     0xcc
#define STR_TMP_TRANS 0x44
#define READ_RAM      0xbe
#define WRT_RAM       0x4e

#define ACCURACY      5   //9 bit,accuracy is 0.5,amplify 10x

#define DEVECE_EXIST  0

INT16U wTemperCal=0;
INT8U TemperType = 0;

INT8U DS_Reset()
{
	INT8U bTemp=0;
	
	DS18B20_DATA_PIN = 0;                        //pull bus low 
	Delay_1p8us(330);                            //597us to reset
	DS18B20_DATA_PIN = 1;                        //release bus
	Delay_1p8us(42);                             //wait 78.6us to read bus 
	bTemp = Read_IO(DS18B20_PORT,DS18B20_PIN);   //if return 0 indicate device is exsist ,else not exsist
	Delay_1p8us(165);                            //delay 300us to wait bus recover
    DS18B20_DATA_PIN = 1;                        //release bus
    
	return bTemp;
}

void DS_WriteData(INT8U bData)
{
	INT8U i;

	for(i=0;i<8;i++)
	{
		DS18B20_DATA_PIN = 0;                //pull bus low to prepare to write
		Delay_1p8us(0);                      // 3.08us
		if((bData&0x01) == 1)
		{
			DS18B20_DATA_PIN = 1;
		}
		else
		{
			DS18B20_DATA_PIN = 0;
		}
		Delay_1p8us(37);                   //69.6us//delay at least 60us to complish writing
		DS18B20_DATA_PIN = 1;              //release bus 
		Delay_1p8us(0);                    //3.08us//continous writing need to delay some time(>1us)
		bData>>=1;
	}
}

INT8U DS_ReadData()
{
	INT8U i;
	INT8U bTemp=0;

	for(i=0;i<8;i++)
	{
		DS18B20_DATA_PIN = 0;    //pull bus low to prepare to read
		Delay_1p8us(0);          //3.08us// >1us
		DS18B20_DATA_PIN = 1;    //release bus
		Delay_1p8us(0);          //after 3.08us,read data
		if(Read_IO(DS18B20_PORT,DS18B20_PIN)==SET)
		{
			bTemp|=(0x01<<i);
		}
		Delay_1p8us(32);        //60.6us
		DS18B20_DATA_PIN = 1;   //release bus
		Delay_1p8us(0);         //3.08us//continous reading need to delay some time(>1us)
	}
	return bTemp;
}

void DS_Init()
{
	Config_IO(DS18B20_PORT,DS18B20_PIN,OPEN_DRAIN_MODE);
	DS18B20_DATA_PIN = 1;      //release bus
	delayN_Tick(20);
	DS_Reset();
	DS_WriteData(IGNOR_ROM);
	DS_WriteData(WRT_RAM);
	DS_WriteData(0x60);  //+96
	DS_WriteData(0xe0);  //-96
	DS_WriteData(0x1f);  //9bit  93.75ms
}

void GetTemperChar(INT8U TemperType)
{
	INT16U TemperTemp=0;

	TemperTemp = wTemperCal;

    if(TemperType == TEMPER_MINUS)
    {
		sGPSData.sTemperSta.tbTemper[0] = '-';
    }
	else if(TemperType == TEMPER_PLUS)
	{
		sGPSData.sTemperSta.tbTemper[0] = '+';
	}

	sGPSData.sTemperSta.tbTemper[1] = ASCIIConvert((TemperTemp/1000),NUM_TO_CHAR);
	TemperTemp = TemperTemp%1000;
	sGPSData.sTemperSta.tbTemper[2] = ASCIIConvert((TemperTemp/100),NUM_TO_CHAR);
	TemperTemp = TemperTemp%100;
	sGPSData.sTemperSta.tbTemper[3] = ASCIIConvert((TemperTemp/10),NUM_TO_CHAR);

	memcpy(sGPSDtBackUp.sTemperSta.bMark,sGPSData.sTemperSta.bMark,7);  //backup
}
void TemperCtr()
{
	INT8U LSB_Data=0;
	INT8U MSB_Data=0;
	static INT8U LowTemCnt1=0;
	static INT8U HighTemCnt1 = 0;
	static INT8U LowTemCnt2 = 0;
	static INT8U HighTemCnt2 = 0;
	static bit Flag=0;

    if(DS_Reset()==DEVECE_EXIST)  //if device is ok
    {
		DS_WriteData(IGNOR_ROM);
		if(Flag == 0)
		{
			Flag = 1;
			DS_WriteData(STR_TMP_TRANS); //since it needs about 100ms to convert,current tick start convert,next tick read termperature
		}
		else                             
		{
			Flag = 0;
			DS_WriteData(READ_RAM);
			LSB_Data = DS_ReadData();
			MSB_Data = DS_ReadData();

			wTemperCal = MSB_Data*256 + LSB_Data;
			if((MSB_Data&0xf8) != 0)             //MSB high 5 bits are 1, negtive temperature
			{
				wTemperCal = ((~wTemperCal)>>3)+1;
				TemperType = TEMPER_MINUS;
			}
			else
			{
				wTemperCal = wTemperCal>>3;
				TemperType = TEMPER_PLUS;
			}
			
			wTemperCal = wTemperCal*ACCURACY;   //accuracy is 0.5 degree,amplify x10

			if(TemperType == TEMPER_MINUS)
			{
				if(wTemperCal>150)  //if <-15
				{
					HighTemCnt2 = 0;
					LowTemCnt2++;
					if(LowTemCnt2>=10)  //2s
					{
						LowTemCnt2 = 0;
						Reset_IO(TEMPER_CTR_PORT,TEMPER_CTR_PIN);
					}
				}
				else if(wTemperCal <130)
				{
					LowTemCnt2 = 0;
					HighTemCnt2++;
					if(HighTemCnt2>=5)
					{
						HighTemCnt2 = 0;
						Set_IO(TEMPER_CTR_PORT,TEMPER_CTR_PIN);
					}
				}
				else
				{
					HighTemCnt2 = 0;
					LowTemCnt2 = 0;
				}
			}
			else if(TemperType == TEMPER_PLUS)
			{
				if(wTemperCal>50)
				{
					LowTemCnt1 = 0;
					HighTemCnt1++;
					if(HighTemCnt1 >5)
					{
						HighTemCnt1 = 0;
						Reset_IO(TEMPER_CTR_PORT,TEMPER_CTR_PIN);
					}
				}
				else if(wTemperCal<10)
				{
					HighTemCnt1 = 0;
					LowTemCnt1++;
					if(LowTemCnt1>5)
					{
						LowTemCnt1 = 0;
						Set_IO(TEMPER_CTR_PORT,TEMPER_CTR_PIN);
					}
				}
				else
				{
					HighTemCnt1 = 0;
					LowTemCnt1 = 0;
				}
			}

			GetTemperChar(TemperType);
		}
    }
}













