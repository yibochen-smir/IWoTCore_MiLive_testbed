#include "scm_gpioi2c.h"
#include "scm_tsl2550d.h"
#include "contiki-conf.h"


//extern void delayms(unsigned int d);

#if 1	//def SCM_TSL2550D

#define TSL_ADDR     0x39
#define TSL_CMD_PD   0x00 // power down
#define TSL_CMD_PU   0x03 // power up and read config.
#define TSL_CMD_EXT  0x1D // set extended mode
#define TSL_CMD_RST  0x18 // reset to standard mode
#define TSL_CMD_ADC0 0x43 // read ADC0 value
#define TSL_CMD_ADC1 0x83 // read ADC1 value

void tsl2550_init(void)
{
//	printf("tsl2550_init\r\n");

#ifdef GPIO_SIM_I2C
    i2c_init();
#endif
}

unsigned char tsl2550_powerup(void)
{
    unsigned char readValue;
#ifdef GPIO_SIM_I2C
	i2c_write_single(TSL_ADDR, TSL_CMD_PU);
	i2c_read_single(TSL_ADDR, &readValue);
#else
    unsigned char cmd = TSL_CMD_PU;
	TWID_Write(&twid, TSL_ADDR, 0x0000, 0, &cmd, 1, 0);
	TWID_Read(&twid, TSL_ADDR, 0x0000, 0, &readValue, 1, 0);
#endif	
    return readValue;
}

void tsl2550_powerdown(void)
{
#ifdef GPIO_SIM_I2C
	i2c_write_single(TSL_ADDR, TSL_CMD_PD);
#else
	unsigned char cmd = TSL_CMD_PD;
	TWID_Write(&twid, TSL_ADDR, 0x0000, 0, &cmd, 1, 0);
#endif	
}

void tsl2550_set_extended()
{
#ifdef GPIO_SIM_I2C
	i2c_write_single(TSL_ADDR, TSL_CMD_EXT);
#else
	unsigned char cmd = TSL_CMD_EXT;
	TWID_Write(&twid, TSL_ADDR, 0x0000, 0, &cmd, 1, 0);
#endif	
}

void tsl2550_set_standard()
{
#ifdef GPIO_SIM_I2C
	i2c_write_single(TSL_ADDR, TSL_CMD_RST);
#else
	unsigned char cmd = TSL_CMD_RST;
	TWID_Write(&twid, TSL_ADDR, 0x0000, 0, &cmd, 1, 0);
#endif	
}

unsigned char tsl2550_read_adc0(void)
{
    unsigned char readValue;
#ifdef GPIO_SIM_I2C
	i2c_write_single(TSL_ADDR, TSL_CMD_ADC0);
	//delayms(400);
	clock_delay_msec(400);
	i2c_read_single(TSL_ADDR, &readValue);
#else
	unsigned char cmd = TSL_CMD_ADC0;
	TWID_Write(&twid, TSL_ADDR, 0x0000, 0, &cmd, 1, 0);
	TWID_Read(&twid, TSL_ADDR, 0x0000, 0, &readValue, 1, 0);
#endif	
    return readValue;
}

unsigned char tsl2550_read_adc1(void)
{
    unsigned char readValue;
#ifdef GPIO_SIM_I2C
	i2c_write_single(TSL_ADDR, TSL_CMD_ADC1);
	//delayms(400);
	clock_delay_msec(400);
	i2c_read_single(TSL_ADDR, &readValue);
#else
	unsigned char cmd = TSL_CMD_ADC1;
	TWID_Write(&twid, TSL_ADDR, 0x0000, 0, &cmd, 1, 0);
	TWID_Read(&twid, TSL_ADDR, 0x0000, 0, &readValue, 1, 0);
#endif	
    return readValue;
}


static const uint8_t tsl2550_Ratio[129] =
{
100,100,100,100,100,100,100,100,
100,100,100,100,100,100,99,99,
99,99,99,99,99,99,99,99,
99,99,99,98,98,98,98,98,
98,98,97,97,97,97,97,96,
96,96,96,95,95,95,94,94,
93,93,93,92,92,91,91,90,
89,89,88,87,87,86,85,84,
83,82,81,80,79,78,77,75,
74,73,71,69,68,66,64,62,
60,58,56,54,52,49,47,44,
42,41,40,40,39,39,38,38,
37,37,37,36,36,36,35,35,
35,35,34,34,34,34,33,33,
33,33,32,32,32,32,32,31,
31,31,31,31,30,30,30,30,
30
};

static const uint16_t tsl2550_Count[128] =
{
   0,   1,   2,   3,   4,   5,   6,   7,
   8,   9, 10, 11, 12, 13, 14, 15,
  16, 18, 20, 22, 24, 26, 28, 30,
  32, 34, 36, 38, 40, 42, 44, 46,
  49, 53, 57, 61, 65, 69, 73, 77,
  81, 85, 89, 93, 97, 101, 105, 109,
 115, 123, 131, 139, 147, 155, 163, 171,
 179, 187, 195, 203, 211, 219, 227, 235,
 247, 263, 279, 295, 311, 327, 343, 359,
 375, 391, 407, 423, 439, 455, 471, 487,
 511, 543, 575, 607, 639, 671, 703, 735,
 767, 799, 831, 863, 895, 927, 959, 991,
1039,1103,1167,1231,1295,1359,1423,1487,
1551,1615,1679,1743,1807,1871,1935,1999,
2095,2223,2351,2479,2607,2735,2863,2991,
3119,3247,3375,3503,3631,3759,3887,4015
};

#define TSL_MAX_LUX_VALUE 1846


uint16_t tsl2550_convert_to_lux(uint8_t adc0, uint8_t adc1)
{
  uint32_t count0 = 0, count1 = 0;
  uint8_t ratio = 128;    // default a scaling factor
  uint16_t lux = 0x10;
  uint8_t R;

  if (adc0 & adc1 & 0x80)
  {
//    memcpy_P(&count0, &tsl2550_Count[ adc0 & 0x7F ], sizeof(uint16_t));
//    memcpy_P(&count1, &tsl2550_Count[ adc1 & 0x7F ], sizeof(uint16_t));
		count0 = tsl2550_Count[ adc0 & 0x7F ];
		count1 = tsl2550_Count[ adc1 & 0x7F ];

    // calculate ratio
    if (count0 && (count1 < count0)) // count1 cannot be greater than count0
    {
      ratio = ((uint32_t)(count1 * 128ul) / count0);
      // calculate lux
      // the "256" is a scaling factor
//      memcpy_P(&R, &tsl2550_Ratio[ ratio ], sizeof(uint8_t));
		R = tsl2550_Ratio[ ratio ];
      lux = ((count0 - count1) * R) / 256;
      // range check lux
      if (lux > TSL_MAX_LUX_VALUE)
        lux = TSL_MAX_LUX_VALUE;
    }
  }
  return lux;
}


#endif
