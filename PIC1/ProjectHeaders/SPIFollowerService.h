#ifndef SPIFOLLOWER_SERVICE_H
#define SPIFOLLOWER_SERVICE_H

#include "ES_Configure.h"
#include "ES_Framework.h"

#define SPI_TIMEOUT_MS 500

#define NAV_CMD_MOVE_FORWARD 0x01
#define NAV_CMD_MOVE_BACKWARD 0x02
#define NAV_CMD_TURN_LEFT 0x03
#define NAV_CMD_TURN_RIGHT 0x04
#define NAV_CMD_STOP 0x05
#define NAV_CMD_TURN_CW 0x06 // Clockwise 180 degrees
#define NAV_CMD_TURN_CCW 0x07 // Counter-clockwise 180 degrees
#define NAV_CMD_QUERY_STATUS 0x08
#define NAV_CMD_ALIGN 0x09

#define NAV_STATUS_OK 0x50
#define NAV_STATUS_AT_CROSS 0x51
#define NAV_STATUS_TURN_DONE 0x52
#define NAV_STATUS_ERROR 0x53
#define NAV_STATUS_TURN_LEFT 0x54
#define NAV_STATUS_TURN_RIGHT 0x55
#define NAV_STATUS_TURN_360 0x56
#define NAV_STATUS_IDLE 0x57 //87
#define NAV_STATUS_LINE_FOLLOW 0x58 //88
#define NAV_STATUS_ALIGN_TAPE 0x59
#define NAV_STATUS_CHECK_INTERSECTION 0x5A
#define NAV_STATUS_LINE_DISCOVER 0x5B
#define NAV_STATUS_CHECK_CRATE 0x5C
#define NAV_STATUS_INIT 0x5D



bool InitSPIFollowerService(uint8_t Priority);
bool PostSPIFollowerService(ES_Event_t ThisEvent);
ES_Event_t RunSPIFollowerService(ES_Event_t ThisEvent);

#endif // SPIFOLLOWER_SERVICE_H