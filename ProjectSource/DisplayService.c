#include "../ProjectHeaders/LEDService.h"
#include "../ProjectHeaders/DM_Display.h"
#include <string.h>
#include "../ProjectHeaders/DisplayService.h"

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

static uint8_t CurIdx = 0;
static char MESSAGE[] = "Bring the fuel back to the station, don't hit the debris !!!";
static uint8_t ScoreIdx = 0;
char SCORE[] = "SCORE: ";
static uint8_t FinalScore = 0;

#define ONE_SEC 1000
#define HALF_SEC (ONE_SEC / 2)
#define QUATER_SEC (HALF_SEC / 2)

static DisplayServiceState_t currentState = InitDisplayState;

bool InitDisplayService(uint8_t Priority)
{
    ES_Event_t ThisEvent;

    MyPriority = Priority;

    // When doing testing, it is useful to announce just which program
    // is running.
    clrScrn();
    puts("\rStarting DisplayService\r");
    DB_printf("compiled at %s on %s\n", __TIME__, __DATE__);
    DB_printf("\n\r\n");
    // post the initial transition event
    ThisEvent.EventType = ES_INIT;
    if (ES_PostToService(MyPriority, ThisEvent) == true)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool PostDisplayService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunDisplayService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    switch (currentState)
    {
    case InitDisplayState: // If current state is initial Psedudo State
    {
        switch (ThisEvent.EventType)
        {
        case ES_INIT:
        {
            ES_Timer_InitTimer(DISPLAY_SERVICE_TIMER, HALF_SEC);
        }
        break;
        case ES_TIMEOUT:
        {
            ES_Timer_InitTimer(DISPLAY_SERVICE_TIMER, HALF_SEC);
            ES_Event_t START_LED_WRITE = {ES_START_LED_WRITE, MESSAGE[CurIdx]};
            CurIdx += 1;
            if (CurIdx >= strlen(MESSAGE))
            {
                CurIdx = 0;
            }
            PostLEDService(START_LED_WRITE);
        }
        break;
        case ES_START_GAME:
        {
            currentState = DisplayLive;
            ES_Event_t event = {ES_START_LED_WRITE_LIVE, 32};
            PostLEDService(event);
        }
        break;
        default:;
        }
    }
    break;

    case DisplayLive:
    {
        switch (ThisEvent.EventType)
        {
        case ES_UPDATE_LIVE:
        {
            uint8_t currentLives = ThisEvent.EventParam;
            // map range 100 to range 32
            DB_printf("Current Lives level: %d\n", currentLives);
            uint8_t currentlevel = currentLives * 1.0 / 100 * 32;
            DB_printf("Current LED level: %d\n", currentlevel);
            ES_Event_t event = {ES_START_LED_WRITE_LIVE, currentlevel};
            PostLEDService(event);
        }
        break;
        case ES_UPDATE_SCORE: // go back to init state to display README
        {
            puts("update score...\n");
            currentState = DisplayScore;
            FinalScore = ThisEvent.EventParam;
            ES_Timer_InitTimer(DISPLAY_SERVICE_TIMER, QUATER_SEC);
            //ES_Event_t event = {ES_TIMEOUT, 0};
            //PostDisplayService(event);
        }
        break;
        default:;
        }
    }
    break;

    case DisplayScore:
    {
        switch (ThisEvent.EventType)
        {
        case ES_TIMEOUT: 
        {
            DB_printf("timer id %d\n", ThisEvent.EventParam);
            if (ThisEvent.EventParam == 12) {
                ES_Timer_InitTimer(DISPLAY_SERVICE_TIMER, QUATER_SEC);
                ES_Event_t START_LED_WRITE = {ES_START_LED_WRITE, SCORE[ScoreIdx]};
                PostLEDService(START_LED_WRITE);
                ScoreIdx += 1;
                puts("########\n");
                if (ScoreIdx >= strlen(SCORE))
                {
                    puts("########\n");
                    ES_Event_t START_LED_WRITE = {ES_START_LED_WRITE, FinalScore + '0'};
                    PostLEDService(START_LED_WRITE);
                    ES_Timer_InitTimer(DISPLAY_SERVICE_TIMER, ONE_SEC * 60);
                    ES_Timer_InitTimer(DISPLAY_SCORE_TIMER, ONE_SEC * 5);
                }
            }
            if (ThisEvent.EventParam == 9) {
                currentState = InitDisplayState;
                ES_Event_t event = {ES_INIT, 0};
                PostDisplayService(event);
                ScoreIdx = 0;                
            }
                        
        }
        break;
        default:;
        }
    }

    break;
    // repeat state pattern as required for other states
    default:;
    } // end switch on Current State
    return ReturnEvent;
}

DisplayServiceState_t QueryDisplayService(void)
{
    return currentState;
}
