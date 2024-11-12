#ifndef SERVO_SERVICE_H
#define SERVO_SERVICE_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"


// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    InitServoState,
    WaitForTarget,
    Moving
} ServoServiceState_t;

// Public Function Prototypes

bool InitServoService(uint8_t Priority);
bool PostServoService(ES_Event_t ThisEvent);
ES_Event_t RunServoService(ES_Event_t ThisEvent);
ServoServiceState_t QueryServoService(void);

#endif /* SERVO_SERVICE_H */
