#ifndef DISPLAY_SERVICE_H
#define DISPLAY_SERVICE_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"


// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    InitDisplayState,
    DisplayLive,
} DisplayServiceState_t;

// Public Function Prototypes

bool InitDisplayService(uint8_t Priority);
bool PostDisplayService(ES_Event_t ThisEvent);
ES_Event_t RunDisplayService(ES_Event_t ThisEvent);
DisplayServiceState_t QueryDisplayService(void);

#endif /* SERVO_SERVICE_H */
