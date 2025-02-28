/****************************************************************************

  Header file for Tape Flat Sate Machine
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef FSMTape_H
#define FSMTape_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
  Idle_tapeFSM, Following_tapeFSM,Looking4Tape_tapeFSM,
}TapeState_t;

// Public Function Prototypes

bool InitTapeFSM(uint8_t Priority);
bool PostTapeFSM(ES_Event_t ThisEvent);
ES_Event_t RunTapeFSM(ES_Event_t ThisEvent);
TapeState_t QueryTapeSM(void);

#endif /* FSMTape_H */

