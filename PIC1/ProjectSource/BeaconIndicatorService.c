/****************************************************************************
 Module
   MotorService.c

 Description
   Motor control service for alignment and side detection

****************************************************************************/

//#include "../ProjectHeaders/MyMotorService.h"
//#include "../ProjectHeaders/PIC32_AD_Lib.h"
//#include "RobotFSM.h"
//#include "PlannerHSM.h"

// Hardware
#include <sys/attribs.h>
#include <xc.h>

// Event & Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_Port.h"
#include "terminal.h"
#include "dbprintf.h"
#include "BeaconIndicatorService.h"

/*----------------------------- Module Defines ----------------------------*/

#define ONE_SEC 1000
#define ELEC_TIME_CONS 148 // in microsec
#define PWM_FREQ 6756 // Set PWM freq at least 1/tao
#define PIC_FREQ 20000000 // PIC 20MHz
#define TIMER2_PRESCALE 1 // Assume 1:1 prescale value
#define TICK_FREQ (PIC_FREQ / 8) // Timer3 Prescale 1:8

// Alignment Constants
#define ALIGNMENT_TIMEOUT 5000  // 5 seconds for alignment
#define FREQ_G 3333
#define FREQ_L 2000
#define FREQ_B 1427
#define FREQ_R 909

// Side Detection Colors
Beacon_t DetectedBeacon;

static uint8_t MyPriority;
//static MotorState_t CurrentState;
static bool aligned = false;
static int detectedFreq;

typedef union {
    uint32_t FullTime;
    uint16_t ByBytes[2];
} Timer32_t;

volatile uint16_t CapturedTime;
volatile static uint16_t NumRollover;
volatile static Timer32_t CurrentVal;
volatile static uint32_t PrevVal;
volatile static uint32_t PulsePR;
uint8_t FreqTolerance = 10;

/*------------------------------ Module Code ------------------------------*/

bool InitBeaconIndicatorService(uint8_t Priority) {
    clrScrn();
    puts("\rStarting Beacon Indicator Service\r");
    
    Config_IC2();
    ConfigTimer3();
    TRISAbits.TRISA3 = 1;
    IC2R = 0b0000; //Map IC2 to RA3

    MyPriority = Priority;
    
    ES_Event_t ThisEvent;
    ThisEvent.EventType = ES_INIT;
    StopMotor();
//    PostBeaconIndicatorService()
    ES_Event_t Event2Post;
    Event2Post.EventType = ES_REQUEST_SIDE_DETECTION;  
    PostBeaconIndicatorService(Event2Post);
    DB_printf("ES_REQUEST_SIDE_DETECTION posted to Side Service!!!!!\r\n");
    __builtin_enable_interrupts();
    
    return ES_PostToService(MyPriority, ThisEvent);
}

