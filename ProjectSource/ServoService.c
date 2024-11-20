#include "../ProjectHeaders/ServoService.h"

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
#include "../ProjectHeaders/PIC32_SPI_HAL.h"
#include "../ProjectHeaders/PWM_PIC32.h"

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

#define ONE_SEC 1000
#define HALF_SEC (ONE_SEC / 2)
#define QUATER_SEC (HALF_SEC / 2)

#define GAME_TIME 60 * ONE_SEC

#define PWM_OUTPUT LATBbits.LATB9

static ServoServiceState_t currentState = InitServoState;

static uint16_t maxPulseTicks = 6250; // +90
static uint16_t minPulseTicks = 1250; // -90
uint16_t currnetPulseTicks = 6250;
uint8_t currentStep = 0;
uint8_t lastPostLiveStep = 0;
static uint8_t maxStep = 60*1000 / QUATER_SEC ;

uint16_t step2Pulsetick(uint16_t currentStep)
{
    uint16_t currnetPulseTicks = (maxPulseTicks - minPulseTicks) * 1.0 / maxStep * currentStep  + 1250;
    //DB_printf("Current Pulse: %d\n", currnetPulseTicks);
    return currnetPulseTicks;
}

bool InitServoService(uint8_t Priority)
{
    ES_Event_t ThisEvent;

    MyPriority = Priority;

    // When doing testing, it is useful to announce just which program
    // is running.
    clrScrn();
    puts("\rStarting ServoService\r");
    DB_printf("compiled at %s on %s\n", __TIME__, __DATE__);
    DB_printf("\n\r\n");

    TRISBbits.TRISB9 = 0; // set RB9 as PMW output pin

    PWMSetup_BasicConfig(3); // Assuming channel 3 is available

    // Step 2: Assign Channel 3 to Timer 2
    PWMSetup_AssignChannelToTimer(3, _Timer2_); // Assuming Timer2 is suitable for this application

    // Step 3: Set PWM Period
    PWMSetup_SetPeriodOnTimer(50000, _Timer2_); // Adjust period based on desired frequency and TICS_PER_MS

    // Step 4: Map PWM Channel 3 to PWM_RPB9
    PWMSetup_MapChannelToOutputPin(3, PWM_RPB9);

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

bool PostServoService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunServoService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    switch (currentState)
    {
    case InitServoState: // If current state is initial Psedudo State
    {
        if (ThisEvent.EventType == ES_INIT) // only respond to ES_Init
        {
            puts("generating pules");
            // Step 5: Set Duty Cycle for Channel 3 to 50%
            // PWMOperate_SetDutyOnChannel(20, 3); // 50% duty cycle
            PWMOperate_SetPulseWidthOnChannel(maxPulseTicks, 3); // -90 degree
            //            for (volatile long i = 0; i < 5000000; ++i)
            //            {
            //                ;
            //            }
            //
            //            PWMOperate_SetPulseWidthOnChannel(5000, 3); // 45 degree/2ms
        }

        if (ThisEvent.EventType == ES_START_GAME)
        {
            PWMOperate_SetPulseWidthOnChannel(maxPulseTicks, 3); // 90 degree
            ES_Timer_InitTimer(SERVO_SERVICE_TIMER, QUATER_SEC);
            currentState = WaitForTarget;
            currentStep = 0;
            lastPostLiveStep = 0;
        }
    }
    break;

    case WaitForTarget:
    {
        if (ThisEvent.EventType == ES_NEW_KEY)
        {
            if ('w' == ThisEvent.EventParam)
            {
                if (currnetPulseTicks < maxPulseTicks)
                {
                    currnetPulseTicks += 100;
                    puts("Moving forward");
                    PWMOperate_SetPulseWidthOnChannel(currnetPulseTicks, 3);
                }
            }
            else
            {
                if ('s' == ThisEvent.EventParam)
                {
                    if (currnetPulseTicks > minPulseTicks)
                    {
                        currnetPulseTicks -= 100;
                        puts("Moving backward");
                        PWMOperate_SetPulseWidthOnChannel(currnetPulseTicks, 3);
                    }
                }
            }
        }

        // count down
        else if (ThisEvent.EventType == ES_TIMEOUT)
        {
            // restart timer
            ES_Timer_InitTimer(SERVO_SERVICE_TIMER, QUATER_SEC);
            ++currentStep;
            currnetPulseTicks = step2Pulsetick(currentStep);
            PWMOperate_SetPulseWidthOnChannel(currnetPulseTicks, 3);
            if (currentStep - lastPostLiveStep == 8) {
                ES_Event_t event = {ES_MINUS_LIVE, 1.0/32 * 100};
                PostLiveService(event);
                lastPostLiveStep = currentStep;
            }
            
            
            
            // for debug
            if (currentStep == maxStep / 4)
            {
                puts("15 seconds passed\n");
                //DB_printf("Current time: %d\n", ES_Timer_GetTime());
            }

            if (currentStep == maxStep / 2)
            {
                puts("30 seconds passed\n");
                //DB_printf("Current time: %d\n", ES_Timer_GetTime());
            }

            // end of game
            if (currentStep == maxStep)
            {
                puts("1 min reached, End Game!!\n");
                ES_Event_t ThisEvent;
                ThisEvent.EventType = ES_END_GAME;
                ES_PostAll(ThisEvent);
            }
            else
            { // restert the step timer
                ES_Timer_InitTimer(SERVO_SERVICE_TIMER, QUATER_SEC);
            }
            /* code */
        }
        else if (ThisEvent.EventType == ES_END_GAME)
        {
            currentState = InitServoState; // do I need to something here?
        }
    }

    break;
    // repeat state pattern as required for other states
    default:;
    } // end switch on Current State
    return ReturnEvent;
}

ServoServiceState_t QueryServoService(void)
{
    return currentState;
}
