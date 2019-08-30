#ifndef MC20_H
#define MC20_H

#define MSGCMD_SEZE  20
#define PHONE_SIZE   11
#define GPS_BUF_LEN 15
#define GPS_ATITU_LEN 7
#define GPS_PRECI_LEN 4
#define BALLOON_STA_LEN 4
#define TEMPER_LEN   4

#define SHUT_DOWN 0
#define START     1

#define COMP_HIGH       120
#define MOTOR_PREPAER   (COMP_HIGH-50)
#define BACK_CMP_HIGH   (COMP_HIGH*2)
#define NOT_CLR_TIME    150



typedef struct
{ 
	struct 
	{
	  INT8U bMark[2];
	  INT8U bGrade[3];
	  INT8U dwMinute[6];
	  INT8U tbEnter;
	} sLatitude;       //weidu
	
	struct 
	{
	  INT8U bMark[2];
	  INT8U bGrade[4];
	  INT8U dwMinute[6];
	  INT8U tbEnter;
	} sLongitude;       //jingdu
	
    struct
    {
		INT8U bMark[2];
		INT8U bFixStatus;
		INT8U tbEnter;
	} sFixStat;

	struct
    {
		INT8U bMark[2];
		INT8U tbPrecision[GPS_PRECI_LEN];
		INT8U tbEnter;
	} sPrecision;
	
	struct
    {
		INT8U bMark[2];
		INT8U tbAtitu[GPS_ATITU_LEN];
		INT8U tbEnter;
	} sAtitude;

	struct
	{
		INT8U bMark[2];
		INT8U tbTemper[TEMPER_LEN];
		INT8U tbEnter;
	} sTemperSta;
	
	struct
    {
		INT8U bMark[2];
		INT8U tbBloStat[BALLOON_STA_LEN];
		//INT8U tbEnter;  //last should move out this enter
	} sBalloonSta;
	
	INT8U bEndChar;
	INT32U dwLatiMinut;
	INT32U dwLontiMinut;
	INT32S dwAtitude;
	INT8U tbDataBuf[GPS_BUF_LEN];
} sGPSDtManage;

 typedef struct 
 {
 	    struct 
		{
		  INT8U bMark[2];
		  INT8U bGrade[3];
		  INT8U dwMinute[6];
		  INT8U tbEnter;
		} sLatitude;       //weidu
		struct 
		{
		  INT8U bMark[2];
		  INT8U bGrade[4];
		  INT8U dwMinute[6];
		  INT8U tbEnter;
		} sLongitude;       //jingdu
	    struct
	    {
			INT8U bMark[2];
			INT8U bFixStatus;
			INT8U tbEnter;
		} sFixStat;
		struct
	    {
			INT8U bMark[2];
			INT8U tbAtitu[GPS_ATITU_LEN];
			INT8U tbEnter;
		} sAtitude;
		struct
		{
			INT8U bMark[2];
			INT8U tbTemper[TEMPER_LEN];
			INT8U tbEnter;
		} sTemperSta;
		struct
	    {
			INT8U bMark[2];
			INT8U tbBloStat[BALLOON_STA_LEN];
			//INT8U tbEnter;  //last should move out this enter
		} sBalloonSta;
		
		INT8U bEndChar;  //this position can not move
		INT32S dwAtitude;
 }sGPSDtBkUp;
 
typedef struct
{
    INT8U DealStatus;
	INT8U tbCommand[MSGCMD_SEZE];
	INT8U tbPhoneBuff[PHONE_SIZE];
	INT8U *pMsgContent;
	INT8U *tbPhoneNum;
} sMsgStr;

extern XINT8U tbMsgPhone[];

extern xdata sGPSDtManage sGPSData;
extern xdata sGPSDtBkUp sGPSDtBackUp;

extern void MC20Init();
extern void MC20Start();
extern void MC20Task(INT8U TaskNum);
extern void MC20PowerCtr(INT8U Cmd);

#endif


































