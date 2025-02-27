/****************************************************************************

  Header file for Stepper service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef ServStepper_H
#define ServStepper_H

#include "ES_Types.h"

// Public Function Prototypes

bool InitStepperService(uint8_t Priority);
bool PostStepperService(ES_Event_t ThisEvent);
ES_Event_t RunStepperService(ES_Event_t ThisEvent);

#endif /* ServStepper_H */

