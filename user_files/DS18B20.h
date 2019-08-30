#ifndef DS18B20_H
#define DS18B20_H

#define DS18B20_PORT  IO_0
#define DS18B20_PIN   PIN_3 

#define TEMPER_MINUS  0
#define TEMPER_PLUS   1

extern INT16U wTemperCal;
extern INT8U TemperType;

extern void TemperCtr();
extern void DS_Init();

#endif































