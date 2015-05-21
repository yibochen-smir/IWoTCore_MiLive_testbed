#ifndef _SCM_GPIOI2C_H_
#define _SCM_GPIOI2C_H_

#include <avr/io.h>

#define DDR(x)         CAT(DDR,  x)
#define PORT(x)        CAT(PORT, x)
#define PIN(x)         CAT(PIN,  x)
#define INLINE static inline

//#include "gpio.h"
#define HAL_ASSIGN_PIN(name, port, bit) \
INLINE void  GPIO_##name##_set()         {PORT##port |= (1 << bit);} \
INLINE void  GPIO_##name##_clr()         {PORT##port &= ~(1 << bit);} \
INLINE uint8_t  GPIO_##name##_read()     {return (PIN##port & (1 << bit)) != 0;} \
INLINE uint8_t  GPIO_##name##_state()    {return (DDR##port & (1 << bit)) != 0;} \
INLINE void  GPIO_##name##_make_out()    {DDR##port |= (1 << bit);} \
INLINE void  GPIO_##name##_make_in()     {DDR##port &= ~(1 << bit); PORT##port &= ~(1 << bit);} \
INLINE void  GPIO_##name##_make_pullup() {PORT##port |= (1 << bit);}\
INLINE void  GPIO_##name##_toggle()      {PORT##port ^= (1 << bit);}

/******************************************************************************
                   Inline static functions section
******************************************************************************/
HAL_ASSIGN_PIN(B0, B, 0);
HAL_ASSIGN_PIN(B1, B, 1);
HAL_ASSIGN_PIN(B2, B, 2);
HAL_ASSIGN_PIN(B3, B, 3);
HAL_ASSIGN_PIN(B4, B, 4);
HAL_ASSIGN_PIN(B5, B, 5);
HAL_ASSIGN_PIN(B6, B, 6);
HAL_ASSIGN_PIN(B7, B, 7);

HAL_ASSIGN_PIN(D0, D, 0);
HAL_ASSIGN_PIN(D1, D, 1);
HAL_ASSIGN_PIN(D2, D, 2);
HAL_ASSIGN_PIN(D3, D, 3);
HAL_ASSIGN_PIN(D4, D, 4);
HAL_ASSIGN_PIN(D5, D, 5);
HAL_ASSIGN_PIN(D6, D, 6);
HAL_ASSIGN_PIN(D7, D, 7);

HAL_ASSIGN_PIN(E0, E, 0);
HAL_ASSIGN_PIN(E1, E, 1);
HAL_ASSIGN_PIN(E2, E, 2);
HAL_ASSIGN_PIN(E3, E, 3);
HAL_ASSIGN_PIN(E4, E, 4);
HAL_ASSIGN_PIN(E5, E, 5);
HAL_ASSIGN_PIN(E6, E, 6);
HAL_ASSIGN_PIN(E7, E, 7);

HAL_ASSIGN_PIN(F0, F, 0);
HAL_ASSIGN_PIN(F1, F, 1);
HAL_ASSIGN_PIN(F2, F, 2);
HAL_ASSIGN_PIN(F3, F, 3);
HAL_ASSIGN_PIN(F4, F, 4);
HAL_ASSIGN_PIN(F5, F, 5);
HAL_ASSIGN_PIN(F6, F, 6);
HAL_ASSIGN_PIN(F7, F, 7);

HAL_ASSIGN_PIN(G0, G, 0);
HAL_ASSIGN_PIN(G1, G, 1);
HAL_ASSIGN_PIN(G2, G, 2);
HAL_ASSIGN_PIN(G3, G, 3);
HAL_ASSIGN_PIN(G4, G, 4);
HAL_ASSIGN_PIN(G5, G, 5);

HAL_ASSIGN_PIN(I2C_CLK, D, 0);
HAL_ASSIGN_PIN(I2C_DATA, D, 1);

