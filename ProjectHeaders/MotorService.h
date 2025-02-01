#ifndef MOTORSERVICE_H
#define MOTORSERVICE_H

#include "ES_Configure.h"
#include "ES_Types.h"

// Public Function Prototypes
bool InitMotorService(uint8_t Priority);
bool PostMotorService(ES_Event_t ThisEvent);
ES_Event_t RunMotorService(ES_Event_t ThisEvent);

#endif /* MOTORSERVICE_H */