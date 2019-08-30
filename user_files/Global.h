#ifndef GLOBAL_DCLARE_H
#define GLOBAL_DCLARE_H

typedef signed char          INT8S;
typedef idata signed char    IINT8S;
typedef xdata signed char    XINT8S;

typedef unsigned char        INT8U;
typedef idata unsigned char  IINT8U;
typedef xdata unsigned char  XINT8U;

typedef signed int           INT16S;
typedef idata signed int     IINT16S;
typedef xdata signed int     XINT16S;

typedef unsigned int         INT16U;
typedef idata unsigned int   IINT16U;
typedef xdata unsigned int   XINT16U;

typedef signed long      INT32S;
typedef unsigned long    INT32U;
typedef float            FL32U;

//#define CRYSTAL_12M
#define CRYSTAL_11P0592M

#ifdef CRYSTAL_12M
#define FOSC     12000000L  //12M
#elif defined CRYSTAL_11P0592M
#define FOSC     11059200L  //11.0592M
#else
#define FOSC     12000000L  //12M
#endif

#define Enable  0x01
#define Disable 0x00
#define SET     1
#define RESET   0

//*****************************  Fault type*************************
#define NO_FAULT            0x0000
#define ANY_FLAULT          0xffff  //check if there is any fault
#define EEPROM_WRITE_FAIL   0x0001
#define EEPROM_READ_FALI    0x0002

#define MC20_FAITAL_FAULT   0x0008
#define MC20_MSG_FAULT      0x0010  //message fault,maybe initial maybe send
#define MC20_BAUD_FAULT     0x0020
#define MC20_BASE_FAULT     0x0040


//************************interrupt name****************************
#define EXT0_I      0x0001
#define TIMER0_I    0x0002
#define EXT1_I      0x0004
#define TIMER1_I    0x0008
#define UART1_I     0x0010
#define ADC_I       0x0020
#define LVD_I       0x0040
#define EA_I        0x0080  //main switch
#define PCA_I       0x0100
#define UART2_I     0x0200
#define SPI_I       0x0400

//***********************interrupt vector num***********************
#define EXT0_VEC      0
#define TIMER0_VEC    1
#define EXT1_VEC      2
#define TIMER1_VEC    3
#define UART1_VEC     4
#define ADC_VEC       5
#define LVD_VEC       6
#define PCA_VEC       7
#define UART2_VEC     8
#define SPI_VEC       9
//***************************Task relevant*******************************
#define USER_TASK        0
#define MC20_TASK        1
#define EXTER_COM_TASK   2

#define MAX_TASK_NUM     3

#define TASK_WAIT    0
#define TASK_READY   1

#define CHAR_TO_NUM  0
#define NUM_TO_CHAR  1
#define SPACE        0x20


//*********************************************************************
#define TaskStaChk(TaskNum) if(TaskStatCheck(TaskNum) == TASK_WAIT) return;

//*********************************************************************
typedef struct
{
	INT16U Period;
	INT16U TimeCnt;
	INT8U TaskStatus;
} TaskManage;

extern xdata TaskManage TaskManaTble[];

extern xdata INT8U tbComUseBuf[];

extern xdata INT32U GlabalTimeCnt;
extern xdata INT32U GlabalTimeTemp;
extern xdata INT16U StartMotorTime;
extern INT16U wFaultRecord;
extern bit NeedStarMotor;



//*********************************************************************
extern void InterruptControl(INT16U IntNum, bit Contr);
extern void UserTasks(INT8U TaskNum);
extern void CreatTask(INT8U TaskNum,INT16U Periods);
extern INT8U TaskStatCheck(INT8U TaskNum);
extern void TaskPend(INT8U TaskNum);
extern INT16U usMBCRC16( INT8U * pucFrame, INT16U usLen );
extern void SetFault(INT16U Fault);
extern void ClrFault(INT16U Fault);
extern INT8U FaultChk(INT16U Fault);
extern void delayN_Tick(INT16U bTickNum);
extern void Delay_1p8us(INT16U wDelayTime);
extern INT8U ASCIIConvert(INT8U symbol,INT8U Type);

#endif






































