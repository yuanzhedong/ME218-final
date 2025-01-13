#include "../ProjectHeaders/StepMotorService.h"

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

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

static uint16_t step_interval = 5;
static uint16_t max_steps = 1000;
static uint16_t current_total_steps = 0;
static uint16_t forward = 1;
static StepMotorServiceState_t currentState = InitPState;
static uint16_t TimeOfLastRise;
static uint16_t TimeOfLastFall;

// full step
// const int Table[4][4] = {
//     {100, 0, 100, 0}, // Step 1
//     {0, 100, 100, 0}, // Step 2
//     {0, 100, 0, 100}, // Step 3
//     {100, 0, 0, 100}  // Step 4
// };

// wave step
// const int Table[4][4] = {
//     {100, 0, 0, 0}, // Step 1: IN1 HIGH
//     {0, 0, 100, 0}, // Step 2: IN2 HIGH
//     {0, 100, 0, 0}, // Step 3: IN3 HIGH
//     {0, 0, 0, 100}  // Step 4: IN4 HIGH
// };

// half step
const int Table[8][4] = {
    {1, 0, 0, 0}, // Step 1: A+ (IN1 HIGH)
    {1, 1, 0, 0}, // Step 2: A+ & B+
    {0, 1, 0, 0}, // Step 3: B+
    {0, 1, 1, 0}, // Step 4: B+ & A-
    {0, 0, 1, 0}, // Step 5: A-
    {0, 0, 1, 1}, // Step 6: A- & B-
    {0, 0, 0, 1}, // Step 7: B-
    {1, 0, 0, 1}  // Step 8: B- & A+
};

static int currentStep = 0;

