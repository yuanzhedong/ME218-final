#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Events.h"
#include "KeyboardService.h"
#include "PWMService.h"
#include "dbprintf.h"
#include "ES_Port.h"

/*----------------------------- Module Defines ----------------------------*/
#define FORWARD 'w'
#define BACKWARD 's'
#define LEFT 'a'
#define RIGHT 'd'

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

    if (ThisEvent.EventType == ES_NEW_KEY)
    {
        char key = ThisEvent.EventParam;
        switch (key)
        {
            case FORWARD:
                MotorCommand(1); // forward full speed
                break;
            case BACKWARD:
                MotorCommand(3); // reverse full speed
                break;
            case LEFT:
                MotorCommand(6); // counter-clockwise 90 deg
                break;
            case RIGHT:
                MotorCommand(5); // clockwise 90 deg
                break;
            default:
                MotorCommand(0); // stop
                break;
        }
    }

    return ReturnEvent;
}