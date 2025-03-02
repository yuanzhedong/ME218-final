//
//#include <xc.h>
//// #include <proc/p32mx170f256b.h>
//
//// Event & Services Framework
//#include "ES_Configure.h"
//#include "ES_Framework.h"
//#include "ES_DeferRecall.h"
//#include "ES_Port.h"
//#include "terminal.h"
//#include "dbprintf.h"
//#include "Joint1ServoService.h"
//#include <sys/attribs.h>
//
///*---------------------------- Module Variables ---------------------------*/
//// with the introduction of Gen2, we need a module level Priority variable
//static uint8_t MyPriority;
//volatile static uint16_t NumRollover;
//
//#define ONE_SEC 1000
//#define HALF_SEC (ONE_SEC / 2)
//#define QUATER_SEC (HALF_SEC / 2)
//
////#define GAME_TIME 60 * ONE_SEC
//
////#define PWM_OUTPUT LATBbits.LATB9
//
//
//static uint16_t maxPulseTicks = 6250; // +90
//static uint16_t minPulseTicks = 1250; // -90
////uint16_t currnetPulseTicks = 6250;
////uint8_t currentStep = 0;
////uint8_t lastPostLiveStep = 0;
//static uint8_t maxStep = 60*1000 / QUATER_SEC ;
//
////static Beacon_t DetectedBeacon;
//static uint8_t DutyCycle;
//// 50 , 1
//#define CHANNEL4_PWM_FREQUENCY 50
//#define TIMER3_PRESCALE 64
//#define PIC_FREQ 20000000 // PIC 20MHz
//// 9.5 for 180 degrees, 2 for 0 degree
//#define PWM_0_DEG 2
//#define PWM_180_DEG 3.2
//#define JOINT2_TIME_STEP 50
//////
//
//static void ConfigPWM_OC3() {
//
////     map OC2 to RB13
//    RPA3R = 0b0101;
//    //Clear OC2CON register: 
//    OC3CON = 0;
//    // Configure the Output Compare module for one of two PWM operation modes
//    OC3CONbits.ON = 0;         // Turn off Output Compare module
//    OC3CONbits.OCM = 0b110;    // PWM mode without fault pin
//    OC3CONbits.OCTSEL = 1;     // Use Timer3 as the time base
//
//    // Set the PWM duty cycle by writing to the OCxRS register
//    OC3RS = PR3 * 0;       // Secondary Compare Register (for duty cycle)
//    OC3R = PR3 * 0;        // Primary Compare Register (initial value)
//    OC3CONbits.ON = 1;         // Enable Output Compare module
//    
//    return;
//}
//
//
//bool InitJoint2ServoService(uint8_t Priority)
//{
//    ES_Event_t ThisEvent;
//
//    MyPriority = Priority;
//
//    // When doing testing, it is useful to announce just which program
//    // is running.
//    puts("\rStarting ServoService\r");
//
//    TRISAbits.TRISA3 = 0; // set RA3 as PMW output pin
//    ConfigPWM_OC3();  // !!!!!!!!!!!!
//    //PR3 = 399999;
//    DutyCycle = PWM_0_DEG;
//
//    // post the initial transition event
//    ThisEvent.EventType = ES_ROTATE_ZERO;
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
//bool PostJoint2ServoService(ES_Event_t ThisEvent)
//{
//    return ES_PostToService(MyPriority, ThisEvent);
//}
//
//
//ES_Event_t RunJoint2ServoService(ES_Event_t ThisEvent)
//{
//    ES_Event_t ReturnEvent;
//    
//    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
//    static float currentPWM = PWM_0_DEG; // Start from PWM_0_DEG
//    const float PWM_STEP = 0.2;
//    static bool increasing = false; // Track direction
//    
//    switch (ThisEvent.EventType)
//    {
//    case ES_INIT:
////        // Start at PWM_0_DEG
////        currentPWM = PWM_0_DEG;
////        OC1RS = (float)(PR3 + 1) * currentPWM / 100;
////        DB_printf("Starting PWM at %f\n", currentPWM);
////        // Post a timeout event to start advancing
////        ES_Timer_InitTimer(JOINT1_SERVO_TIMER, JOINT1_TIME_STEP);
//    break;
//    
//    case ES_ROTATE_ZERO:
//        // Start at PWM_0_DEG
//        currentPWM = PWM_0_DEG;
//        increasing = false;
//        OC3RS = (float)(PR3 + 1) * currentPWM / 100;
//        DB_printf("Starting PWM at %f\n", currentPWM);
//        // Post a timeout event to start advancing
//        ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP);
//        break;
//    
//    case ES_ROTATE_180:
//        increasing = true;
//        ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP);
//        break;
//    
//    case ES_TIMEOUT:
//        if (increasing) {
//            if (currentPWM > PWM_180_DEG) {
//                currentPWM -= PWM_STEP;
//                if (currentPWM < PWM_180_DEG) {
//                    currentPWM = PWM_180_DEG; // Ensure it does not exceed limit
//                }
//                OC3RS = (float)(PR3 + 1) * currentPWM / 100;
//                //DB_printf("Decreasing PWM: %f\n", currentPWM);
//                ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP);
//            }
//        } else {
//            if (currentPWM < PWM_0_DEG) {
//                currentPWM += PWM_STEP;
//                if (currentPWM > PWM_0_DEG) {
//                    currentPWM = PWM_0_DEG; // Ensure it does not exceed limit
//                }
//                OC3RS = (float)(PR3 + 1) * currentPWM / 100;
//                //DB_printf("Increasing PWM: %f\n", currentPWM);
//                ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP*5);
//            }
//        }
//        break;
//    
//    default:
//        break;
//    }
//    return ReturnEvent;
//}



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
static uint8_t maxStep = 60*1000 / QUATER_SEC ;

