#ifndef SPIMASTER_SERVICE_H
#define SPIMASTER_SERVICE_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"
#include "ES_Port.h"

bool InitSPIMasterService(uint8_t Priority);
bool PostSPIMasterService(ES_Event_t ThisEvent);
ES_Event_t RunSPIMasterService(ES_Event_t ThisEvent);

#endif // SPIMASTER_SERVICE_H