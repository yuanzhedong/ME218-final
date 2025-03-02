
#include <xc.h>
// #include <proc/p32mx170f256b.h>

// Event & Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_Port.h"
#include "terminal.h"
#include "dbprintf.h"
#include "ServoService.h"
#include <sys/attribs.h>

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
volatile static uint16_t NumRollover;

#define ONE_SEC 1000
#define HALF_SEC (ONE_SEC / 2)
#define QUATER_SEC (HALF_SEC / 2)

#define GAME_TIME 60 * ONE_SEC

//#define PWM_OUTPUT LATBbits.LATB9


static uint16_t maxPulseTicks = 6250; // +90
static uint16_t minPulseTicks = 1250; // -90
uint16_t currnetPulseTicks = 6250;
uint8_t currentStep = 0;
uint8_t lastPostLiveStep = 0;
static uint8_t maxStep = 60*1000 / QUATER_SEC ;

static Beacon_t DetectedBeacon;
static uint8_t DutyCycle;
// 50 , 1
#define CHANNEL4_PWM_FREQUENCY 50
#define TIMER3_PRESCALE 64
#define PIC_FREQ 20000000 // PIC 20MHz
#define BLUE_PWM 9
#define GREEN_PWM 5
#define INIT_PWM 7

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

static void ConfigPWM_OC4() {

//     map OC2 to RB13
    RPB13R = 0b0101;
    //Clear OC2CON register: 
    OC4CON = 0;
    // Configure the Output Compare module for one of two PWM operation modes
    OC4CONbits.ON = 0;         // Turn off Output Compare module
    OC4CONbits.OCM = 0b110;    // PWM mode without fault pin
    OC4CONbits.OCTSEL = 1;     // Use Timer3 as the time base

    // Set the PWM duty cycle by writing to the OCxRS register
    OC4RS = PR3 * 0;       // Secondary Compare Register (for duty cycle)
    OC4R = PR3 * 0;        // Primary Compare Register (initial value)
    OC4CONbits.ON = 1;         // Enable Output Compare module
    
    return;
}

bool InitServoService(uint8_t Priority)
{
    ES_Event_t ThisEvent;

    MyPriority = Priority;

    // When doing testing, it is useful to announce just which program
    // is running.
    puts("\rStarting ServoService\r");

    TRISBbits.TRISB13 = 0; // set RB13 as PMW output pin
    ConfigTimer3();
    ConfigPWM_OC4();
    //PR3 = 399999;
    DutyCycle = INIT_PWM;

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
    switch (ThisEvent.EventType)
    {
    case ES_INIT:;
        OC4RS = (float)(PR3 + 1) * DutyCycle / 100;
        DB_printf("curent PR3 is %d\n", PR3);
        
    break;
    
    case ES_SIDE_DETECTED:
        DetectedBeacon ==ThisEvent.EventParam;
        if (DetectedBeacon == BEACON_B || DetectedBeacon == BEACON_L){
            DutyCycle = BLUE_PWM;
        }else if (DetectedBeacon == BEACON_G || DetectedBeacon == BEACON_R){
            DutyCycle = GREEN_PWM;
        }
        OC4RS = (float)(PR3 + 1) * DutyCycle / 100;
        DB_printf("ES_SIDE_DETECTED is  %d\n", DetectedBeacon);
    break;
     
    default:
        break;
    } // end switch on Current State
    return ReturnEvent;
}

//
//void __ISR(_TIMER_3_VECTOR, IPL6SOFT) Timer3_ISR(void) {
//    __builtin_disable_interrupts();
//    if (IFS0bits.T3IF) {
//        NumRollover++;
//        IFS0CLR = _IFS0_T3IF_MASK;
//    }
//    __builtin_enable_interrupts();
//}
