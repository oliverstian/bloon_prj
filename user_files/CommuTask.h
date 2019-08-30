#ifndef COMU_TASK_H
#define COMU_TASK_H

#define FAIL         0
#define SUCCESS      1
#define DT_INVALID   3

#define UART_1      0x01
#define UART_2      0x02
#define UART_TI_FLG 0x01
#define UART_RI_FLG 0x02

#define INTER_RX    0x01
#define EXTER_RX    0x02
#define INTER_TX    0x03
#define EXTER_TX    0x04

#define BUFF_EMPTY  0
#define RX_READY    1
#define RX_FINISH   3
#define RX_FINISH_TIME 2  //2 tick sycle = 10ms
#define TX_BUSY     0
#define TX_READY    1

#define INTER_RX_MAX_SIZE  150
#define INTER_TX_MAX_SIZE  60
#define EXTER_RX_MAX_SIZE  60
#define EXTER_TX_MAX_SIZE  150

extern XINT8U InterRxBuff[];
extern XINT8U BackUpRxBuf[];
extern XINT8U InterTxBuff[];
extern XINT8U ExterRxBuff[];
extern XINT8U ExterTxBuff[];

typedef struct 
{
	INT8U bRxStatus;
	INT8U bRxLength;
	INT8U bRxCnt;
	INT8U bRxSize;
	INT8U bReadCnt;
	INT8U *pRxBuffer;
} sRxManage;

typedef struct
{
	INT8U bSendStatus;
	INT8U bSendLength;
	INT8U bTxCnt;
	INT8U* pSendBuff;
} sTxManage;

typedef struct
{
	INT8U bRxStatus;
	INT8U bRxCnt;
	INT8U bRxTimeCnt;
	INT8U bRxTimeOut;
	INT8U bRxSize;
	INT8U *pRxBuffer;
} sInterRxManage;

extern idata sInterRxManage sInterRx;
extern idata sRxManage sExterRx;
extern idata sTxManage sInterTx;
extern idata sTxManage sExterTx;

extern void UartInit();
extern bit UartReadFlg(INT8U Uartx,INT8U UartFlg);
extern void UartClearFlg(INT8U Uartx,INT8U UartFlg);
extern void ExterComTask(INT8U TaskNum);
extern void ClrInterRxBuf();
extern INT8U UartSendData(INT8U TxType,INT8U *OutBuff,INT8U DataLen);

#endif




































