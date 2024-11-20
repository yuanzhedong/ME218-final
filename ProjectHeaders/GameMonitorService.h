#ifndef GAMEMONITORSERVICE_H
#define GAMEMONITORSERVICE_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    GIdle,
    GMonitor,
} GameMonitorState_t;

// Public Function Prototypes

bool InitGameMonitorService(uint8_t Priority);
bool PostGameMonitorService(ES_Event_t ThisEvent);
ES_Event_t RunGameMonitorService(ES_Event_t ThisEvent);
GameMonitorState_t QueryGameMonitorService(void);

#endif /* COINLEDSERVICE_H */
