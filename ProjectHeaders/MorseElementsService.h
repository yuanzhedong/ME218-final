/****************************************************************************

  Header file for Test Harness Service0
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef MorseElementsService_H
#define MorseElementsService_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"
#include "ES_Port.h"                // needed for definition of REENTRANT
// Public Function Prototypes

typedef enum
{
  InitPState, CalWaitForRise, CalWaitForFall,EOC_WaitRise
}MorseElementState_t;


bool InitMorseElementsService(uint8_t Priority);
bool PostMorseElementsService(ES_Event_t ThisEvent);
ES_Event_t RunMorseElementsService(ES_Event_t ThisEvent);
void TestCalibration();
#endif /* ServTemplate_H */

