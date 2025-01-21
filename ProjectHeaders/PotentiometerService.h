#ifndef POTENTIOMETER_SERVICE_H
#define POTENTIOMETER_SERVICE_H

#include "ES_Events.h"
#include "ES_Port.h"

// Public Function Prototypes
uint8_t InitPotentiometerService(uint8_t Priority);
uint8_t PostPotentiometerService(ES_Event_t ThisEvent);
ES_Event_t RunPotentiometerService(ES_Event_t ThisEvent);

#endif /* POTENTIOMETER_SERVICE_H */