bool InitStepMotorService(uint8_t Priority)
{
    ES_Event_t ThisEvent;

    MyPriority = Priority;

    // When doing testing, it is useful to announce just which program
    // is running.
    clrScrn();
    puts("\rStarting StepMotorService\r");
    DB_printf("compiled at %s on %s\n", __TIME__, __DATE__);
    DB_printf("\n\r\n");

    TRISAbits.TRISA0 = 0; // PWM output for PWM1
    ANSELAbits.ANSA0 = 0;
    TRISAbits.TRISA1 = 0; // PWM output for PWM2
    TRISAbits.TRISA2 = 0; // PWM output for PWM3
    TRISAbits.TRISA3 = 0; // PWM output for PWM4

    // Configure PWM channels
    PWMSetup_BasicConfig(4); // Channel 1 for RA0
    // PWMSetup_BasicConfig(2); // Channel 2 for RA1
    // PWMSetup_BasicConfig(3); // Channel 3 for RA2

    // Assign channels to Timer 2
    PWMSetup_AssignChannelToTimer(1, _Timer2_);
    PWMSetup_AssignChannelToTimer(2, _Timer2_);
    PWMSetup_AssignChannelToTimer(3, _Timer2_);
    PWMSetup_AssignChannelToTimer(4, _Timer2_);

    // Set PWM Period
    PWMSetup_SetPeriodOnTimer(2500, _Timer2_); // Adjust period based on desired frequency and TICS_PER_MS

    // Map PWM channels to output pins
    PWMSetup_MapChannelToOutputPin(1, PWM_RPA0);
    PWMSetup_MapChannelToOutputPin(2, PWM_RPA1);
    PWMSetup_MapChannelToOutputPin(3, PWM_RPA3);
    PWMSetup_MapChannelToOutputPin(4, PWM_RPA2);

    SPISetup_BasicConfig(SPI_SPI1);
    SPISetup_SetLeader(SPI_SPI1, SPI_SMP_MID);
    // SPISetup_MapSSOutput(SPI_SPI1, SPI_RPA0);
    // SPISetup_MapSDOutput(SPI_SPI1, SPI_RPA1);

    SPI1BUF;
    SPISetEnhancedBuffer(SPI_SPI1, 1);
    SPISetup_SetBitTime(SPI_SPI1, 10000);
    SPISetup_SetXferWidth(SPI_SPI1, SPI_16BIT);
    SPISetup_SetActiveEdge(SPI_SPI1, SPI_SECOND_EDGE);
    SPISetup_SetClockIdleState(SPI_SPI1, SPI_CLK_HI);
    SPI1CONbits.FRMPOL = 0;
    SPISetup_EnableSPI(SPI_SPI1);

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

bool PostStepMotorService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunStepMotorService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    switch (currentState)
    {
    case InitPState: // If current state is initial Psedudo State
    {
        if (ThisEvent.EventType == ES_INIT) // only respond to ES_Init
        {
            PWMOperate_SetDutyOnChannel(50, 1); // Set duty cycle to 0 for channel 0
            PWMOperate_SetDutyOnChannel(50, 2); // Set duty cycle to 0 for channel 1
            PWMOperate_SetDutyOnChannel(50, 3); // Set duty cycle to 0 for channel 2
            PWMOperate_SetDutyOnChannel(50, 4); // Set duty cycle to 0 for channel 3
            ES_Timer_InitTimer(STEP_MOTOR_TIMER, step_interval);
            currentState = WaitForSpeed;
        }
    }
    break;

    case WaitForSpeed: // If current state is state one
    {
        switch (ThisEvent.EventType)
        {
        case ES_NEW_KEY:
        {
            DB_printf("%d\n", ThisEvent.EventParam);
            step_interval = 3 + ((float)(ThisEvent.EventParam) / 1024) * (8);
            DB_printf("step_interval %d\n", step_interval);
        }
        case ES_TIMEOUT:
        {
            // ES_Timer_InitTimer(STEP_MOTOR_TIMER, 1000 / steps_per_second);

            ES_Timer_InitTimer(STEP_MOTOR_TIMER, step_interval);

            // Update PWM duty cycles based on the current step

            PWMOperate_SetDutyOnChannel(Table[currentStep][0], 1);
            PWMOperate_SetDutyOnChannel(Table[currentStep][1], 2);
            PWMOperate_SetDutyOnChannel(Table[currentStep][2], 3);
            PWMOperate_SetDutyOnChannel(Table[currentStep][3], 4);

            // PWMOperate_SetPulseWidthOnChannel(50, 1);
            //  Print the current step and duty values
            // DB_printf("Current Step: %d\n", currentStep);
            //  DB_printf("Duty Values: %d, %d, %d, %d\n",
            //            Table[currentStep][0],
            //            Table[currentStep][1],
            //            Table[currentStep][2],
            //            Table[currentStep][3]);

            // Move to the next step
            if (forward == 1)
            {
                currentStep = (currentStep + 1) % (sizeof(Table) / sizeof(Table[0]));
            }

            if (forward == 0)
            {
                if (currentStep == 0)
                {
                    currentStep = (sizeof(Table) / sizeof(Table[0])) - 1;
                }
                else
                {
                    currentStep -= 1;
                }
            }

            current_total_steps += 1;

            if (forward == 1 && current_total_steps == max_steps)
            {
                puts("backward");
                forward = 0;
                current_total_steps = 0;
                currentStep = (sizeof(Table) / sizeof(Table[0])) - 1;
                for (uint16_t delayCounter = 0; delayCounter < 65535; delayCounter++)
                {
                }
            }
            if (forward == 0 && current_total_steps == max_steps)
            {
                puts("forward");
                forward = 1;
                current_total_steps = 0;
                currentStep = 0;
                for (uint16_t delayCounter = 0; delayCounter < 65535; delayCounter++)
                {
                }
                currentState = Pause;
            }
        }
        case Pause:
        {
        }
        // repeat cases as required for relevant events
        default:;
        } // end switch on CurrentEvent
    }
    break;
    // repeat state pattern as required for other states
    default:;
    } // end switch on Current State
    return ReturnEvent;
}

StepMotorServiceState_t QueryStepMotorService(void)
{
    return currentState;
}
