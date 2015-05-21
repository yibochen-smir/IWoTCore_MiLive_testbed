#ifndef _IWOTCORE_H_
#define _IWOTCORE_H_

#include <avr/io.h>

#define DDR(x)         CAT(DDR,  x)
#define PORT(x)        CAT(PORT, x)
#define PIN(x)         CAT(PIN,  x)
#define INLINE static inline
/******************************************************************************
* void IWOTCORE_X_set() sets IWOTCORE_X pin to logical 1 level.
* void IWOTCORE_X_clr() clears IWOTCORE_X pin to logical 0 level.
* void IWOTCORE_X_make_in makes IWOTCORE_X pin as input.
* void IWOTCORE_X_make_out makes IWOTCORE_X pin as output.
* uint8_t IWOTCORE_X_read() returns logical level IWOTCORE_X pin.
* uint8_t IWOTCORE_X_state() returns configuration of IWOTCORE_X port.
*******************************************************************************/
#define IWOTCORE_ASSIGN_PIN(name, port, bit) \
INLINE void  IWOTCORE_##name##_set()         {PORT##port |= (1 << bit);} \
INLINE void  IWOTCORE_##name##_clr()         {PORT##port &= ~(1 << bit);} \
INLINE uint8_t  IWOTCORE_##name##_read()     {return (PIN##port & (1 << bit)) != 0;} \
INLINE uint8_t  IWOTCORE_##name##_state()    {return (DDR##port & (1 << bit)) != 0;} \
INLINE void  IWOTCORE_##name##_make_out()    {DDR##port |= (1 << bit);} \
INLINE void  IWOTCORE_##name##_make_in()     {DDR##port &= ~(1 << bit); PORT##port &= ~(1 << bit);} \
INLINE void  IWOTCORE_##name##_make_pullup() {PORT##port |= (1 << bit);}\
INLINE void  IWOTCORE_##name##_toggle()      {PORT##port ^= (1 << bit);}

IWOTCORE_ASSIGN_PIN(SenEn, D, 4);
IWOTCORE_ASSIGN_PIN(ARMEn, B, 3);       //on Core Board PMOS
//IWOTCORE_ASSIGN_PIN(ARMEn, D, 6);     //on SEES Board

INLINE void  IWOTCORE_ARM_Power_On()
{
	IWOTCORE_ARMEn_make_out();
	IWOTCORE_ARMEn_set();
}

INLINE void  IWOTCORE_ARM_Power_Off()
{
	IWOTCORE_ARMEn_make_out();
	IWOTCORE_ARMEn_clr();
}

INLINE void  IWOTCORE_SENSOR_On()
{
	IWOTCORE_SenEn_make_out();
	IWOTCORE_SenEn_set();
}

INLINE void  IWOTCORE_SENSOR_Off()
{
	IWOTCORE_SenEn_make_out();
	IWOTCORE_SenEn_clr();
}

INLINE void Init_Pin_ADC2(void)
{
	ANT_DIV &= 0xFB; //disables pin DIG1 and DIG2

	DDRF  &= 0xFB;		//configure Pin F3 as input
	PORTF &= 0xFB;		//disable Pin F3 pull-up
}

extern void Init_Pin_ADC3(void);
//extern void delayms(unsigned int d);
extern void delay_5us_cyb(unsigned char d);


#define delay_us(u) delay_5us_cyb((u+4)/5)


// #define _SEES_Ultra_Sound_
/*******YIBO: we delete the definitions of _SEES_Ultra_Sound_ part*******/

// Init IWOTCORE_Outx pin as output.
INLINE void  IWOTCORE_IO_Init()
{
	MCUCR &=  (~(0x10));

    Init_Pin_ADC3();  //YIBO: we don't have decagon sensors currently.
    Init_Pin_ADC2();

	//IWOTCORE_ARMEn_make_out();
	//IWOTCORE_ARMEn_set();

	IWOTCORE_SenEn_make_out();
	IWOTCORE_SenEn_set();
}

#endif //_IWOTCORE_H_

