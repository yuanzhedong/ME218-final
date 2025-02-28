/****************************************************************************
 Header file for ServoService

 ****************************************************************************/

#ifndef SERVO_SERVICE_H
#define SERVO_SERVICE_H

// Include standard libraries
#include <stdint.h>
#include <stdbool.h>

// Include the Event & Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"

// State definitions for use with the query function
// Beacon type definition
typedef enum {
    BEACON_G,
    BEACON_B,
    BEACON_R,
    BEACON_L,
    BEACON_UNKNOWN
} Beacon_t;

// Public Function Prototypes
bool InitServoService(uint8_t Priority);
bool PostServoService(ES_Event_t ThisEvent);
ES_Event_t RunServoService(ES_Event_t ThisEvent);
//ServoServiceState_t QueryServoService(void);

#endif /* SERVO_SERVICE_H */
