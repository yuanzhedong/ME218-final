/****************************************************************************

  Header file for Test Harness Service0
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef SendStringService_H
#define SendStringService_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"
#include "ES_Port.h"                // needed for definition of REENTRANT
// Public Function Prototypes

bool InitSendStringService(uint8_t Priority);
bool PostSendStringService(ES_Event_t ThisEvent);
ES_Event_t RunSendStringService(ES_Event_t ThisEvent);

#endif /* ServTemplate_H */

