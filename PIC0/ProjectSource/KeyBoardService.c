#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Events.h"
#include "KeyboardService.h"
#include "dbprintf.h"
#include "ES_Port.h"
#include "PlannerHSM.h"
#include "SPIMasterService.h"

/*----------------------------- Module Defines ----------------------------*/
#define INIT_COMPLETE 'i'
#define HAS_CRATE 'p'
#define SIDE_DETECTED 's'
#define AT_COLUMN_INTERSECTION 'n'
#define PROCESS_COLUMN 'c'
#define AT_STACK 'g'
#define DROPPED 'd'
#define CHECK_ROBO_STATUS 'v'
#define AT_CRATE 't'
#define PICKUP_CRATE 'k'
#define COLUMN_COMPLETE 'o'

#define NAV_MOVE_FORWARD '1'
#define NAV_MOVE_BACKWARD '2'
#define NAV_TURN_LEFT '3'
#define NAV_TURN_RIGHT '4'
#define NAV_STOP '5'
#define NAV_TURN_180 '6'
#define NAV_TURN_360 '7'

/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;

/*------------------------------ Module Code ------------------------------*/
bool InitKeyboardService(uint8_t Priority)
{
    MyPriority = Priority;
    ES_Event_t ThisEvent;
    ThisEvent.EventType = ES_INIT;
    return ES_PostToService(MyPriority, ThisEvent);
}

bool PostKeyboardService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunKeyboardService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT;
    ES_Event_t CurEvent;

    if (ThisEvent.EventType == ES_NEW_KEY)
    {   
        char key = ThisEvent.EventParam;
        switch (key)
        {
            case INIT_COMPLETE:
                CurEvent.EventType = ES_INIT_COMPLETE;
                PostPlannerHSM(CurEvent);
                break;
            case HAS_CRATE:
                CurEvent.EventType = ES_HAS_CRATE;
                PostPlannerHSM(CurEvent);
                break;
            case SIDE_DETECTED:
                CurEvent.EventType = ES_SIDE_DETECTED;
                PostPlannerHSM(CurEvent);
                break;
            case AT_STACK:
                CurEvent.EventType = ES_AT_STACK;
                PostPlannerHSM(CurEvent);
                break;
            case DROPPED:
                CurEvent.EventType = ES_DROPPED;
                PostPlannerHSM(CurEvent);
                break;
            case AT_CRATE:
                CurEvent.EventType = ES_AT_CRATE;
                PostPlannerHSM(CurEvent);
                break;
            case PICKUP_CRATE:
                CurEvent.EventType = ES_HAS_CRATE;
                PostPlannerHSM(CurEvent);
                break;
            case AT_COLUMN_INTERSECTION:
                CurEvent.EventType = ES_AT_COLUMN_INTERSECTION;
                PostPlannerHSM(CurEvent);
                break;
            case COLUMN_COMPLETE:
                CurEvent.EventType = ES_COLUMN_COMPLETE;
                PostPlannerHSM(CurEvent);
                break;
            case NAV_MOVE_FORWARD:
                CurEvent.EventType = ES_NEW_NAV_CMD;
                CurEvent.EventParam = NAV_CMD_MOVE_FORWARD;
                PostSPIMasterService(CurEvent);
                break;
            case NAV_MOVE_BACKWARD:
                CurEvent.EventType = ES_NEW_NAV_CMD;
                CurEvent.EventParam = NAV_CMD_MOVE_BACKWARD;
                PostSPIMasterService(CurEvent);
                break;
            case NAV_TURN_LEFT:
                CurEvent.EventType = ES_NEW_NAV_CMD;
                CurEvent.EventParam = NAV_CMD_TURN_LEFT;
                PostSPIMasterService(CurEvent);
                break;
            case NAV_TURN_RIGHT:
                CurEvent.EventType = ES_NEW_NAV_CMD;
                CurEvent.EventParam = NAV_CMD_TURN_RIGHT;
                PostSPIMasterService(CurEvent);
                break;
            case NAV_STOP:
                CurEvent.EventType = ES_NEW_NAV_CMD;
                CurEvent.EventParam = NAV_CMD_STOP;
                PostSPIMasterService(CurEvent);
                break;
            case NAV_CMD_TURN_180:
                CurEvent.EventType = ES_NEW_NAV_CMD;
                CurEvent.EventParam = NAV_CMD_TURN_180;
                PostSPIMasterService(CurEvent);
                break;
            case NAV_CMD_TURN_360:
                CurEvent.EventType = ES_NEW_NAV_CMD;
                CurEvent.EventParam = NAV_CMD_TURN_360;
                PostSPIMasterService(CurEvent);
                break;
        
            default:
                return ReturnEvent;
        }
    }
    return ReturnEvent;
}
