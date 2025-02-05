#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Events.h"
#include "KeyboardService.h"
#include "PWMService.h"
#include "dbprintf.h"
#include "ES_Port.h"
#include "MyMotorService.h"

/*----------------------------- Module Defines ----------------------------*/
#define FORWARD 'w'
#define BACKWARD 's'
#define LEFT 'a'
#define RIGHT 'd'
#define STOP 'x'

/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitKeyboardService

****************************************************************************/
bool InitKeyboardService(uint8_t Priority)
{
    MyPriority = Priority;
    ES_Event_t ThisEvent;
    ThisEvent.EventType = ES_INIT;
    return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
     PostKeyboardService

****************************************************************************/
bool PostKeyboardService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunKeyboardService

****************************************************************************/
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
            case FORWARD:
                CurEvent.EventType = ES_FORWARD;
                CurEvent.EventParam = 100;
                PostMotorService(CurEvent);
                break;
            case BACKWARD:
                CurEvent.EventType = ES_BACKWARD;
                CurEvent.EventParam = 100;
                PostMotorService(CurEvent);
                break;
            case LEFT:
                CurEvent.EventType = ES_LEFT;
                CurEvent.EventParam = 100;
                PostMotorService(CurEvent);
                break;
            case RIGHT:
                CurEvent.EventType = ES_RIGHT;
                CurEvent.EventParam = 100;
                PostMotorService(CurEvent);
                break;
             case STOP:
                CurEvent.EventType = ES_STOP;
                PostMotorService(CurEvent);
                break;
                
            default:
                MotorCommand(0); // stop
                break;
        }
    }

    return ReturnEvent;
}