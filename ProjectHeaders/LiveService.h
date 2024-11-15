/****************************************************************************

  Header file for Test Harness Service0
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef LiveService_H
#define LiveService_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"
#include "ES_Port.h" // needed for definition of REENTRANT
// Public Function Prototypes

bool InitLiveService(uint8_t Priority);
bool PostLiveService(ES_Event_t ThisEvent);
ES_Event_t RunLiveService(ES_Event_t ThisEvent);

#endif /* ServTemplate_H */
