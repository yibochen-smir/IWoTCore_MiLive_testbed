#ifndef _SCM_DECAGON_H_
#define _SCM_DECAGON_H_
#include "contiki-conf.h"
#include "adc.h"

//typedef void (*F_decagon_Callback)(void *pdecagonControl); // callback

typedef struct
{
  	uint16_t decagonData0;
  	uint16_t decagonData1;
  	uint16_t decagonData2;
/*  	uint16_t decagonData2_ch0;
  	uint16_t decagonData2_ch1;
  	uint16_t decagonData2_ch2;
  	uint16_t decagonData2_ch3;
  	uint16_t decagonData2_ch4;
  	uint16_t decagonData2_ch5;
  	uint16_t decagonData2_ch6;
  	uint16_t decagonData2_ch7;    */
  	uint16_t decagonData3;
  	uint16_t siscurData;
  	F_decagon_Callback decagonCallback;
}decagonControl_t_2;

// states
typedef enum
{
  	DECA_IDLE,      		// idle
  	DECA_BUSY,      		// opened and ready to be used
  	DECA_DATA0,			// performs request channel1
  	DECA_DATA1,       	// performs request channel1
  	DECA_DATA2,       	// performs request channel2
/*  	DECA_DATA2_CH0,	   		// performs request channel2
  	DECA_DATA2_CH1,	   		// performs request channel2
  	DECA_DATA2_CH2,	   		// performs request channel2
  	DECA_DATA2_CH3,	   		// performs request channel2
  	DECA_DATA2_CH4,	   		// performs request channel2
  	DECA_DATA2_CH5,	   		// performs request channel2
  	DECA_DATA2_CH6,	   		// performs request channel2
  	DECA_DATA2_CH7//,	   		// performs request channel2    */
  	DECA_DATA3,
  	DECA_SIS_CUR
}decagonStates_t;

typedef uint8_t result_t;
#define BC_SUCCESS 0
#define BC_FAIL    1


result_t opendecagon();

result_t closedecagon();

result_t readdecagonData(F_decagon_Callback callback);

result_t readSISCurData(F_decagon_Callback callback);

void bspdecagonHandler(decagonStates_t decagonState_cyb);

#endif //_SCM_DECAGON_H_

