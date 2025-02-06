/****************************************************************************

  Header file for lab8_main Flat Sate Machine
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef FSMlab8_main_H
#define FSMlab8_main_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
  InitPState,StopMotion,NormalMove, BeaconAlign,
}lab8_mainState_t;

// Public Function Prototypes

bool Initlab8_mainFSM(uint8_t Priority);
bool Postlab8_mainFSM(ES_Event_t ThisEvent);
ES_Event_t Runlab8_mainFSM(ES_Event_t ThisEvent);
lab8_mainState_t Querylab8_mainSM(void);

#endif /* FSMlab8_main_H */

