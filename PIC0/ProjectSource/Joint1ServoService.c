
#include <xc.h>
// #include <proc/p32mx170f256b.h>

// Event & Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_Port.h"
#include "terminal.h"
#include "dbprintf.h"
#include "Joint1ServoService.h"
#include <sys/attribs.h>

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
volatile static uint16_t NumRollover;

#define ONE_SEC 1000
#define HALF_SEC (ONE_SEC / 2)
#define QUATER_SEC (HALF_SEC / 2)

//#define GAME_TIME 60 * ONE_SEC

//#define PWM_OUTPUT LATBbits.LATB9


static uint16_t maxPulseTicks = 6250; // +90
static uint16_t minPulseTicks = 1250; // -90
//uint16_t currnetPulseTicks = 6250;
//uint8_t currentStep = 0;
//uint8_t lastPostLiveStep = 0;
static uint8_t maxStep = 60*1000 / QUATER_SEC ;

//static Beacon_t DetectedBeacon;
static uint8_t DutyCycle;

// 50 , 1
#define CHANNEL4_PWM_FREQUENCY 50
#define TIMER3_PRESCALE 64
#define PIC_FREQ 20000000 // PIC 20MHz
#define PWM_0_DEG 7
#define PWM_90_DEG 3.2
#define JOINT1_TIME_STEP 50

////
//void ConfigTimer3() {
//    T3CONbits.ON = 0;
//    T3CONbits.TCS = 0;
//    T3CONbits.TCKPS = 0b10;
//    TMR3 = 0;
//    PR3 = PIC_FREQ*50 / (CHANNEL4_PWM_FREQUENCY * TIMER3_PRESCALE) - 1;
//    //PR3 = 399;
//
//    IFS0CLR = _IFS0_T3IF_MASK;
//    IPC3bits.T3IP = 6;
//    IEC0SET = _IEC0_T3IE_MASK;
//    T3CONbits.ON = 1;
//}

static void ConfigPWM_OC1() {

//     map OC2 to RB13
    RPB15R = 0b0101;
    //Clear OC2CON register: 
    OC1CON = 0;
    // Configure the Output Compare module for one of two PWM operation modes
    OC1CONbits.ON = 0;         // Turn off Output Compare module
    OC1CONbits.OCM = 0b110;    // PWM mode without fault pin
    OC1CONbits.OCTSEL = 1;     // Use Timer3 as the time base

    // Set the PWM duty cycle by writing to the OCxRS register
    OC1RS = PR3 * 0;       // Secondary Compare Register (for duty cycle)
    OC1R = PR3 * 0;        // Primary Compare Register (initial value)
    OC1CONbits.ON = 1;         // Enable Output Compare module
    
    return;
}

//bool InitJoint1ServoService(uint8_t Priority)
//{
//    ES_Event_t ThisEvent;
//
//    MyPriority = Priority;
//
//    // When doing testing, it is useful to announce just which program
//    // is running.
//    puts("\rStarting ServoService\r");
//
//    TRISBbits.TRISB15 = 0; // set RB15 as PMW output pin
//    ConfigPWM_OC1();  // !!!!!!!!!!!!
//    //PR3 = 399999;
//    DutyCycle = PWM_0_DEG;
//
//    // post the initial transition event
//    ThisEvent.EventType = ES_INIT;
//    if (ES_PostToService(MyPriority, ThisEvent) == true)
//    {
//        return true;
//    }
//    else
//    {
//        return false;
//    }
//}
//
//bool PostJoint1ServoService(ES_Event_t ThisEvent)
//{
//    return ES_PostToService(MyPriority, ThisEvent);
//}
//
//
//ES_Event_t RunJoint1ServoService(ES_Event_t ThisEvent)
//{
//    ES_Event_t ReturnEvent;
//    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
//    static float currentPWM = PWM_0_DEG; // Start from PWM_0_DEG
//    const float PWM_STEP = 0.2;
//    
//    switch (ThisEvent.EventType)
//    {
//    case ES_INIT:
//        // Start at PWM_0_DEG
//        currentPWM = PWM_0_DEG;
//        OC1RS = (float)(PR3 + 1) * currentPWM / 100;
//        DB_printf("Starting PWM at %f\n", currentPWM);
//        // Post a timeout event to start advancing
//        ES_Timer_InitTimer(JOINT1_SERVO_TIMER, JOINT1_TIME_STEP);
//        break;
//    
//    case ES_TIMEOUT:
//        if (currentPWM > PWM_90_DEG) {
//            currentPWM -= PWM_STEP;
//            if (currentPWM < PWM_90_DEG) {
//                currentPWM = PWM_90_DEG; // Ensure it does not exceed limit
//            }
//            OC1RS = (float)(PR3 + 1) * currentPWM / 100;
//            DB_printf("Current PWM: %f\n", currentPWM);
//            // Restart timer for next step
//            ES_Timer_InitTimer(JOINT1_SERVO_TIMER, JOINT1_TIME_STEP);
//        }
//        break;
//       
//    
//    
//    default:
//        break;
//    }
//    return ReturnEvent;
//}
//
//
////
////void __ISR(_TIMER_3_VECTOR, IPL6SOFT) Timer3_ISR(void) {
////    __builtin_disable_interrupts();
////    if (IFS0bits.T3IF) {
////        NumRollover++;
////        IFS0CLR = _IFS0_T3IF_MASK;
////    }
////    __builtin_enable_interrupts();
////}


