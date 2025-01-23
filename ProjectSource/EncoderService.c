#include "../ProjectHeaders/EncoderService.h"
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Timers.h"
#include <sys/attribs.h>
#include <xc.h> // Include processor files - each processor file is guarded.

#define TICKS_PER_REVOLUTION 512

static uint8_t max_rpm = 42;
static uint8_t min_rpm = 25;
static uint8_t MyPriority;
static volatile uint16_t EncoderTicks = 0;
static volatile uint32_t RolloverCounter = 0; // Tracks timer rollovers

// Add Rollover handling variables
typedef union
{
    struct
    {
        uint16_t TimerValue;    // Lower 16 bits: Captured timer value
        uint16_t RolloverCount; // Upper 16 bits: Rollover counter
    };
    uint32_t FullTime; // Full 32-bit timestamp
} TimerValue_t;

static volatile TimerValue_t CurrentVal;
static volatile TimerValue_t PrevVal;
static volatile uint32_t DeltaTicks = 0; // Time difference between two input captures

void __ISR(_INPUT_CAPTURE_3_VECTOR, IPL7SOFT) IC3ISR(void)
{
    // Read the IC3 buffer into a variable
    uint16_t CapturedTime = IC3BUF;

    // Clear the interrupt flag for IC3
    IFS0CLR = _IFS0_IC3IF_MASK;

    // If a rollover has occurred and the Timer 2 interrupt flag is still set
    if ((0x8000 > CapturedTime) && (1 == IFS0bits.T2IF))
    {
        // Increment the rollover counter
        RolloverCounter++;
        // Clear the Timer 2 interrupt flag
        IFS0CLR = _IFS0_T2IF_MASK;
    }

    // Set the lower 16 bits of CurrentVal equal to the captured value
    CurrentVal.TimerValue = CapturedTime;
    // Set the upper 16 bits of CurrentVal equal to the rollover counter
    CurrentVal.RolloverCount = RolloverCounter;

    // Compute the period (in ticks) between two pulses
    DeltaTicks = CurrentVal.FullTime - PrevVal.FullTime;
    //DB_printf("%d\n", DeltaTicks);                                                          

    // Update the previous captured value
    PrevVal = CurrentVal;
    // puts("encoder interrupt");
}


// Timer2 ISR for Rollover Handling
void __ISR(_TIMER_2_VECTOR, IPL6SOFT) Timer2_ISR(void)
{
    // Disable interrupts globally
    __builtin_disable_interrupts();

    // If the Timer 2 interrupt flag is set
    if (1 == IFS0bits.T2IF)
    {
        // Increment the rollover counter
        RolloverCounter++;
        // Clear the Timer 2 interrupt flag
        IFS0CLR = _IFS0_T2IF_MASK;
        //puts("timer interrupt");
    }

    // Enable interrupts globally
    __builtin_enable_interrupts();
}

uint8_t InitEncoderService(uint8_t Priority)
{
    MyPriority = Priority;

    // Map RB11 to IC3 using PPS
    TRISBbits.TRISB11 = 1; // Set RB11 as input
    //ANSELBbits.ANSB11 = 0; // Set RB11 as digital
    IC3R = 0b0011;         // Assign RB11 as IC3 input

    // Configure IC3
    IC3CONbits.ICM = 0b011; // Capture rising edges only
    IC3CONbits.ICTMR = 1;   // Use Timer2 as time base
    IC3CONbits.ON = 1;      // Enable Input Capture 3

    // Enable IC3 interrupt
    IFS0bits.IC3IF = 0; // Clear IC3 interrupt flag
    IEC0bits.IC3IE = 1; // Enable IC3 interrupt
    IPC3bits.IC3IP = 3; // Set interrupt priority to 3

    // // Configure Timer2
    // T2CON = 0x0000;          // Stop Timer2 and clear configuration
    // T2CONbits.TCKPS = 0b110; // Set prescaler to 1:64
    // PR2 = 0xFFFF;            // Set maximum period (16-bit timer)
    // TMR2 = 0;                // Clear Timer2 count
    // IEC0bits. = 1;       // Enable Timer2 interrupt
    // IFS0bits.T2IF = 0;       // Clear Timer2 interrupt flag
    // T2CONbits.ON = 1;        // Start Timer2


    // Configure Timer 2 (to be used for Input Capture)
    // Disable Timer 2
    T2CONbits.ON = 0;
    // Select the internal clock as the source
    T2CONbits.TCS = 0;
    //T2CONbits.TGATE = 0; //added
    // Choose a 1:4 prescaler
    T2CONbits.TCKPS = 0b110;
    // Set the initial value of the timer to 0
    TMR2 = 0;
    // Set the Period Register value to the max
    PR2 = 0xFFFF;
    // Clear the Timer 2 interrupt flag
    IFS0CLR = _IFS0_T2IF_MASK;
    // Set the priority of the Timer 2 interrupt to 6
    IPC2bits.T2IP = 6;
    // Enable interrupts from Timer 2
    IEC0SET = _IEC0_T2IE_MASK;
    // Enable Timer 2
    T2CONbits.ON = 1;

    // Initialize the timer for speed calculation
    ES_Timer_InitTimer(ENCODER_TIMER, 1000); // 1 second timer

    PostEncoderService((ES_Event_t){.EventType = ES_INIT});
    return true;
}

uint8_t PostEncoderService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunEncoderService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT;

    switch (ThisEvent.EventType)
    {
    case ES_INIT:
        ES_Timer_InitTimer(ENCODER_TIMER, 10);
        break;

    case ES_TIMEOUT:
        ES_Timer_InitTimer(ENCODER_TIMER, 1000);

        // Calculate Speed in RPM
        //float TimeInterval = (float)(DeltaTicks) *  / (20000000 / 64.0);       // DeltaTicks to seconds (PBCLK = 20MHz, prescaler = 64)
        float TimeInterval = 512 * (float)(DeltaTicks) * 64 / 20000000;
        //float TimeInterval = 0.4;
        //float RPS = 1.0 / (TimeInterval * TICKS_PER_REVOLUTION); // Revolutions per second
        
        
        //uint16_t RPM = RPS * 60.0;                                  // Convert RPS to RPM

        uint16_t RPM = 60 / TimeInterval / 5.9;
        // Send speed as an event
        // ES_Event_t NewEvent;
        // NewEvent.EventType = ES_ENCODER_SPEED;
        // NewEvent.EventParam = (uint16_t)(RPM * 100); // Speed * 100 to preserve two decimal places
        // PostEncoderService(NewEvent);
        DB_printf("Deltaticks: %d\n", DeltaTicks);
        DB_printf("TimeInterval is : %f\n", TimeInterval);

        DB_printf("Speed: %d RPM\n", RPM);
        uint16_t ct = TMR2;
        DB_printf("Current Time: %d\n", ct);

        break;

    default:
        break;
    }

    return ReturnEvent;
}
