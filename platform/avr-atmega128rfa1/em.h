#ifndef _AVR_ExtMiLive_EM_for_EV_H_
#define _AVR_ExtMiLive_EM_for_EV_H_

#include <avr/io.h>

#define DDR(x)         CAT(DDR,  x)
#define PORT(x)        CAT(PORT, x)
#define PIN(x)         CAT(PIN,  x)
#define INLINE static inline
/******************************************************************************
* void EXTtoEM_X_set() sets EXTtoEM_X pin to logical 1 level.
* void EXTtoEM_X_clr() clears EXTtoEM_X pin to logical 0 level.
* void EXTtoEM_X_make_in makes EXTtoEM_X pin as input.
* void EXTtoEM_X_make_out makes EXTtoEM_X pin as output.
* uint8_t EXTtoEM_X_read() returns logical level EXTtoEM_X pin.
* uint8_t EXTtoEM_X_state() returns configuration of EXTtoEM_X port.
*******************************************************************************/
#define EXTtoEM_ASSIGN_PIN(name, port, bit) \
INLINE void  EXTtoEM_##name##_set()         {PORT##port |= (1 << bit);} \
INLINE void  EXTtoEM_##name##_clr()         {PORT##port &= ~(1 << bit);} \
INLINE uint8_t  EXTtoEM_##name##_read()     {return (PIN##port & (1 << bit)) != 0;} \
INLINE uint8_t  EXTtoEM_##name##_state()    {return (DDR##port & (1 << bit)) != 0;} \
INLINE void  EXTtoEM_##name##_make_out()    {DDR##port |= (1 << bit);} \
INLINE void  EXTtoEM_##name##_make_in()     {DDR##port &= ~(1 << bit); PORT##port &= ~(1 << bit);} \
INLINE void  EXTtoEM_##name##_make_pullup() {PORT##port |= (1 << bit);}\
INLINE void  EXTtoEM_##name##_toggle()      {PORT##port ^= (1 << bit);}

//	AVR G0  PmReq     -->  EM PA0
//	AVR G1  PmReply   <--  EM PA1

EXTtoEM_ASSIGN_PIN(PmReq, G, 0);
EXTtoEM_ASSIGN_PIN(PmReply, G, 1);

// Init EXTtoEM_PmReq pin as output.
// Init EXTtoEM_PmReply pin as input.
INLINE void  EXTtoEM_Init()
{
	EXTtoEM_PmReq_make_out();
	EXTtoEM_PmReq_clr();

	TRX_CTRL_1 &= 0x7F; //disables pin DIG3 and pin DIG4
	ANT_DIV &= 0xFB; //disables pin DIG1 and DIG2

	EXTtoEM_PmReply_make_in();
}

INLINE void  EXTtoEM_SetPmReqOn()
{
	TRX_CTRL_1 &= 0x7F; //disables pin DIG3 and pin DIG4
	ANT_DIV &= 0xFB; //disables pin DIG1 and DIG2

	EXTtoEM_PmReq_make_out();
	EXTtoEM_PmReq_set();
}

INLINE void  EXTtoEM_SetPmReqOff()
{
	TRX_CTRL_1 &= 0x7F; //disables pin DIG3 and pin DIG4
	ANT_DIV &= 0xFB; //disables pin DIG1 and DIG2

	EXTtoEM_PmReq_make_out();
	EXTtoEM_PmReq_clr();
}

INLINE uint8_t EXTtoEM_GetPmReplyState()
{
	TRX_CTRL_1 &= 0x7F; //disables pin DIG3 and pin DIG4
	ANT_DIV &= 0xFB; //disables pin DIG1 and DIG2

	EXTtoEM_PmReply_make_in();
	return EXTtoEM_PmReply_read();
}

extern uint8_t EXTtoEM_ReadEmStatus();

void visualizeAirTxStarted(void);
void visualizeSerialTx(void);
//static void decagon_DateReady(void *pdecagonControl);

#endif //_AVR_ExtMiLive_EM_for_EV_H_

