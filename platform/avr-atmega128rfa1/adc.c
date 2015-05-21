#include <adc.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "contiki.h"
#include "contiki-lib.h"


#define delay_us( us )   ( _delay_us( ( us ) ) )


//#define memcpy_P   memcpy
/******************************************************************************
                   Types section
******************************************************************************/
typedef enum
{
  	IDLE,      // idle
  	DATA,      // performs request
  	BUSY       // the module is ready to start conversion
} AdcStates_t;

typedef struct
{
  	void (*callback)(void); // address of callback
} HalAdcControl_t;


void halSigAdcHandler(AdcStates_t halAdcState_cyb);


/******************************************************************************
                   Defines section
******************************************************************************/
#define ALL_CHANNEL_MASK          0x1F
#define CHANNEL_MASK_1            0x01
#define CHANNEL_MASK_2            0x03
#define CHANNEL_MASK_3            0x04
#define CHANNEL_MASK_4            0x0C
#define DELAY_FOR_STABILIZE       125

/******************************************************************************
                   Constants section
******************************************************************************/
PROGMEM_DECLARE(const uint8_t halAdcDivider[5]) = {3, 4, 5, 6, 7};


/******************************************************************************
                   Global variables section
******************************************************************************/
#ifndef NULL
#define NULL    0
#endif

static volatile uint8_t halAdcResolution = RESOLUTION_8_BIT;
static volatile void *halAdcDataPointer = NULL;
static volatile uint16_t halAdcCurCount = 0;
static volatile uint16_t halAdcMaxCount = 0;

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
Initializations the ADC.
Parameters:
  param - pointer to parameter structure
Returns:
  none.
******************************************************************************/
void  halOpenAdc(HAL_AdcParams_t *param)
{
  halAdcMaxCount = param->selectionsAmount;
  halAdcResolution = param->resolution;
  halAdcDataPointer = param->bufferPointer;

  /* sets voltage reference */
  ADMUX = param->voltageReference;
  /* Enable left adjust result */
  if (RESOLUTION_8_BIT == halAdcResolution)
    ADMUX |= (1 << ADLAR);

  uint8_t tmp;
  memcpy_P(&tmp, &(halAdcDivider[param->sampleRate]), 1);
  ADCSRA = tmp | (1 << ADEN);
}

/******************************************************************************
Starts convertion on the ADC channel.
Parameters:
  channel - channel number.
Returns:
  none.
******************************************************************************/
void halStartAdc(uint8_t channel)
{
  halAdcCurCount = 0;
  /* disable digital buffers */
  if (HAL_ADC_CHANNEL3 >= channel)
  {
    DIDR0 = (1 << channel);
  }
  else
  {
    if ((HAL_ADC_DIFF_CHANNEL0 == channel) || (HAL_ADC_DIFF_CHANNEL2 == channel))
      DIDR0 = CHANNEL_MASK_1;
    else if ((HAL_ADC_DIFF_CHANNEL1 == channel) || (HAL_ADC_DIFF_CHANNEL3 == channel))
      DIDR0 = CHANNEL_MASK_2;
    else if ((HAL_ADC_DIFF_CHANNEL4 == channel) || (HAL_ADC_DIFF_CHANNEL6 == channel))
      DIDR0 = CHANNEL_MASK_3;
    else if ((HAL_ADC_DIFF_CHANNEL5 == channel) || (HAL_ADC_DIFF_CHANNEL7 == channel))
      DIDR0 = CHANNEL_MASK_4;
  }

  uint8_t tmp = ADMUX & ALL_CHANNEL_MASK;

  /* clear previous channel number */
  ADMUX &= ~ALL_CHANNEL_MASK;
  /* set current channel number */
  ADMUX |= channel;

  /* if new differential channel is settled then must make 125 us delay for gain stabilize. */
  if ((tmp != channel) && (HAL_ADC_CHANNEL3 < channel))
    //delay_us(DELAY_FOR_STABILIZE);
	//_delay_us(DELAY_FOR_STABILIZE);
    clock_delay_usec(DELAY_FOR_STABILIZE);
  if (halAdcMaxCount > 1)
    ADCSRA |= ((1 << ADIE)  | (1 << ADATE) | (1 << ADSC));  // Starts running mode
  else
    ADCSRA |= ((1 << ADIE) | (1 << ADSC)); // Starts one conversion
}