bool InitJoint1ServoService(uint8_t Priority)
{
    ES_Event_t ThisEvent;

    MyPriority = Priority;

    // When doing testing, it is useful to announce just which program
    // is running.
    puts("\rStarting ServoService\r");

    TRISBbits.TRISB15 = 0; // set RB15 as PMW output pin
    ConfigPWM_OC1();  // !!!!!!!!!!!!
    //PR3 = 399999;
    DutyCycle = PWM_0_DEG;

    // post the initial transition event
    ThisEvent.EventType = ES_ROTATE_90;
    if (ES_PostToService(MyPriority, ThisEvent) == true)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool PostJoint1ServoService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}


ES_Event_t RunJoint1ServoService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    static float currentPWM = PWM_0_DEG; // Start from PWM_0_DEG
    const float PWM_STEP = 0.2;
    static bool decreasing = false; // Track direction
    
    switch (ThisEvent.EventType)
    {
    case ES_INIT:
//        // Start at PWM_0_DEG
        currentPWM = PWM_0_DEG;
        OC1RS = (float)(PR3 + 1) * currentPWM / 100;
        DB_printf("Starting PWM at %f\n", currentPWM);
//        // Post a timeout event to start advancing
//        ES_Timer_InitTimer(JOINT1_SERVO_TIMER, JOINT1_TIME_STEP);
    break;
    
    case ES_ROTATE_ZERO:
        // Start at PWM_0_DEG
        currentPWM = PWM_0_DEG;
        decreasing = false;
//        OC1RS = (float)(PR3 + 1) * currentPWM / 100;
//        DB_printf("Starting PWM at %f\n", currentPWM);
        // Post a timeout event to start advancing
        ES_Timer_InitTimer(JOINT1_SERVO_TIMER, JOINT1_TIME_STEP);
        break;
    
    case ES_ROTATE_90:
        decreasing = true;
        ES_Timer_InitTimer(JOINT1_SERVO_TIMER, JOINT1_TIME_STEP);
        break;
    
    case ES_TIMEOUT:
        if (decreasing) {
            if (currentPWM > PWM_90_DEG) {
                currentPWM -= PWM_STEP;
                if (currentPWM < PWM_90_DEG) {
                    currentPWM = PWM_90_DEG; // Ensure it does not exceed limit
                }
                OC1RS = (float)(PR3 + 1) * currentPWM / 100;
                //DB_printf("Decreasing PWM: %f\n", currentPWM);
                ES_Timer_InitTimer(JOINT1_SERVO_TIMER, JOINT1_TIME_STEP);
            }
        } else {
            if (currentPWM < PWM_0_DEG) {
                currentPWM += PWM_STEP;
                if (currentPWM > PWM_0_DEG) {
                    currentPWM = PWM_0_DEG; // Ensure it does not exceed limit
                }
                OC1RS = (float)(PR3 + 1) * currentPWM / 100;
                //DB_printf("Increasing PWM: %f\n", currentPWM);
                ES_Timer_InitTimer(JOINT1_SERVO_TIMER, JOINT1_TIME_STEP*5);
            }
        }
        break;
    
    default:
        break;
    }
    return ReturnEvent;
}
