/****************************************************************************

  Header file for template service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef BeaconFSM_H
#define BeaconFSM_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef union {
    struct {
        uint16_t LowByteRolloverCount;
        uint16_t HighByteRolloverCount;
    } ByBytes;
    uint32_t TotalTime;
} Period_32BitCount;

typedef enum
{
  InitPState, Waiting4Beacon, BeaconDetected
}BeaconState_t;

bool InitBeaconFSM(uint8_t Priority);
bool PostBeaconFSM(ES_Event_t ThisEvent);
ES_Event_t RunBeaconFSM(ES_Event_t ThisEvent);
BeaconState_t QueryBeaconFSM(void);
void TestHarnessBeacon_RisingEdge(void);
void TestHarnessBeacon_FallingEdge(void);

#endif /* BeaconService_H */

