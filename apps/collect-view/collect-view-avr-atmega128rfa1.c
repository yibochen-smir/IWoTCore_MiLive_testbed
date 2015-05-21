#include "collect-view.h"

#include "collect-view-avr-atmega128rfa1.h"

//#include "stdio.h"

/*---------------------------------------------------------------------------*/
void
collect_view_arch_read_sensors(struct collect_view_data_msg *msg)
{
#if iwotcore_CYB
	testAppGetSensorData();
#endif
	msg->sensors[BATTERY_VOLTAGE_SENSOR] = appMessage.data.scm_sensors.battery;
  	msg->sensors[LIGHT1_SENSOR] = appMessage.data.scm_sensors.light;
	if (appMessage.data.scm_sensors.temperature>0)
		msg->sensors[TEMP_SENSOR] = (uint16_t)appMessage.data.scm_sensors.temperature;
	else
		msg->sensors[TEMP_SENSOR] = 0;

#if iwotcore_CYB_nano
	msg->sensors[NANO_SENSOR] = (uint16_t)appMessage.data.scm_sensors.NanoMetric;
#endif
	msg->sensors[BATTERY_INDICATOR] = (uint16_t) (((100*powerleft)/powerInitial));

	//YIBO: Only for real-world deployment, power mode has essential meaning...
#if iwotcore_real_world_deployment	
	if(msg->sensors[BATTERY_VOLTAGE_SENSOR]>=572 && msg->sensors[BATTERY_VOLTAGE_SENSOR]<=980)
		appMessage.data.scm_sensors.powermode_yibo = 1;
	else if (msg->sensors[BATTERY_VOLTAGE_SENSOR]< 572 )
		appMessage.data.scm_sensors.powermode_yibo = 0;
	else if (msg->sensors[BATTERY_VOLTAGE_SENSOR]> 980 )
		appMessage.data.scm_sensors.powermode_yibo = 2;
	else
		appMessage.data.scm_sensors.powermode_yibo = 0;
#endif		
}
/*---------------------------------------------------------------------------*/