static uint8_t DutyCycle;
// 50 , 1
#define CHANNEL4_PWM_FREQUENCY 50
#define TIMER3_PRESCALE 64
#define PIC_FREQ 20000000 // PIC 20MHz
// 9.5 for 180 degrees, 2 for 0 degree
#define PWM_0_DEG 2
#define PWM_180_DEG 9.5
#define JOINT2_TIME_STEP 50
static float currentPWM;
static bool increasing = false;

static void ConfigPWM_OC3() {
    RPA3R = 0b0101;
    OC3CON = 0;
    OC3CONbits.ON = 0;
    OC3CONbits.OCM = 0b110;
    OC3CONbits.OCTSEL = 1;
    OC3RS = PR3 * 0;
    OC3R = PR3 * 0;
    OC3CONbits.ON = 1;
    return;
}

bool InitJoint2ServoService(uint8_t Priority)
{
    ES_Event_t ThisEvent;

    MyPriority = Priority;
    puts("\rStarting ServoService2!!\r");
    TRISAbits.TRISA3 = 0; // set RA3 as PMW output pin
    ConfigPWM_OC3();
    DutyCycle = PWM_0_DEG;

    // Move to 0-degree position at startup
    //OC3RS = (float)(PR3 + 1) * PWM_0_DEG / 100;
    //DB_printf("Initializing at 0 degrees (PWM: %f)\n", PWM_0_DEG);
    
    ThisEvent.EventType = ES_INIT;
    //currentPWM = PWM_0_DEG;
    if (ES_PostToService(MyPriority, ThisEvent) == true)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool PostJoint2ServoService(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunJoint2ServoService(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
//    static float currentPWM = PWM_180_DEG;
    const float PWM_STEP = 0.2;
//    static bool increasing = false;
    
    switch (ThisEvent.EventType)
    {
    case ES_INIT:
        currentPWM = PWM_180_DEG;
        increasing = true;
        OC3RS = (float)(PR3 + 1) * currentPWM / 100;
        DB_printf("[INIT] Servo at 180 degrees (PWM: %d)\n", (int)(currentPWM * 100));
        break;
    
    case ES_ROTATE_ZERO:
        DB_printf("Recevie ES_ROTATE_ZERO");
        if (currentPWM == PWM_0_DEG) {
            DB_printf("[EVENT] Already at 0 degrees, no action needed.\n");
        } else {
            increasing = false;
            DB_printf("[EVENT] Moving to 0 degrees.\n");
            ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP);
        }
        break;
    
    case ES_ROTATE_180:
        DB_printf("Recevie ES_ROTATE_180");
        if (currentPWM == PWM_180_DEG) {
            DB_printf("[EVENT] Already at 180 degrees, no action needed.\n");
        } else {
            increasing = true;
            DB_printf("[EVENT] Moving to 180 degrees.\n");
            ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP);
        }
        break;
    
    case ES_TIMEOUT:
        if (increasing) {
            if (currentPWM < PWM_180_DEG) {
                currentPWM += PWM_STEP;
                if (currentPWM > PWM_180_DEG) {
                    currentPWM = PWM_180_DEG;
                }
                OC3RS = (float)(PR3 + 1) * currentPWM / 100;
                DB_printf("[TIMEOUT] Increasing PWM: %d\n", (int)(currentPWM * 100));
                ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP);
            } else {
                DB_printf("[TIMEOUT] Reached 180 degrees. Stopping.\n");
            }
        } else {
            if (currentPWM > PWM_0_DEG) {
                currentPWM -= PWM_STEP;
                if (currentPWM < PWM_0_DEG) {
                    currentPWM = PWM_0_DEG;
                }
                OC3RS = (float)(PR3 + 1) * currentPWM / 100;
                DB_printf("[TIMEOUT] Decreasing PWM: %d\n", (int)(currentPWM * 100));
                ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP);
            } else {
                DB_printf("[TIMEOUT] Reached 0 degrees. Stopping.\n");
            }
        }
        break;
    
    default:
        DB_printf("[UNKNOWN EVENT] Type: %d\n", ThisEvent.EventType);
        break;
    }
    return ReturnEvent;
}





