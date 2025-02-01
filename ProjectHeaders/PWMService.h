/****************************************************************************

  Header file for PWM Service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef PWMService_H
#define PWMService_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"
#include "ES_Port.h"                // needed for definition of REENTRANT

// Public Function Prototypes

bool InitPWMService(uint8_t Priority);
bool PostPWMService(ES_Event_t ThisEvent);
ES_Event_t RunPWMService(ES_Event_t ThisEvent);

#endif /* PWMService_H */