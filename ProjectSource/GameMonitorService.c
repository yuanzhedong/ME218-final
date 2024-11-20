#include "../ProjectHeaders/GameMonitorService.h"

// Hardware
#include <xc.h>
// #include <proc/p32mx170f256b.h>

// Event & Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_Port.h"
#include "terminal.h"
#include "dbprintf.h"

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;


#define ONE_SEC 1000
#define TWENTY_SEC 20 * ONE_SEC

static GameMonitorState_t currentState = GIdle; // init as idle state

bool InitGameMonitorService(uint8_t Priority)
{
    
    ES_Event_t ThisEvent;

    MyPriority = Priority;

    // When doing testing, it is useful to announce just which program
    // is running.
    clrScrn();
    puts("\rStarting GameMonitor\r");
    DB_printf("compiled at %s on %s\n", __TIME__, __DATE__);
    DB_printf("\n\r\n");

    return true;
    // post the initial transition event
    // ThisEvent.EventType = ES_INIT;
    // if (ES_PostToService(MyPriority, ThisEvent) == true)
    // {
    //     return true;
    // }
    // else
    // {
    //     return false;
    // }
}

bool PostGameMonitorService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunGameMonitorService(ES_Event_t ThisEvent)
{
    switch (currentState)
    {
    case GIdle:
    {
        if (ThisEvent.EventType == ES_START_GAME)
        {
            currentState = GMonitor;
            ES_Timer_InitTimer(GAME_MONITOR_SERVICE_TIMER, TWENTY_SEC);
        }
    }
    break;
    case GMonitor:
    {
        if (ThisEvent.EventType == ES_START_GAME) { //reset the time when game start
            ES_Timer_InitTimer(GAME_MONITOR_SERVICE_TIMER, TWENTY_SEC);
        }
        if (ThisEvent.EventType == ES_TIMEOUT)
        {
            puts("*******");
            ES_Event_t event = {ES_END_GAME, 0};
            ES_PostAll(event);
            currentState = GIdle;
        }
        if (ThisEvent.EventType == ES_RESET_GAME_MONITOR)
        {
            puts("000000\n");
            ES_Timer_InitTimer(GAME_MONITOR_SERVICE_TIMER, TWENTY_SEC);
            //puts("Reset monitor...\n");
        }
    }
    break;

    default:
        break;
    }
}
