/**********************************************************************//**
  \file AVR_ExtMiLive_EM_for_EV.c

  \brief

  \author
    hongling.shi@isima.fr

  \internal
  History:
    15/02/2013  hongling - Created
**************************************************************************/
#include "em.h"
#include "scm_gpioi2c.h"
#include "scm_decagon.h"
#include "iwotcore_gpio.h"


#include "contiki-conf.h"
#include "dev/battery-sensor.h"
#include <util/delay.h>
#define delay_us( us )   ( _delay_us( ( us ) ) )


//extern AppMessageRequest_t	appMessage;

//extern void delay_5us(unsigned char d);
//extern void delay_1us();
//extern void delayms(unsigned int d);  //repaced by clock_delay_msec
//extern void delay3us(unsigned int d);

//#define delay_us(u) delay_5us((u+4)/5);    //repaced by delay_us( us ) in <util/delay.h>

#define EV_EM_Wait_FirstBit     			(50)	//Try 50 * 2ms = 100ms
#define EV_EM_GetBit_Wait_Pulse_Cnt 		(1000) 	//1000 * 2us = 2ms
#define EV_EM_GetBit_Sample_Time    		(450) 	//us
#define EV_EM_GetBit_PostSample_Time		(500) 	//us

#define EV_EM_ErrNo_NoResponse			(0x8000)
#define EV_EM_ErrNo_Protocol_Err		(0x4000)
#define EV_EM_ErrNo_Suc_32Bit			(0x2000)
#define EV_EM_ErrNo_Suc_8Bit			(0x1000)

static uint8_t gs_bEVEM;

void delay3us(unsigned int d)
{
    volatile unsigned int i;
    for (i = 0; i<d; i++)
    {
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
    }
}

void delay_1us()
{
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
}


void delay_5us_cyb(unsigned char d)
{
    volatile unsigned char i;
    for (i = 0; i<d; i++)
    {
        ;
    }
    for (i = 0; i<d; i++)
    {
        ;
    }
}

#define EXTtoEM_Wait_FirstBit     		(50)	//Try 50 * 2ms = 100ms
#define EXTtoEM_GetBit_Wait_Pulse_Cnt 		(1000) 	//1000 * 2us = 2ms
#define EXTtoEM_GetBit_Sample_Time    		(450) 	//us
#define EXTtoEM_GetBit_PostSample_Time		(500) 	//us

#define EXTtoEM_ErrNo_NoResponse		(0x8000)
#define EXTtoEM_ErrNo_Protocol_Err		(0x4000)
#define EXTtoEM_ErrNo_Suc_32Bit			(0x2000)
#define EXTtoEM_ErrNo_Suc_8Bit			(0x1000)

static uint8_t gs_bEXTtoEM;

