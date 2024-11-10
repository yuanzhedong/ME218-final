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

#define PWM_OUTPUT LATBbits.LATB9

static ServoServiceState_t currentState = InitPState;

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

    // Step 3: Set PWM Period (for a frequency of 1 kHz, example value)
    PWMSetup_SetPeriodOnTimer(40000, _Timer2_); // Adjust period based on desired frequency and TICS_PER_MS

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
    case InitPState: // If current state is initial Psedudo State
    {
        if (ThisEvent.EventType == ES_INIT) // only respond to ES_Init
        {
            puts("generating puls");
            // Step 5: Set Duty Cycle for Channel 3 to 50%
            PWMOperate_SetDutyOnChannel(50, 3); // 50% duty cycle
        }
    }

    break;
    // repeat state pattern as required for other states
    default:;
    } // end switch on Current State
    return ReturnEvent;
}

CoinLEDServiceState_t QueryCoinLEDService(void)
{
    return currentState;
}
