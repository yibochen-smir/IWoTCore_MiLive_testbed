#include "scm_decagon.h"
#include "iwotcore_gpio.h"

decagonStates_t decagonState = DECA_IDLE; // Monitors current state


#define DE_READ_MAX_CNT 10
uint8_t  decagonReadCnt;
uint16_t decagonDataSum;
decagonControl_t decagonControl;
void decagonADCCallback(void);

static HAL_AdcParams_t decagon_adcParam;
static uint16_t decagonAdcData;

void Init_Pin_ADC3(void)
{
	TRX_CTRL_1 &= 0x7F; //disables pin DIG3 and pin DIG4

	DDRF  &= 0xF7;		//configure Pin F3 as input
	PORTF &= 0xF7;		//disable Pin F3 pull-up
}

void decagon_OpenAdc(void)
{
    decagon_adcParam.bufferPointer = &decagonAdcData;
    decagon_adcParam.callback = decagonADCCallback;
    decagon_adcParam.resolution = RESOLUTION_10_BIT;
    decagon_adcParam.sampleRate = ADC_4800SPS;
    decagon_adcParam.selectionsAmount = 1;
    decagon_adcParam.voltageReference = INTERNAL_2d56V; //Internal 1.6V Voltage Reference
    HAL_OpenAdc(&decagon_adcParam);
}

/******************************************************************************
	Callabck about ADC request was completed.
	Parameters:
  		data - result of ADC.
	Returns:
 	 	none.
******************************************************************************/
void decagonADCCallback()
{
	decagonDataSum += decagonAdcData;
	decagonReadCnt ++;
	switch(decagonState)
	{
	case DECA_DATA0:
		decagonControl.decagonData0 = decagonAdcData;
		//bspPostTask(BSP_DECAGON);
		bspdecagonHandler(DECA_DATA0);
		break;

	case DECA_DATA1:
		decagonControl.decagonData1 = decagonAdcData;
		//bspPostTask(BSP_DECAGON);
		bspdecagonHandler(DECA_DATA1);
		break;

	case DECA_DATA2:
		decagonControl.decagonData2 = decagonAdcData;
		//bspPostTask(BSP_DECAGON);
		break;

	case DECA_DATA3:
		decagonControl.decagonData3 = decagonAdcData;
		//bspPostTask(BSP_DECAGON);
		break;

	case DECA_SIS_CUR:
		decagonControl.siscurData = decagonAdcData;
		//bspPostTask(BSP_DECAGON);
		break;

	default:
		break;
	}
}

/******************************************************************************
  	Opens the component to use.
  	Returns:
    		SUCCESS - the component is ready to been use.
    		FAIL - otherwise.
******************************************************************************/
result_t opendecagon()
{
  	Init_Pin_ADC3();

  	if (decagonState == DECA_IDLE)
  	{
    	decagonState = DECA_BUSY;
		//HAL_OpenAdc(ADC_DIVISION_FACTOR_128, decagonADCCallback);
		decagon_OpenAdc();
    	return BC_SUCCESS;
  	}
  	return BC_FAIL;
}

/******************************************************************************
  	Closes component.
  	Returns:
    		SUCCESS - always.
******************************************************************************/
result_t closedecagon()
{
  	if (decagonState == DECA_IDLE)
    	return BC_FAIL;
  	decagonState = DECA_IDLE;
  	HAL_CloseAdc();
  	return BC_SUCCESS;
}

/******************************************************************************
  	Starts ADC request on decagon channel.
  	Parameters:
    		callback   callback.
  	Returns:
    		FAIL - decagon component was not opened.
    		SUCCESS - other case.
******************************************************************************/
result_t readdecagonData(F_decagon_Callback callback)
{
  	if (decagonState != DECA_BUSY)
    	return BC_FAIL;
  	decagonState = DECA_DATA0;
  	decagonReadCnt = 0;
  	decagonDataSum = 0;
  	decagonControl.decagonCallback = callback;
  	HAL_ReadAdc(HAL_ADC_CHANNEL0);
	//bspdecagonHandler(decagonState);
	//HAL_ReadAdc(HAL_ADC_CHANNEL1);

	
  	return BC_SUCCESS;
}