//ES_Event_t RunJoint2ServoService(ES_Event_t ThisEvent)
//{
//    ES_Event_t ReturnEvent;
//    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
//    static float currentPWM = PWM_180_DEG;  // Start at 180 degrees
//    const float PWM_STEP = 0.2;
//    static bool movingToZero = false; // Track direction
//    
//    switch (ThisEvent.EventType)
//    {
//    case ES_INIT:
//        currentPWM = PWM_180_DEG;  // Ensure initial position
//        movingToZero = false;
//        OC3RS = (float)(PR3 + 1) * currentPWM / 100;
//        DB_printf("[INIT] Servo at 180 degrees (PWM: %f)\n", currentPWM);
//        break;
//    
//    case ES_ROTATE_ZERO:
//        if (currentPWM == PWM_0_DEG) {
//            DB_printf("[EVENT] Already at 0 degrees, no action needed.\n");
//        } else {
//            movingToZero = true;
//            DB_printf("[EVENT] Moving to 0 degrees.\n");
//            ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP);
//        }
//        break;
//    
//    case ES_ROTATE_180:
//        if (currentPWM == PWM_180_DEG) {
//            DB_printf("[EVENT] Already at 180 degrees, no action needed.\n");
//        } else {
//            movingToZero = false;
//            DB_printf("[EVENT] Moving to 180 degrees.\n");
//            ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP);
//        }
//        break;
//    
//    case ES_TIMEOUT:
//        if (movingToZero) {
//            if (currentPWM > PWM_0_DEG) {
//                currentPWM -= PWM_STEP;
//                if (currentPWM < PWM_0_DEG) {
//                    currentPWM = PWM_0_DEG;
//                }
//                OC3RS = (float)(PR3 + 1) * currentPWM / 100;
//                DB_printf("[TIMEOUT] Decreasing PWM: %f\n", currentPWM);
//                ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP);
//            } else {
//                DB_printf("[TIMEOUT] Reached 0 degrees. Stopping.\n");
//            }
//        } else {
//            if (currentPWM < PWM_180_DEG) {
//                currentPWM += PWM_STEP;
//                if (currentPWM > PWM_180_DEG) {
//                    currentPWM = PWM_180_DEG;
//                }
//                OC3RS = (float)(PR3 + 1) * currentPWM / 100;
//                DB_printf("[TIMEOUT] Increasing PWM: %f\n", currentPWM);
//                ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP);
//            } else {
//                DB_printf("[TIMEOUT] Reached 180 degrees. Stopping.\n");
//            }
//        }
//        break;
//    
//    default:
//        DB_printf("[UNKNOWN EVENT] Type: %d\n", ThisEvent.EventType);
//        break;
//    }
//    return ReturnEvent;
//}