bool PostBeaconIndicatorService(ES_Event_t ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunBeaconIndicatorService(ES_Event_t ThisEvent) {
    ES_Event_t ReturnEvent = {ES_NO_EVENT, 0}; 

    uint8_t dutyCycle = 60;

    switch (ThisEvent.EventType) {
        case ES_REQUEST_SIDE_DETECTION:
            DB_printf("\r Receive Side detection request: Start Aligning\r\n");

            // Start rotating CCW
            LATBbits.LATB11 = 1;
            LATBbits.LATB9 = 0;
//            OC4RS = 400;
//            OC3RS = 0;
            OC4RS = (PR2 + 1) * (100 - dutyCycle) / 100;
            OC3RS = (PR2 + 1) * dutyCycle / 100;

            ES_Timer_InitTimer(BEACON_ALIGN_TIMER, ALIGNMENT_TIMEOUT);
            break;

        case ES_STOP:
            DB_printf("\rMotor: Stop!!!!!!! in ES\r\n");
            LATBbits.LATB11 = 0;
            LATBbits.LATB9 = 0;
            OC4RS = 0;
            OC3RS = 0;
            break;

        case ES_TIMEOUT:
            if (ThisEvent.EventParam == BEACON_ALIGN_TIMER) {
                if (aligned){
                    DB_printf("Aligned succeed,R\n");
                    DB_printf("Detected Frequency: %d\n", detectedFreq);
                    DB_printf("Detected DetectedBeacon is %d\n", DetectedBeacon);
                    Moveforwards();
                    
                    ES_Event_t Event2Post = {ES_SIDE_DETECTED, DetectedBeacon};
                    //PostPlannerHSM(Event2Post);
                    ES_Timer_InitTimer(BEACON_ALIGN_STOP_TIMER, ALIGNMENT_TIMEOUT*5/10);
                }else{
                  DB_printf("Aligned failed, stopping motor.. ..\n"); 
//                  ES_Event_t Event2Post = {ES_SIDE_DETECTED, BEACON_UNKNOWN};
                  //PostPlannerHSM(Event2Post);
                  //Moveforwards();
                  //ES_Event_t Event2Post = {ES_SIDE_DETECTED, DetectedBeacon};
                    //PostPlannerHSM(Event2Post);
                  //ES_Timer_InitTimer(BEACON_ALIGN_STOP_TIMER, ALIGNMENT_TIMEOUT*5/10);
                  StopMotor();
                }
//                ES_Event_t Event2Post = {aligned ? ES_ALIGN_SUCCESS : ES_ALIGN_FAIL, 0};
//                PostRobotFSM(Event2Post);
            }else if (ThisEvent.EventParam == BEACON_ALIGN_STOP_TIMER){
                StopMotor();
                DB_printf("stopping motor anyway.. ..\n"); 
            } 
            break;

        default:
            break;
    }
    return ReturnEvent;
}

/***************************************************************************
 * Private Functions
 ***************************************************************************/


void ConfigTimer3() {
  // Clear the ON control bit to disable the timer. 
  T3CONbits.ON = 0;
  // Clear the TCS control bit to select the internal PBCLK source.
  T3CONbits.TCS = 0;
  // Set input clock prescale to be 1:8. 
  T3CONbits.TCKPS = 0b011; 
  // Clear the timer register TMR2 
  TMR3 = 0;
  // Load PR3 with desired 16-bit match value
  PR3 = 0xFFFF;

  // Clear the T2IF interrupt flag bit in the IFS2 register
  IFS0CLR = _IFS0_T3IF_MASK;
  // Set the priority of the Timer 3 interrupt to 6
  IPC3bits.T3IP = 6; 

  // Enable interrupts on Timer 3
  IEC0SET = _IEC0_T3IE_MASK;
  // Turn on the timer 3
  T3CONbits.ON = 1;
  return; 
}


void Config_IC2() {
    IC2CONbits.ON = 0;
    IC2CONbits.C32 = 0;
    IC2CONbits.ICTMR = 0;
    IC2CONbits.ICI = 0b00;
    IC2CONbits.ICM = 0b011;

    IFS0CLR = _IFS0_IC2IF_MASK;
    IPC2bits.IC2IP = 7;
    IEC0SET = _IEC0_IC2IE_MASK;
    IC2CONbits.ON = 1;
}

/***************************************************************************
 * Interrupt Service Routines
 ***************************************************************************/

void __ISR(_INPUT_CAPTURE_2_VECTOR, IPL7SOFT) IC2ISR(void) {
    CapturedTime = IC2BUF;
    IFS0CLR = _IFS0_IC2IF_MASK;

    if ((IFS0bits.T3IF == 1) && (CapturedTime < 0x8000)) {
        NumRollover++;
        IFS0CLR = _IFS0_T3IF_MASK;
    }

    CurrentVal.ByBytes[0] = CapturedTime;
    CurrentVal.ByBytes[1] = NumRollover;
    PulsePR = CurrentVal.FullTime - PrevVal;

    float freq = TICK_FREQ / PulsePR;
    //puts("detecting...");
    //DB_printf("PulsePR is: %d\n", PulsePR);
    if (PulsePR > 0 && !aligned) {
        detectedFreq = (int)(freq+0.5);
        DB_printf("Frequency is: %d\n", freq);
        
        if (abs(detectedFreq - FREQ_G) <= FreqTolerance) {
            //DB_printf("Aligned with BEACON G\n");
            DetectedBeacon = BEACON_G;
            aligned = true;
            ES_Event_t myEvent;
            myEvent.EventType = ES_STOP;
            PostBeaconIndicatorService(myEvent);
            //StopMotor();
        } else if (abs(detectedFreq - FREQ_B) <= FreqTolerance) {
            //DB_printf("Aligned with BEACON B\n");
            DetectedBeacon = BEACON_B;
            aligned = true;
            ES_Event_t myEvent;
            myEvent.EventType = ES_STOP;
            PostBeaconIndicatorService(myEvent);
            //StopMotor();
        } else if (abs(detectedFreq - FREQ_R) <= FreqTolerance) {
            //DB_printf("Aligned with BEACON R\n");
            DetectedBeacon = BEACON_R;
            aligned = true;
            //puts("We got it!");
            
            ES_Event_t myEvent;
            myEvent.EventType = ES_STOP;
            PostBeaconIndicatorService(myEvent);
            //StopMotor();
        } else if (abs(detectedFreq - FREQ_L) <= FreqTolerance) {
            //DB_printf("Aligned with BEACON L\n");
            DetectedBeacon = BEACON_L;
            aligned = true;
            ES_Event_t myEvent;
            myEvent.EventType = ES_STOP;
            PostBeaconIndicatorService(myEvent);
            //StopMotor();
        }
    }
    PrevVal = CurrentVal.FullTime;
}



void StopMotor(){
    LATBbits.LATB11 = 0;
    LATBbits.LATB9 = 0;
    OC4RS = 0;
    OC3RS = 0;
    puts("stopping motor!");
}

void Moveforwards(){
    uint8_t dutyCycle = 70;
    LATBbits.LATB11 = 0;
    LATBbits.LATB9 = 0;
//    OC4RS = 500;
//    OC3RS = 500;
    OC4RS = (PR2 + 1) * dutyCycle / 100;
    OC3RS = (PR2 + 1) * dutyCycle / 100;
    puts("moving forwards!!!!");
}


void __ISR(_TIMER_3_VECTOR,IPL6SOFT) Timer3_ISR(void){
    // puts("\r Entered Timer 3 ISR \r");
    // Disable interrupts globally
    __builtin_disable_interrupts();
    // If T3IF is pending
    if(1 == IFS0bits.T3IF){
        // Increment the rollover counter
        NumRollover++;
        // Clear the Timer 3 interrupt flag
        IFS0CLR = _IFS0_T3IF_MASK;
    }
    // Enable interrupts globally
    __builtin_enable_interrupts();
}