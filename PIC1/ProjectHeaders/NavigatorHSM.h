/****************************************************************************
 Module
   NavigatorHSM.h

 Revision
   1.0.1

 Description
   Header file for the Navigator state machine.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 10/10/23       GitHub Copilot  Initial implementation
****************************************************************************/

#ifndef NavigatorHSM_H
#define NavigatorHSM_H

// Include standard libraries
#include <stdint.h>
#include <stdbool.h>

// Include the Event Framework
#include "ES_Events.h"

// Public Function Prototypes

// Define the states for the state machine
typedef enum {
    Init,
    Idle,
    LineFollowForward,
    LineFollowBackward,
    AlignBeacon,
    CheckIntersection,
    TurnLeft,
    TurnRight,
    LineDiscover,
    CheckCrate
} NavigatorState_t;

bool InitNavigatorHSM(uint8_t Priority);
bool PostNavigatorHSM(ES_Event_t ThisEvent);
ES_Event_t RunNavigatorHSM(ES_Event_t ThisEvent);
void StartNavigatorHSM(ES_Event_t ThisEvent);
NavigatorState_t QueryNavigatorHSM(void);

#endif /* NavigatorHSM_H */