//ES_Event_t RunJoint2ServoService(ES_Event_t ThisEvent)
//{
//    ES_Event_t ReturnEvent;
//    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
//    static float currentPWM = PWM_0_DEG; // Start at 0 degrees
//    const float PWM_STEP = 0.2;
//    static bool increasing = false; // Track direction
//
//    switch (ThisEvent.EventType)
//    {
//    case ES_INIT:
//        currentPWM = PWM_0_DEG;
//        increasing = false;
//        OC3RS = (float)(PR3 + 1) * currentPWM / 100;
//        DB_printf("[INIT] Servo at 0 degrees (PWM: %f)\n", currentPWM);
//        break;
//    
//    case ES_ROTATE_ZERO:
//        if (currentPWM != PWM_0_DEG) {
//            increasing = false;
//            DB_printf("[EVENT] Received ES_ROTATE_ZERO. Starting movement.\n");
//            ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP);
//        } else {
//            DB_printf("[EVENT] Ignored ES_ROTATE_ZERO. Already at position.\n");
//        }
//        break;
//    
//    case ES_ROTATE_180:
//        if (currentPWM != PWM_180_DEG) {
//            increasing = true;
//            DB_printf("[EVENT] Received ES_ROTATE_180. Starting movement.\n");
//            ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP);
//        } else {
//            DB_printf("[EVENT] Ignored ES_ROTATE_180. Already at position.\n");
//        }
//        break;
//    
//    case ES_TIMEOUT:
//        if (increasing) {
//            if (currentPWM < PWM_180_DEG) {
//                currentPWM += PWM_STEP;
//                if (currentPWM > PWM_180_DEG) {
//                    currentPWM = PWM_180_DEG;
//                }
//                OC3RS = (float)(PR3 + 1) * currentPWM / 100;
//                DB_printf("[TIMEOUT] Increasing PWM: %f\n", currentPWM);
//                ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP);
//            } else {
//                DB_printf("[TIMEOUT] Reached 180 degrees. Stopping.\n");
//            }
//        } else {
//            if (currentPWM > PWM_0_DEG) {
//                currentPWM -= PWM_STEP;
//                if (currentPWM < PWM_0_DEG) {
//                    currentPWM = PWM_0_DEG;
//                }
//                OC3RS = (float)(PR3 + 1) * currentPWM / 100;
//                DB_printf("[TIMEOUT] Decreasing PWM: %f\n", currentPWM);
//                ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP);
//            } else {
//                DB_printf("[TIMEOUT] Reached 0 degrees. Stopping.\n");
//            }
//        }
//        break;
//    
//    default:
//        DB_printf("[UNKNOWN EVENT] Type: %d\n", ThisEvent.EventType);
//        break;
//    }
//    return ReturnEvent;
//}
//


//ES_Event_t RunJoint2ServoService(ES_Event_t ThisEvent)
//{
//    ES_Event_t ReturnEvent;
//    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
//    static float currentPWM = PWM_0_DEG;
//    const float PWM_STEP = 0.2;
//    static bool increasing = false;
//    
//    switch (ThisEvent.EventType)
//    {
//    case ES_INIT:
//        currentPWM = PWM_180_DEG;
//        increasing = true;
//        OC3RS = (float)(PR3 + 1) * currentPWM / 100;
//        DB_printf("Servo initialized at 0 degrees\n");
//        break;
//    
//    case ES_ROTATE_ZERO:
//        if (currentPWM != PWM_0_DEG) {
//            increasing = false;
//            ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP);
//        }
//        break;
//    
//    case ES_ROTATE_180:
//        if (currentPWM != PWM_180_DEG) {
//            increasing = true;
//            ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP);
//        }
//        break;
//    
//    case ES_TIMEOUT:
//        if (increasing) {
//            if (currentPWM < PWM_180_DEG) {
//                currentPWM += PWM_STEP;
//                if (currentPWM > PWM_180_DEG) {
//                    currentPWM = PWM_180_DEG;
//                }
//                OC3RS = (float)(PR3 + 1) * currentPWM / 100;
//                ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP);
//            }
//        } else {
//            if (currentPWM > PWM_0_DEG) {
//                currentPWM -= PWM_STEP;
//                if (currentPWM < PWM_0_DEG) {
//                    currentPWM = PWM_0_DEG;
//                }
//                OC3RS = (float)(PR3 + 1) * currentPWM / 100;
//                ES_Timer_InitTimer(JOINT2_SERVO_TIMER, JOINT2_TIME_STEP);
//            }
//        }
//        break;
//    
//    default:
//        break;
//    }
//    return ReturnEvent;
//}