#include "scm_gpioi2c.h"
//#include "scm_hsdtvi.h"
//#include "em.h" //For testing definitions....


#if 0	//For AT91
#define PIN_SDA_OUT_1    {1 << 3, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_SDA_OUT_0    {1 << 3, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define PIN_SCL_OUT_1    {1 << 4, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_SCL_OUT_0    {1 << 4, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define PIN_SDA_IN    	 {1 << 3, AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_INPUT, PIO_DEFAULT}

static const Pin pin_I2C_SDA_1[] = {PIN_SDA_OUT_1};
static const Pin pin_I2C_SDA_0[] = {PIN_SDA_OUT_0};
static const Pin pin_I2C_SCL_1[] = {PIN_SCL_OUT_1};
static const Pin pin_I2C_SCL_0[] = {PIN_SCL_OUT_0};
static const Pin pin_I2C_SDA_In[] = {PIN_SDA_IN};

#define Set_SDA		PIO_Configure(pin_I2C_SDA_1, PIO_LISTSIZE(pin_I2C_SDA_1));\
					PIO_Set(pin_I2C_SDA_1)
#define Clr_SDA		PIO_Configure(pin_I2C_SDA_0, PIO_LISTSIZE(pin_I2C_SDA_0));\
					PIO_Clear(pin_I2C_SDA_0)

#define Set_SCL		PIO_Configure(pin_I2C_SCL_1, PIO_LISTSIZE(pin_I2C_SCL_1));\
					PIO_Set(pin_I2C_SCL_1)
#define Clr_SCL		PIO_Configure(pin_I2C_SCL_0, PIO_LISTSIZE(pin_I2C_SCL_0));\
					PIO_Clear(pin_I2C_SCL_0)

#define Switch_SDA_Input  PIO_Configure(pin_I2C_SDA_In, PIO_LISTSIZE(pin_I2C_SDA_In))
#define Get_SDA  PIO_Get(pin_I2C_SDA_In)
#endif


void i2c_init(void)
{
//    PIO_Configure(pin_I2C_SDA_1, PIO_LISTSIZE(pin_I2C_SDA_1));
//    PIO_Configure(pin_I2C_SCL_1, PIO_LISTSIZE(pin_I2C_SCL_1));

//	I2C_EN;

    Set_SDA;
    Set_SCL;
}

void i2c_deinit(void)
{
//	I2C_DIS;

//  Clr_SDA;
//  Clr_SCL;
	Set_SCL;
	Set_SDA;

//	Switch_SDA_Input;
//	Switch_SCL_Input;
}

void _i2c_delay(void)
{
	volatile unsigned char d;
	d++; d++; d++;
}

void i2c_start(void)
{
	 /* release two wires */
	Set_SDA;
	Set_SCL;
	_i2c_delay();

	/* pull down the SDA */
	Clr_SDA;
	_i2c_delay();

	/* end */
	Clr_SCL;
	_i2c_delay();
}

void i2c_stop(void)
{
        /*  */
    Clr_SDA;
    Set_SCL;
    _i2c_delay();

    /* pull up the SDA */
    Set_SDA;
    _i2c_delay();
}

unsigned char i2c_tx(unsigned char data)
{
	unsigned char i;
	volatile unsigned char d = 0;

	// printf("sending(0x%x) ", data);
	for(i=0; i<8; i++)
	{
		/* put data on SDA */
		if (data & 0x80) {
			// printf("1 ");
			Set_SDA;
		} else {
			// printf("0 ");
			Clr_SDA;
		}
		_i2c_delay();

		/* emit the SDA via trigger a square-wave on SCL */
		Set_SCL;
		d++; d++; d++;
		data <<= 1;
		Clr_SCL;
	}
	// printf("\n");
	d++; d++; d++;

//	Set_SDA;
	Switch_SDA_Input;
	Set_SCL;
	_i2c_delay();
//	i = IORD_ALTERA_AVALON_PIO_DATA(SDA_BASE); /* ACK bit */
	i = Get_SDA;
	Clr_SCL;

//	printf("i2c_tx Ack = %d\r\n", i);
	return i;
}

unsigned char i2c_rx(char ack)
{
	char i, data = 0;

	Switch_SDA_Input;
//	Set_SDA;
	// printf("received: ");
	for (i=0; i<8; i++)
	{
		data <<= 1;

//		do {
			Set_SCL;
//		} while (IORD_ALTERA_AVALON_PIO_DATA(SCL_BASE) == 0);

		_i2c_delay();
//		if (IORD_ALTERA_AVALON_PIO_DATA(SDA_BASE))
		if(Get_SDA)
		{
		    // printf("1 ");
			data |= 1;
		}
		else
		{
		    // printf("0 ");
		}
		Clr_SCL;

		_i2c_delay();
	}
	// printf("\n");

	if (ack) {
		Clr_SDA;
	} else {
		Set_SDA;
	}

	Set_SCL;
	_i2c_delay();         /* sending (N)ACK */
	Clr_SCL;

//	Set_SDA;
	return data;
}

void i2c_write_single(unsigned char ADDR, unsigned char CMD)
{
	i2c_start();	//总线上插入开始
	i2c_tx(ADDR<<1);	//发出i2c地址（写）
	i2c_tx(CMD);	//发出命令
	i2c_stop(); 	//结束本次通讯
}

void i2c_read_single(unsigned char ADDR, unsigned char *pDATA)
{
	i2c_start();	//总线上插入开始
	i2c_tx( (ADDR<<1) | 0x01 );	//发出i2c地址（读）
	*pDATA = i2c_rx(0);	//发出命令
	i2c_stop(); 	//结束本次通讯
}

void i2c_read_double(unsigned char ADDR, unsigned char *pDATA1, unsigned char *pDATA2)
{
	i2c_start();	//总线上插入开始
	i2c_tx( (ADDR<<1) | 0x01 );	//发出i2c地址（读）
	*pDATA1 = i2c_rx(0);	//发出命令
	*pDATA2 = i2c_rx(0);	//发出命令
	i2c_stop(); 	//结束本次通讯
}




