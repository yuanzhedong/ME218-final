#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Events.h"
#include "KeyboardService.h"
#include "dbprintf.h"
#include "ES_Port.h"
#include "PlannerHSM.h"

/*----------------------------- Module Defines ----------------------------*/
#define INIT 'i'
#define SEARCH_PICKUP 'p'
#define SIDE_DETECTION 's'
#define NAVIGATE_COLUMN1 'n'
#define PROCESS_COLUMN 'c'
#define GO_TO_STACK 'g'
#define DROP_CRATE 'd'
#define UPDATE_PROGRESS1 'u'
#define UPDATE_PROGRESS2 'v'
#define GO_TO_CRATE 't'
#define PICKUP_CRATE 'k'
#define NAVIGATE_COLUMN2 'm'
#define GAME_OVER 'o'

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
            case INIT:
                CurEvent.EventType = ES_INIT_COMPLETE;
                break;
            case SEARCH_PICKUP:
                CurEvent.EventType = ES_HAS_CRATE;
                break;
            case SIDE_DETECTION:
                CurEvent.EventType = ES_SIDE_DETECTED;
                break;
            case NAVIGATE_COLUMN1:
                CurEvent.EventType = ES_AT_COLUMN1_INTERSECTION;
                break;
            case GO_TO_STACK:
                CurEvent.EventType = ES_AT_STACK;
                break;
            case DROP_CRATE:
                CurEvent.EventType = ES_DROPPED;
                break;
            case GO_TO_CRATE:
                CurEvent.EventType = ES_AT_CRATE;
                break;
            case PICKUP_CRATE:
                CurEvent.EventType = ES_HAS_CRATE;
                break;
            case NAVIGATE_COLUMN2:
                CurEvent.EventType = ES_AT_COLUMN2_INTERSECTION;
                break;
            case GAME_OVER:
                CurEvent.EventType = ES_COLUMN2_COMPLETE;
                break;
            default:
                return ReturnEvent;
        }
        
        DB_printf("Posting Event: %d from Keyboard Input\r\n", CurEvent.EventType);
        PostPlannerHSM(CurEvent);
    }
    return ReturnEvent;
}