/******************************************************************************
  	Starts ADC request on decagon channel.
  	Parameters:
    		callback   callback.
  	Returns:
    		FAIL - decagon component was not opened.
    		SUCCESS - other case.
******************************************************************************/
result_t readSISCurData(F_decagon_Callback callback)
{
  	if (decagonState != DECA_BUSY)
    	return BC_FAIL;
  	decagonState = DECA_SIS_CUR;
  	decagonReadCnt = 0;
  	decagonDataSum = 0;
  	decagonControl.decagonCallback = callback;
  	HAL_ReadAdc(HAL_ADC_CHANNEL3);
  	return BC_SUCCESS;
}

/******************************************************************************
BSP decagon handler.
******************************************************************************/
void bspdecagonHandler(decagonStates_t decagonState_cyb)
{
	switch(decagonState_cyb)
	{
		case DECA_DATA0:
			HAL_CloseAdc();
			//HAL_OpenAdc(ADC_DIVISION_FACTOR_128, decagonADCCallback);
			decagon_OpenAdc();
			if(decagonReadCnt >= DE_READ_MAX_CNT)
			{
				decagonControl.decagonData0 = (decagonDataSum+decagonReadCnt/2)/decagonReadCnt;
				decagonState = DECA_DATA1;
				decagonReadCnt = 0;
				decagonDataSum = 0;
				HAL_ReadAdc(HAL_ADC_CHANNEL1);
			}
			else
			{
				HAL_ReadAdc(HAL_ADC_CHANNEL0);
			}
			break;

		case DECA_DATA1:
			HAL_CloseAdc();
			//HAL_OpenAdc(ADC_DIVISION_FACTOR_128, decagonADCCallback);
			decagon_OpenAdc();
			if(decagonReadCnt >= DE_READ_MAX_CNT)
			{
				//IWOTCORE_USnd_Select(0);
				decagonControl.decagonData1 = (decagonDataSum+decagonReadCnt/2)/decagonReadCnt;
				decagonState = DECA_DATA2; //DECA_DATA2_CH0;
				decagonReadCnt = 0;
				decagonDataSum = 0;
				HAL_ReadAdc(HAL_ADC_CHANNEL2);
			}
			else
			{
				HAL_ReadAdc(HAL_ADC_CHANNEL1);
				decagonControl.decagonCallback(&decagonControl);				
			}
			break;

		case DECA_DATA2:
			HAL_CloseAdc();
			//HAL_OpenAdc(ADC_DIVISION_FACTOR_128, decagonADCCallback);
			decagon_OpenAdc();
			if(decagonReadCnt >= DE_READ_MAX_CNT)
			{
				decagonControl.decagonData2 = (decagonDataSum+decagonReadCnt/2)/decagonReadCnt;
				decagonReadCnt = 0;
				decagonDataSum = 0;
				decagonState = DECA_DATA3;
				HAL_ReadAdc(HAL_ADC_CHANNEL3);
			}
			else
			{
				HAL_ReadAdc(HAL_ADC_CHANNEL2);
			}
			break;

		case DECA_DATA3:
			if(decagonReadCnt >= DE_READ_MAX_CNT)
			{
				decagonControl.decagonData3 = (decagonDataSum+decagonReadCnt/2)/decagonReadCnt;
				decagonReadCnt = 0;
				decagonDataSum = 0;
				decagonState = DECA_BUSY;
				decagonControl.decagonCallback(&decagonControl);
			}
			else
			{
				HAL_CloseAdc();
				//HAL_OpenAdc(ADC_DIVISION_FACTOR_128, decagonADCCallback);
				decagon_OpenAdc();
				HAL_ReadAdc(HAL_ADC_CHANNEL3);
			}
			break;

		case DECA_SIS_CUR:
			if(decagonReadCnt >= DE_READ_MAX_CNT)
			{
				decagonControl.siscurData = (decagonDataSum+decagonReadCnt/2)/decagonReadCnt;
				decagonReadCnt = 0;
				decagonDataSum = 0;
				decagonState = DECA_BUSY;
				decagonControl.decagonCallback(&decagonControl);
			}
			else
			{
				HAL_CloseAdc();
				//HAL_OpenAdc(ADC_DIVISION_FACTOR_128, decagonADCCallback);
				decagon_OpenAdc();
				HAL_ReadAdc(HAL_ADC_CHANNEL3);
			}
			break;

		default:
			break;
	}
}

