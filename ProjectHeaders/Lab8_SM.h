/****************************************************************************

  Header file for template service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef Lab8_SM_H
#define Lab8_SM_H

#include "ES_Types.h"

// Public Function Prototypes

bool InitLab8_SM(uint8_t Priority);
bool PostLab8_SM(ES_Event_t ThisEvent);
ES_Event_t RunLab8_SM(ES_Event_t ThisEvent);

// States
typedef enum
{
  Generating = 0,
  Aligning, 
  Aligned,
  Approaching,
  Done
}Lab8State_t;

#define STOP_CMD 0x00
#define CW_90_CMD 0x02
#define CW_45_CMD 0x03
#define CCW_90_CMD 0x04
#define CCW_45_CMD 0x05
#define FWD_FULL_CMD 0x08
#define FWD_HALF_CMD 0x09
#define BWD_FULL_CMD 0x10
#define BWD_HALF_CMD 0x11
#define ALIGN_CMD 0x20
#define FIND_TAPE_CMD 0x40

#endif /* ServTemplate_H */

