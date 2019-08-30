#include"Stc12c5a.h"
#include"Global.h"
#include"intrins.h"
#include"IO.h"
#include"Timer.h"
#include"CommuTask.h"
#include"EEProm.h"
#include <string.h>
#include"MC20.h"
#include"DS18B20.h"

void TaskInit()
{
	CreatTask(USER_TASK,20);        //100ms
	
    CreatTask(EXTER_COM_TASK,40);   //200ms

	CreatTask(MC20_TASK,36);        //200ms  //compasate 4*5 = 20ms
}

void InitSys()
{
	InterruptControl(EA_I|TIMER0_I|TIMER1_I|UART1_I|UART2_I,Enable);
	
    InitTimer();
	
	UartInit();

	IOInit();

	MC20Start();
	
	PowerOnReadEE();
	
	TaskInit();

	MC20Init();

	DS_Init();

	GlabalTimeCnt = 0;
}

void main()
{
	InitSys();

	while(1)
	{
		UserTasks(USER_TASK);

        MC20Task(MC20_TASK);

		ExterComTask(EXTER_COM_TASK);
	}
}





















