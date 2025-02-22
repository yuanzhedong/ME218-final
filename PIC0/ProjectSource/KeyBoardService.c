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
#define AT_COLUMN1_INTERSECTION 'n'
#define PROCESS_COLUMN 'c'
#define AT_STACK 'g'
#define DROPPED 'd'
#define UPDATE_PROGRESS1 'u'
#define UPDATE_PROGRESS2 'v'
#define AT_CRATE 't'
#define PICKUP_CRATE 'k'
#define AT_COLUMN2_INTERSECTION 'm'
#define COLUMN2_COMPLETE 'o'



#define SEND_NAV_CMD_MOVE '1'
#define SEND_NAV_CMD_TURN_LEFT '2'
#define SEND_NAV_CMD_TURN_RIGHT '3'
#define SEND_NAV_CMD_TURN_360 '4'
#define SEND_NAV_CMD_STOP '5'
#define SEND_NAV_CMD_QUERY '6'

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
                break;
            case HAS_CRATE:
                CurEvent.EventType = ES_HAS_CRATE;
                break;
            case SIDE_DETECTED:
                CurEvent.EventType = ES_SIDE_DETECTED;
                break;
            case AT_COLUMN1_INTERSECTION:
                CurEvent.EventType = ES_AT_COLUMN1_INTERSECTION;
                break;
            case AT_STACK:
                CurEvent.EventType = ES_AT_STACK;
                break;
            case DROPPED:
                CurEvent.EventType = ES_DROPPED;
                break;
            case AT_CRATE:
                CurEvent.EventType = ES_AT_CRATE;
                break;
            case PICKUP_CRATE:
                CurEvent.EventType = ES_HAS_CRATE;
                break;
            case AT_COLUMN2_INTERSECTION:
                CurEvent.EventType = ES_AT_COLUMN2_INTERSECTION;
                break;
            case COLUMN2_COMPLETE:
                CurEvent.EventType = ES_COLUMN2_COMPLETE;
                break;

            case SEND_NAV_CMD_MOVE:
                CurEvent.EventType = ES_NEW_NAV_CMD;
                CurEvent.EventParam = NAV_CMD_MOVE;
                PostSPIMasterService(CurEvent);
                break;

            default:
                return ReturnEvent;
        }
        
        PostPlannerHSM(CurEvent);
    }
    return ReturnEvent;
}