uint8_t EXTtoEM_GetNextBit()
{
	int i;
	for(i=0; i<EXTtoEM_GetBit_Wait_Pulse_Cnt; i++)
	{
		if(EXTtoEM_GetPmReplyState())
		{
			//delay3us(EXTtoEM_GetBit_Sample_Time/3);
			delay_us(EXTtoEM_GetBit_Sample_Time);
			if(EXTtoEM_GetPmReplyState())
			{
				appMessage.data.scm_sensors.NanoRiscStatus |= (1L<<(31-gs_bEXTtoEM));
			}
			gs_bEXTtoEM ++;

			//delay3us(EXTtoEM_GetBit_PostSample_Time/3);
			delay_us(EXTtoEM_GetBit_Sample_Time);
			if(EXTtoEM_GetPmReplyState())
			{
				appMessage.data.scm_sensors.NanoIOErrNo |= EXTtoEM_ErrNo_Protocol_Err;
				return gs_bEXTtoEM;
			}

			if(gs_bEXTtoEM >= 32)
			{
				appMessage.data.scm_sensors.NanoIOErrNo |= EXTtoEM_ErrNo_Suc_32Bit;
				return gs_bEXTtoEM;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			//delay_1us();
			delay_us(1);
		}
	}
	if(gs_bEXTtoEM == 8)
	{
		appMessage.data.scm_sensors.NanoIOErrNo |= EXTtoEM_ErrNo_Suc_8Bit;
		return gs_bEXTtoEM;
	}
	return gs_bEXTtoEM;
}


uint8_t EXTtoEM_ReadEmStatus()
{
	uint8_t bFinish;
	int i;
	appMessage.data.scm_sensors.NanoRiscStatus = 0;
	appMessage.data.scm_sensors.NanoIOErrNo = 0;
	gs_bEXTtoEM = 0;
	for(i=0; i<EXTtoEM_Wait_FirstBit; i++)
	{
		bFinish = EXTtoEM_GetNextBit();
		if(bFinish)
		{
			appMessage.data.scm_sensors.NanoIOErrNo |= bFinish;

//			appMessage.data.scm_sensors.light = appMessage.data.scm_sensors.NanoIOErrNo;
//			appMessage.data.scm_sensors.watermark3 = (appMessage.data.scm_sensors.NanoRiscStatus >> 16) & 0xffff;
//			appMessage.data.scm_sensors.watermark4 = (appMessage.data.scm_sensors.NanoRiscStatus) & 0xffff;
			appMessage.data.scm_sensors.NanoIOCountNo = gs_bEXTtoEM;

			return bFinish;
		}
	}
	appMessage.data.scm_sensors.NanoIOErrNo |= EXTtoEM_ErrNo_NoResponse;


//	appMessage.data.scm_sensors.light = appMessage.data.scm_sensors.NanoIOErrNo;
//	appMessage.data.scm_sensors.watermark3 = (appMessage.data.scm_sensors.NanoRiscStatus >> 16) & 0xffff;
//	appMessage.data.scm_sensors.watermark4 = (appMessage.data.scm_sensors.NanoRiscStatus) & 0xffff;

	return 0;
}

/************************************************************************
**************************************************************************/
void visualizeAirTxStarted(void)
{
// -----  For End-Device  ----
	EXTtoEM_SetPmReqOn();
	EXTtoEM_ReadEmStatus();
	EXTtoEM_SetPmReqOff();

    if(appMessage.data.scm_sensors.NanoIOErrNo != 0x2020)
    {
        EXTtoEM_SetPmReqOn();
        EXTtoEM_ReadEmStatus();
        EXTtoEM_SetPmReqOff();
        appMessage.data.scm_sensors.NanoIOErrCnt ++;
    }
    if(appMessage.data.scm_sensors.NanoIOErrNo != 0x2020)
    {
        appMessage.data.scm_sensors.NanoIOErrCnt += 0x100;
    }

}

/************************************************************************
**************************************************************************/
void visualizeSerialTx(void)
{
// -----  For Coordinator  ----
	EXTtoEM_SetPmReqOn();
	EXTtoEM_ReadEmStatus();
	EXTtoEM_SetPmReqOff();

    if(appMessage.data.scm_sensors.NanoIOErrNo != 0x2020)
    {
        EXTtoEM_SetPmReqOn();
        EXTtoEM_ReadEmStatus();
        EXTtoEM_SetPmReqOff();
        appMessage.data.scm_sensors.NanoIOErrCnt ++;
    }
    if(appMessage.data.scm_sensors.NanoIOErrNo != 0x2020)
    {
        appMessage.data.scm_sensors.NanoIOErrCnt += 0x100;
    }

}
/*---------------------------------------------------------------------------*/
//static void decagon_DateReady(uint16_t data0, uint16_t data1, uint16_t data2, uint16_t data3);
void ADC_PowerON(void)
{
	GPIO_D5_make_out();    //MiLive
	GPIO_D5_set();

//	IWOTCORE_SenEn_make_out();
//	IWOTCORE_SenEn_set();

}
void ADC_PowerOFF(void)
{
	GPIO_D5_make_out();     //MiLive
	GPIO_D5_clr();

//	IWOTCORE_SenEn_make_out();
//	IWOTCORE_SenEn_clr();
}

/*****----------------------------------Adapted to Contiki sensors process ---------------------------------*****/
const struct sensors_sensor em_battery_sensor;
/*---------------------------------------------------------------------------*/
static int
value(int type)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
configure(int type, int c)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status(int type)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(embattery_sensor, BATTERY_SENSOR, value, configure, status);

//eof AVR_ExtMiLive_EM_for_EV.c

