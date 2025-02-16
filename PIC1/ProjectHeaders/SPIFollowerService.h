#ifndef SPIFOLLOWER_SERVICE_H
#define SPIFOLLOWER_SERVICE_H

#include "ES_Configure.h"
#include "ES_Framework.h"

#define SPI_TIMEOUT_MS 500

typedef enum {
    NAV_CMD_MOVE = 0x01,
    NAV_CMD_TURN_LEFT = 0x02,
    NAV_CMD_TURN_RIGHT = 0x03,
    NAV_CMD_STOP = 0x04,
    NAV_CMD_TURN_360 = 0x05,
    NAV_CMD_QUERY_STATUS = 0x06,
} NavCommand_t;

typedef enum {
    NAV_STATUS_OK = 0x10,
    NAV_STATUS_AT_CROSS = 0x11,
    NAV_STATUS_TURN_DONE = 0x12,
    NAV_STATUS_ERROR = 0x13,
    NAV_STATUS_TURN_LEFT = 0x14,
    NAV_STATUS_TURN_RIGHT = 0x15,
    NAV_STATUS_TURN_360 = 0x16,
    NAV_STATUS_IDLE = 0x17,
    NAV_STATUS_LINE_FOLLOW = 0x18,
    NAV_STATUS_ALIGN_BEACON = 0x19,
    NAV_STATUS_CHECK_INTERSECTION = 0x1A,
    NAV_STATUS_LINE_DISCOVER = 0x1B,
    NAV_STATUS_CHECK_CRATE = 0x1C,
    NAV_STATUS_INIT = 0x1D,
} NavStatus_t;

bool InitSPIFollowerService(uint8_t Priority);
bool PostSPIFollowerService(ES_Event_t ThisEvent);
ES_Event_t RunSPIFollowerService(ES_Event_t ThisEvent);

#endif // SPIFOLLOWER_SERVICE_H