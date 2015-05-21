#include "scm_gpioi2c.h"
#include "lm75.h"

//extern void delayms(unsigned int d);

#if 1	//def SCM_TSL2550D

#define LM75_ADDR     0x48

void lm75_init(void)
{
//	printf("lm75_init\r\n");

#ifdef GPIO_SIM_I2C
    i2c_init();
#endif
}

short lm75_read_temperature(void)
{
    char Val[2];
    short TempReg;
#ifdef GPIO_SIM_I2C
	i2c_read_double(LM75_ADDR, &Val[0], &Val[1]);
#else
	unsigned char cmd = LM75_CMD_ADC0;
	TWID_Read(&twid, LM75_ADDR, 0x0000, 0, &Val, 2, 0);
#endif
    TempReg = Val[0]<<1 ;
    TempReg  += Val[1]>>7;
    return TempReg;
}

#endif

