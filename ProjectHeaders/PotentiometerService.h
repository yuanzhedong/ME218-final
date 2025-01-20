#ifndef POTENTIOMETER_SERVICE_H
#define POTENTIOMETER_SERVICE_H

#include "ES_Events.h"
#include "ES_Port.h" // needed for definition of REENTRANT

// Public Function Prototypes
uint8_t InitPotentiometerService(uint8_t Priority);
uint8_t PostPotentiometerService(ES_Event ThisEvent);
ES_Event RunPotentiometerService(ES_Event ThisEvent);

#endif /* POTENTIOMETER_SERVICE_H */