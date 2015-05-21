#ifndef COLLECT_VIEW_AVR_ATMEGA128RFA1_H
#define COLLECT_VIEW_AVR_ATMEGA128RFA1_H

#include "collect-view.h"

enum {
  BATTERY_VOLTAGE_SENSOR,
  LIGHT1_SENSOR,
  TEMP_SENSOR,
  NANO_SENSOR,
  BATTERY_INDICATOR,
  SOLAR_PANEL_SENSOR,
  RSSI_SENSOR,
  ETX1_SENSOR,
};

extern uint8_t powermode;
extern uint32_t powerleft;
extern uint32_t powerInitial;


#endif /* COLLECT_VIEW_AVR_ATMEGA128RFA1_H */

