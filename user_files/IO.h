#ifndef IO_H
#define IO_H

#define IO_0  0x01
#define IO_1  0x02
#define IO_2  0x04
#define IO_3  0x08
#define IO_4  0x10
#define IO_5  0x20
#define MAX_IO_NUM   0x3f

#define PIN_0   0x01
#define PIN_1   0x02
#define PIN_2   0x04
#define PIN_3   0x08
#define PIN_4   0x10
#define PIN_5   0x20
#define PIN_6   0x40
#define PIN_7   0x80
#define ALL_PIN 0xff

#define POWE_KEY_PORT  IO_0
#define POWE_KEY_PIN   PIN_4

#define POWE_EN_PORT  IO_0
#define POWE_EN_PIN   PIN_5


#define MCU_LED_PORT  IO_0
#define MCU_LED_PIN   PIN_7

#define MOTER_PORT IO_4
#define MOTER_PIN  PIN_2

#define MC_PW_CONTR_PORT  IO_4
#define MC_PW_CONTR_PIN   PIN_6

#define TEMPER_CTR_PORT   IO_0
#define TEMPER_CTR_PIN    PIN_6
   



#define TWO_WAY_MODE    0x00     //weak pull up
#define PUSH_PULL_MODE  0x01     
#define HIGH_RES_MODE   0x10     
#define OPEN_DRAIN_MODE 0x11     //no resistor pull up,needs to add pull up resistor
#define MAX_MODE_NUM    3

extern void Config_IO(INT8U IO,INT8U PIN,INT8U Mode);
extern void Set_IO(INT8U Port,INT8U Pin_Num);
extern void Reset_IO(INT8U Port,INT8U Pin_Num);
extern INT8U Read_IO(INT8U Port,INT8U Pin_Num);
extern void IOInit();

#endif




































