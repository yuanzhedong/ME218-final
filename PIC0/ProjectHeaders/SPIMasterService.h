#ifndef SPIMASTER_SERVICE_H
#define SPIMASTER_SERVICE_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"
#include "ES_Port.h"

#define SPI_TIMEOUT_MS 500

#define NAV_CMD_MOVE_FORWARD 0x10
#define NAV_CMD_MOVE_BACKWARD 0x20
#define NAV_CMD_TURN_LEFT 0x30
#define NAV_CMD_TURN_RIGHT 0x40
#define NAV_CMD_STOP 0x50
#define NAV_CMD_TURN_CW 0x60 // Clockwise 180 degrees
#define NAV_CMD_TURN_CCW 0x70 // Counter-clockwise 180 degrees
#define NAV_CMD_QUERY_STATUS 0x80
#define NAV_CMD_ALIGN 0x90

#define NAV_STATUS_MOVE_FORWARD_COMPLETE 0x11
#define NAV_STATUS_MOVE_BACKWARD_COMPLETE 0x21
#define NAV_STATUS_TURN_LEFT_COMPLETE 0x31
#define NAV_STATUS_TURN_RIGHT_COMPLETE 0x41
#define NAV_STATUS_STOP_COMPLETE 0x51
#define NAV_STATUS_TURN_CW_COMPLETE 0x61
#define NAV_STATUS_TURN_CCW_COMPLETE 0x71
#define NAV_STATUS_QUERY_COMPLETE 0x81
#define NAV_STATUS_ALIGN_COMPLETE 0x91

#define NAV_STATUS_OK 0x50
#define NAV_STATUS_AT_CROSS 0x51
#define NAV_STATUS_ERROR 0x53
#define NAV_STATUS_TURN_LEFT 0x54
#define NAV_STATUS_TURN_RIGHT 0x55
#define NAV_STATUS_TURN_360 0x56
#define NAV_STATUS_IDLE 0x57
#define NAV_STATUS_LINE_FOLLOW 0x58
#define NAV_STATUS_ALIGN_TAPE 0x59
#define NAV_STATUS_CHECK_INTERSECTION 0x5A
#define NAV_STATUS_LINE_DISCOVER 0x5B
#define NAV_STATUS_CHECK_CRATE 0x5C
#define NAV_STATUS_INIT 0x5D

bool InitSPIMasterService(uint8_t Priority);
bool PostSPIMasterService(ES_Event_t ThisEvent);
ES_Event_t RunSPIMasterService(ES_Event_t ThisEvent);

const char* TranslateNavCmdToStr(uint8_t command);
const char* TranslateNavStatusToStr(uint8_t status);

#endif // SPIMASTER_SERVICE_H