/******************************************************************************
Closes the ADC.
Parameters:
  none
Returns:
  none
******************************************************************************/
void halCloseAdc(void)
{
  ADMUX  = 0;
  ADCSRA = 0;
  // Digital input enable
  DIDR0 = 0;
}

/******************************************************************************
ADC conversion complete interrupt handler.
******************************************************************************/
ISR(ADC_vect)
{
  //BEGIN_MEASURE
  // Read ADC conversion result
  if (RESOLUTION_8_BIT == halAdcResolution)
    ((uint8_t *)halAdcDataPointer)[halAdcCurCount++] = ADCH;
  else {
    ((uint16_t *)halAdcDataPointer)[halAdcCurCount++] = ADC;	
  }

  if (halAdcCurCount == halAdcMaxCount)
  {
    // Disable ADC Interrupt
    ADCSRA &= ~(1 << ADIE);
	halSigAdcHandler(DATA);
    //halSigAdcInterrupt();
  }
  //END_MEASURE(HALISR_ADC_TIME_LIMIT)
}






/******************************************************************************
                   Global variables section
******************************************************************************/
AdcStates_t halAdcState = IDLE; // Monitors current state
HalAdcControl_t halAdcControl;

/******************************************************************************
                   Implementations section
******************************************************************************/
/******************************************************************************
Opens the ADC to make the measuring on a ADC channel.
Parameters:
  param - pointer to parameter structure
Returns:
  -1 - unsupported parameter or ADC is busy.
   0 - on success.
******************************************************************************/
int HAL_OpenAdc(HAL_AdcParams_t *param)
{
  if (IDLE != halAdcState)
    return -1;
  if (NULL == param)
    return -1;
  if (NULL == param->bufferPointer)
    return -1;
  if (param->resolution > RESOLUTION_10_BIT)
    return -1;
  /* unsupported voltage reference */
  if (param->voltageReference & 0x3F)
    return -1;
  /* adc speed must be only 9600 or 4800 SPS for 10 bit resolution */
  if ((RESOLUTION_10_BIT == param->resolution) && (param->sampleRate < ADC_9600SPS))
    return -1;

  halAdcState = BUSY;
  halOpenAdc(param);
  halAdcControl.callback = param->callback;
  return 0;
}

/******************************************************************************
Starts ADC with the parameters that were defined at HAL_OpenAdc.
Parameters:
  channel - number of channel
Returns:
  -1 - the ADC was not opened, unsupported channel number.
   0 - on success.
******************************************************************************/
int HAL_ReadAdc(HAL_AdcChannelNumber_t channel)
{
  if (BUSY != halAdcState)
    return -1;
  if (((channel > HAL_ADC_CHANNEL3) && (channel < HAL_ADC_DIFF_CHANNEL0)) || (channel > HAL_ADC_DIFF_CHANNEL7))
    return -1;

  halAdcState = DATA;
  halStartAdc(channel);
  return 0;
}

/******************************************************************************
Closes the ADC.
Parameters:
  none.
Returns:
  -1  - the module was not opened to be used.
   0  - on success.
******************************************************************************/
int HAL_CloseAdc(void)
{
  if (IDLE == halAdcState)
    return -1;

  halAdcState = IDLE;
  halCloseAdc();
  return 0;
}

/******************************************************************************
 ADC interrupt handler.
******************************************************************************/
void halSigAdcHandler(AdcStates_t halAdcState_cyb)
{
  if (DATA == halAdcState_cyb)
  {
    halAdcState = BUSY;
    if (NULL != halAdcControl.callback)
      halAdcControl.callback();
  }
}
// eof adc.c

