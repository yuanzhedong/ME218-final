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
#define FORWARDHALF 'e'
#define BACKWARD 's'
#define BACKWARDHALF 'f'
#define LEFT90 'a'
#define LEFT45 'z'
#define RIGHT90 'd'
#define RIGHT45 'c'
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
                CurEvent.EventType = ES_FWDFULL;
                CurEvent.EventParam = 100;
                PostMotorService(CurEvent);
                break;
            case FORWARDHALF:
                CurEvent.EventType = ES_FWDHALF;
                CurEvent.EventParam = 50;
                PostMotorService(CurEvent);
                break;
            case BACKWARD:
                CurEvent.EventType = ES_BWDFULL;
                CurEvent.EventParam = 100;
                PostMotorService(CurEvent);
                break;
            case BACKWARDHALF:
                CurEvent.EventType = ES_BWDHALF;
                CurEvent.EventParam = 50;
                PostMotorService(CurEvent);
                break;
            case LEFT90:
                CurEvent.EventType = ES_LEFT90;
                CurEvent.EventParam = 100;
                PostMotorService(CurEvent);
                break;
            case LEFT45:
                CurEvent.EventType = ES_LEFT45;
                CurEvent.EventParam = 100;
                PostMotorService(CurEvent);
                break;
            case RIGHT90:
                CurEvent.EventType = ES_RIGHT90;
                CurEvent.EventParam = 100;
                PostMotorService(CurEvent);
                break;
            case RIGHT45:
                CurEvent.EventType = ES_RIGHT45;
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