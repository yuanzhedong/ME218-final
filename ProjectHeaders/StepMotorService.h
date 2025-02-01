#ifndef STEPMOTORSERVICE_H
#define STEPMOTORSERVICE_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"
#include "ES_Port.h"                // needed for definition of REENTRANT

#include "PWM_PIC32.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
    InitPState,
    WaitForSpeed,
    Pause,
} StepMotorServiceState_t;

// Public Function Prototypes

bool InitStepMotorService(uint8_t Priority);
bool PostStepMotorService(ES_Event_t ThisEvent);
ES_Event_t RunStepMotorService(ES_Event_t ThisEvent);
StepMotorServiceState_t QueryStepMotorService(void);

#endif /* STEPMOTORSERVICE_H */
