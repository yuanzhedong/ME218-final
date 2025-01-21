#include "../ProjectHeaders/EncoderService.h"
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Timers.h"
#include <sys/attribs.h>
#include <xc.h> // Include processor files - each processor file is guarded.

#define ENCODER_TIMER 1
#define TICKS_PER_REVOLUTION 20 // Adjust based on your encoder specification

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

// Exact ISR code from the image
void __ISR(_INPUT_CAPTURE_2_VECTOR,IPL7SOFT) IC2ISR(void){

    // Read the IC2 buffer into a variable
    uint16_t CapturedTime = IC2BUF;

    // Clear the interrupt flag for IC2
    IFS0CLR = _IFS0_IC2IF_MASK;

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

    // Update the previous captured value
    PrevVal = CurrentVal;
    puts("encoder interrupt");
}

// Timer2 ISR for Rollover Handling
void __ISR(_TIMER_2_VECTOR,IPL4SOFT) Timer2_ISR(void)
{
    // Disable interrupts globally
    //_builtin_disable_interrupts();

    // If the Timer 2 interrupt flag is set
    if (1 == IFS0bits.T2IF)
    {
        // Increment the rollover counter
        RolloverCounter++;
        // Clear the Timer 2 interrupt flag
        IFS0CLR = _IFS0_T2IF_MASK;
        puts("timer interrupt");
    }

    // Enable interrupts globally
    //_builtin_enable_interrupts();
}

uint8_t InitEncoderService(uint8_t Priority)
{
    MyPriority = Priority;

    // Configure the external interrupt for the encoder
    TRISBbits.TRISB1 = 1;  // Set RB1 as input
    INTCONbits.INT1EP = 1; // Interrupt on falling edge
    IEC0bits.INT1IE = 1;   // Enable INT1 interrupt
    IFS0bits.INT1IF = 0;   // Clear the interrupt flag

    // Configure Timer2
    T2CON = 0x0000;          // Stop Timer2 and clear configuration
    T2CONbits.TCKPS = 0b110; // Set prescaler to 1:64
    PR2 = 0xFFFF;            // Set maximum period (16-bit timer)
    TMR2 = 0;                // Clear Timer2 count
    IEC0bits.T2IE = 1;       // Enable Timer2 interrupt
    IFS0bits.T2IF = 0;       // Clear Timer2 interrupt flag
    T2CONbits.ON = 1;        // Start Timer2

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
        ES_Timer_InitTimer(ENCODER_TIMER, 1000);
        break;

    case ES_TIMEOUT:
        ES_Timer_InitTimer(ENCODER_TIMER, 1000);

        // Calculate Speed in RPM
        uint32_t CurrentTime = ES_Timer_GetTime();
        float TimeInterval = DeltaTicks / (20.0e6 / 64.0);       // DeltaTicks to seconds (PBCLK = 20MHz, prescaler = 64)
        float RPS = 1.0 / (TimeInterval * TICKS_PER_REVOLUTION); // Revolutions per second
        float RPM = RPS * 60.0;                                  // Convert RPS to RPM

        // Send speed as an event
        //ES_Event_t NewEvent;
        //NewEvent.EventType = ES_ENCODER_SPEED;
        //NewEvent.EventParam = (uint16_t)(RPM * 100); // Speed * 100 to preserve two decimal places
        //PostEncoderService(NewEvent);

        break;

    default:
        break;
    }

    return ReturnEvent;
}
