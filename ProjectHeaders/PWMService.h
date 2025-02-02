#ifndef MOTORSERVICE_H
#define MOTORSERVICE_H

#include "ES_Configure.h"
#include "ES_Types.h"

// Public Function Prototypes
bool InitPWMService(uint8_t Priority);
bool PostPWMService(ES_Event_t ThisEvent);
ES_Event_t RunPWMService(ES_Event_t ThisEvent);

void MotorCommand(uint16_t MotorSetting);


#define M_STOP 0
#define M_FWD_FS 1
#define M_FWD_HS 2
#define M_BWD_FS 3
#define M_BWD_HS 4
#define M_CW_90 5
#define M_CCW_90 6
#define M_CW_45 7
#define M_CCW_45 8
#define M_SPIN 9

#endif /* MOTORSERVICE_H */