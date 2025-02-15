#ifndef SPIFOLLOWER_SERVICE_H
#define SPIFOLLOWER_SERVICE_H

#include "ES_Configure.h"
#include "ES_Framework.h"

bool InitSPIFollowerService(uint8_t Priority);
bool PostSPIFollowerService(ES_Event_t ThisEvent);
ES_Event_t RunSPIFollowerService(ES_Event_t ThisEvent);

#endif // SPIFOLLOWER_SERVICE_H