#ifndef WEIGHT_SENSOR_H
#define WEIGHT_SENSOR_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"

// Public Function Prototypes

bool InitWeightSensor(uint8_t Priority);
bool PostWeightSensor(ES_Event_t ThisEvent);
ES_Event_t RunWeightSensor(ES_Event_t ThisEvent);

#endif /* WEIGHT_SENSOR_H */
