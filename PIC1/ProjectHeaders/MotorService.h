/****************************************************************************

  Header file for Motor service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef ServMotor_H
#define ServMotor_H

#include "ES_Types.h"

// Public Function Prototypes

bool InitMotorService(uint8_t Priority);
bool PostMotorService(ES_Event_t ThisEvent);
ES_Event_t RunMotorService(ES_Event_t ThisEvent);

#endif /* ServMotor_H */

