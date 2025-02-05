/****************************************************************************

  Header file for template service
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef MyMotorService_H
#define MyMotorService_H

#include <stdint.h>
#include <stdbool.h>

#include "ES_Events.h"
#include "ES_Port.h"   

typedef enum
{
    NOT_INIT,
    CWRot,
    CCWRot,
    Pause
}MotorState_t;

// Public Function Prototypes

bool InitMotorService(uint8_t Priority);
bool PostMotorService(ES_Event_t ThisEvent);
ES_Event_t RunMotorService(ES_Event_t ThisEvent);
void ConfigTimer2();
void ConfigPWM_OC1();
void ConfigPWM_OC3();
#endif /* ServTemplate_H */

