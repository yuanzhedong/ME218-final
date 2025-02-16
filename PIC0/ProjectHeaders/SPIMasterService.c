#ifndef SPIMASTER_SERVICE_H
#define SPIMASTER_SERVICE_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"
#include "ES_Port.h"


typedef enum {
    NAV_CMD_MOVE = 0x01,
    NAV_CMD_TURN_LEFT = 0x02,
    NAV_CMD_TURN_RIGHT = 0x03,
    NAV_CMD_STOP = 0x04,
    NAV_CMD_TURN_360 = 0x05,
    NAV_QUERY_STATUS = 0x06,
} NavCommand_t;


bool InitSPIMasterService(uint8_t Priority);
bool PostSPIMasterService(ES_Event_t ThisEvent);
ES_Event_t RunSPIMasterService(ES_Event_t ThisEvent);

#endif // SPIMASTER_SERVICE_H