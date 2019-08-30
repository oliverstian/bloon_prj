#include"Stc12c5a.h"
#include"Global.h"
#include"IO.h"

#define PxM0_SET     0x01
#define PxM1_SET     0x10

void Config_IO(INT8U IO,INT8U PIN,INT8U Mode)
{
	INT8U PxM0_Temp=0;
	INT8U PxM1_Temp=0;

	if((IO>MAX_IO_NUM)||(Mode>MAX_MODE_NUM))
	{
		return;
	}
	
	if((Mode&PxM0_SET) == PxM0_SET)
	{
		PxM0_Temp |= PIN;
	}
	
	if((Mode&PxM1_SET) == PxM1_SET)
	{
		PxM1_Temp |= PIN;
	}

	if((IO&IO_0) == IO_0)
	{
		P0M0 &= ~PIN;
		P0M0 |= PxM0_Temp;
		P0M1 &= ~PIN;
		P0M1 |= PxM1_Temp;
	}
    if((IO&IO_1) == IO_1)
    {
		P1M0 &= ~PIN;
		P1M0 |= PxM0_Temp;
		P1M1 &= ~PIN;
		P1M1 |= PxM1_Temp;
	}
	if((IO&IO_2) == IO_2)
    {
		P2M0 &= ~PIN;
		P2M0 |= PxM0_Temp;
		P2M1 &= ~PIN;
		P2M1 |= PxM1_Temp;
	}
	if((IO&IO_3) == IO_3)
	{
		P3M0 &= ~PIN;
		P3M0 |= PxM0_Temp;
		P3M1 &= ~PIN;
		P3M1 |= PxM1_Temp;
	}
    if((IO&IO_4) == IO_4)
    {
		P4M0 &= ~PIN;
		P4M0 |= PxM0_Temp;
		P4M1 &= ~PIN;
		P4M1 |= PxM1_Temp;
	}
	if((IO&IO_5) == IO_5)
	{
		P5M0 &= ~PIN;
		P5M0 |= PxM0_Temp;
		P5M1 &= ~PIN;
		P5M1 |= PxM1_Temp;
	}
}

void Set_IO(INT8U Port,INT8U Pin_Num)
{
	if(Port >MAX_IO_NUM)
	{
		return;
	}

	if((Port&IO_0) == IO_0)
	{
		P0 |= Pin_Num;
	}
	if((Port&IO_1) == IO_1)
	{
		P1 |= Pin_Num;
	}
	if((Port&IO_2) == IO_2)
	{
		P2 |= Pin_Num;
	}
	if((Port&IO_3) == IO_3)
	{
		P3 |= Pin_Num;
	}
	if((Port&IO_4) == IO_4)
	{
		P4 |= Pin_Num;
	}
	if((Port&IO_5) == IO_5)
	{
		P5 |= Pin_Num;
	}
}

void Reset_IO(INT8U Port,INT8U Pin_Num)
{
	if(Port >MAX_IO_NUM)
	{
		return;
	}
	
	if((Port&IO_0) == IO_0)
	{
		P0 &= ~Pin_Num;
	}
	if((Port&IO_1) == IO_1)
	{
		P1 &= ~Pin_Num;
	}
	if((Port&IO_2) == IO_2)
	{
		P2 &= ~Pin_Num;
	}
	if((Port&IO_3) == IO_3)
	{
		P3 &= ~Pin_Num;
	}
	if((Port&IO_4) == IO_4)
	{
		P4 &= ~Pin_Num;
	}
	if((Port&IO_5) == IO_5)
	{
		P5 &= ~Pin_Num;
	}
}

INT8U Read_IO(INT8U Port,INT8U Pin_Num)
{
	INT8U Temp=0;
	
	switch(Port)
	{
		case IO_0:Temp = P0&Pin_Num;break;
		case IO_1:Temp = P1&Pin_Num;break;
		case IO_2:Temp = P2&Pin_Num;break;
		case IO_3:Temp = P3&Pin_Num;break;
		case IO_4:Temp = P4&Pin_Num;break;
		case IO_5:Temp = P5&Pin_Num;break;
	}
	
    if(Temp == 0)
    {
		return RESET;
	}
	else
	{
		return SET;
	}
}

void IOInit()
{
	Config_IO(IO_1,PIN_0,OPEN_DRAIN_MODE);
	Set_IO(IO_1,PIN_0);
	
	Config_IO(POWE_KEY_PORT,POWE_KEY_PIN,PUSH_PULL_MODE);
	Reset_IO(POWE_KEY_PORT,POWE_KEY_PIN);

	Config_IO(MCU_LED_PORT,MCU_LED_PIN,PUSH_PULL_MODE);
	Reset_IO(MCU_LED_PORT,MCU_LED_PIN);

	Config_IO(POWE_EN_PORT,POWE_EN_PIN,PUSH_PULL_MODE);
	Reset_IO(POWE_EN_PORT,POWE_EN_PIN);

	Config_IO(MOTER_PORT,MOTER_PIN,PUSH_PULL_MODE);
	Reset_IO(MOTER_PORT,MOTER_PIN);

    P4SW |= 0x40;  //set P46 to be common IO port,this pin should be changed later
	Config_IO(MC_PW_CONTR_PORT,MC_PW_CONTR_PIN,PUSH_PULL_MODE);

	Config_IO(TEMPER_CTR_PORT,TEMPER_CTR_PIN,PUSH_PULL_MODE);
	Reset_IO(TEMPER_CTR_PORT,TEMPER_CTR_PIN);
}
































