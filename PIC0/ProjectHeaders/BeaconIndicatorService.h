#ifndef BEACON_SERVICE_H
#define BEACON_SERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include "ES_Configure.h"
#include "ES_Framework.h"

//typedef enum { BEACON_G, BEACON_B, BEACON_R, BEACON_L, BEACON_UNKNOWN } Beacon_t;

bool InitBeaconIndicatorService(uint8_t Priority);
bool PostBeaconIndicatorService(ES_Event_t ThisEvent);
ES_Event_t RunBeaconIndicatorService(ES_Event_t ThisEvent);

void ConfigTimer2(void);
void ConfigPWM_OC1(void);
void ConfigPWM_OC3(void);
void ConfigTimer3(void);
void Config_IC2(void);

#endif // MOTOR_SERVICE_H
