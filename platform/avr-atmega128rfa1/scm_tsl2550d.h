#ifndef _SCM_TSL2550D_H_
#define _SCM_TSL2550D_H_

#include <avr/io.h>


void tsl2550_init(void);

unsigned char tsl2550_powerup(void);

void tsl2550_powerdown(void);

void tsl2550_set_extended();

void tsl2550_set_standard();

uint8_t tsl2550_read_adc0(void);

uint8_t tsl2550_read_adc1(void);

uint16_t tsl2550_convert_to_lux(uint8_t adc0, uint8_t adc1);


#endif //_SCM_TSL2550D_H_
