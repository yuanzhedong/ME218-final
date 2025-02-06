/****************************************************************************

  Header file for template service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef RobotFSM_H
#define RobotFSM_H

#include "ES_Types.h"

// Public Function Prototypes

bool InitRobotFSM(uint8_t Priority);
bool PostRobotFSM(ES_Event_t ThisEvent);
ES_Event_t RunRobotFSM(ES_Event_t ThisEvent);

// States
typedef enum
{
  NormalMove = 0,
  BeaconAlign, 
  MovingToTape
}RobotState_t;
typedef enum
{
  FindingPeak = 0,//where the robot rotate 360 degree to find the peak
  AlignPeak,
  
}AlignSubState_t;
#define STOP_CMD 0x00
#define CW_90_CMD 0x02
#define CW_45_CMD 0x03
#define CCW_90_CMD 0x04
#define CCW_45_CMD 0x05
#define FWD_HALF_CMD 0x08
#define FWD_FULL_CMD 0x09
#define BWD_HALF_CMD 0x10
#define BWD_FULL_CMD 0x11
#define ALIGN_CMD 0x20
#define FIND_TAPE_CMD 0x40

#endif /* ServTemplate_H */