HAL_ASSIGN_PIN(ADC_INPUT_0, F, 0);
HAL_ASSIGN_PIN(ADC_INPUT_1, F, 1);
HAL_ASSIGN_PIN(ADC_INPUT_2, F, 2);
HAL_ASSIGN_PIN(ADC_INPUT_3, F, 3);
HAL_ASSIGN_PIN(ADC_INPUT_4, F, 4);
HAL_ASSIGN_PIN(ADC_INPUT_5, F, 5);
HAL_ASSIGN_PIN(ADC_INPUT_6, F, 6);
HAL_ASSIGN_PIN(ADC_INPUT_7, F, 7);

HAL_ASSIGN_PIN(USART0_RXD, E, 0);
HAL_ASSIGN_PIN(USART0_TXD, E, 1);
HAL_ASSIGN_PIN(USART0_EXTCLK, E, 2);

HAL_ASSIGN_PIN(USART1_RXD, D, 2);
HAL_ASSIGN_PIN(USART1_TXD, D, 3);
HAL_ASSIGN_PIN(USART1_EXTCLK, D, 5);
HAL_ASSIGN_PIN(USART_RTS, D, 4);
HAL_ASSIGN_PIN(USART_CTS, D, 5);

HAL_ASSIGN_PIN(IRQ_0, D, 0);
HAL_ASSIGN_PIN(IRQ_1, D, 1);
HAL_ASSIGN_PIN(IRQ_2, D, 2);
HAL_ASSIGN_PIN(IRQ_3, D, 3);
HAL_ASSIGN_PIN(IRQ_4, E, 4);
HAL_ASSIGN_PIN(IRQ_5, E, 5);
HAL_ASSIGN_PIN(IRQ_6, E, 6);
HAL_ASSIGN_PIN(IRQ_7, E, 7);

// Macros for the SPI_CS pin manipulation.
HAL_ASSIGN_PIN(SPI_CS, G, 5);
// Macros for the SPI_CS pin manipulation.
HAL_ASSIGN_PIN(HW_SPI_CS, B, 0);
// Macros for the SPI_SCK pin manipulation.
HAL_ASSIGN_PIN(SPI_SCK, B, 1);
// Macros for the SPI_MOSI pin manipulation.
HAL_ASSIGN_PIN(SPI_MOSI, B, 2);
// Macros for the SPI_MISO pin manipulation.
HAL_ASSIGN_PIN(SPI_MISO, B, 3);



#define GPIO_SIM_I2C

#define I2C_EN		GPIO_B6_make_out();\
					GPIO_B6_set();					

#define I2C_DIS		GPIO_B6_make_out();\
					GPIO_B6_clr();					

#define Set_SDA		GPIO_I2C_DATA_make_out();\
					GPIO_I2C_DATA_set()
#define Clr_SDA		GPIO_I2C_DATA_make_out();\
					GPIO_I2C_DATA_clr()
                    
#define Set_SCL		GPIO_I2C_CLK_make_out();\
					GPIO_I2C_CLK_set()
#define Clr_SCL		GPIO_I2C_CLK_make_out();\
					GPIO_I2C_CLK_clr()

#define Switch_SDA_Input  GPIO_I2C_DATA_make_in()

#define Switch_SCL_Input  GPIO_I2C_CLK_make_in()

#define Get_SDA  GPIO_I2C_DATA_read()


void i2c_init(void);

void i2c_deinit(void);

void i2c_write_single(unsigned char ADDR, unsigned char CMD);

void i2c_read_single(unsigned char ADDR, unsigned char *pDATA);

void i2c_read_double(unsigned char ADDR, unsigned char *pDATA1, unsigned char *pDATA2);

void _i2c_delay(void);
void i2c_start(void);
void i2c_stop(void);
unsigned char i2c_tx(unsigned char data);
unsigned char i2c_rx(char ack);

void ADC_PowerOFF(void);
void ADC_PowerON(void);

#endif //_SCM_GPIOI2C_H_

