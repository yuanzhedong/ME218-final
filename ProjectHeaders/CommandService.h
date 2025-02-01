/****************************************************************************

  Header file for template service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef CommandService_H
#define CommandService_H

#include "ES_Types.h"

// Public Function Prototypes

bool InitCommandService(uint8_t Priority);
bool PostCommandService(ES_Event_t ThisEvent);
ES_Event_t RunCommandService(ES_Event_t ThisEvent);

#endif /* ServTemplate_H */

