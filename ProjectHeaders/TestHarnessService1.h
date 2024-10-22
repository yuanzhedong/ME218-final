/****************************************************************************

  Header file for Test Harness Service0
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef TestHarnessService1_H
#define TestHarnessService1_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"
#include "ES_Port.h"                // needed for definition of REENTRANT
// Public Function Prototypes

bool InitTestHarnessService1(uint8_t Priority);
bool PostTestHarnessService1(ES_Event_t ThisEvent);
ES_Event_t RunTestHarnessService0(ES_Event_t ThisEvent);

#endif /* ServTemplate_H */

