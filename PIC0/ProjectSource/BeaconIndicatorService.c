
/****************************************************************************
 Module
   MotorService.c

 Description
   Motor control service for alignment and side detection

****************************************************************************/

//#include "../ProjectHeaders/MyMotorService.h"
//#include "../ProjectHeaders/PIC32_AD_Lib.h"
//#include "RobotFSM.h"
#include "PlannerHSM.h"

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
#include "ServoService.h"

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
#define FREQ_R 2000

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
uint8_t FreqTolerance = 30;

/*------------------------------ Module Code ------------------------------*/

bool InitBeaconIndicatorService(uint8_t Priority) {
    clrScrn();
    puts("\rStarting Beacon Indicator Service\r");
    
    // Set motor control pins as digital outputs
    TRISBbits.TRISB2 = 0;
    ANSELBbits.ANSB2 = 0;
    TRISBbits.TRISB9 = 0;

    // Configure Timer2 & PWM
    ConfigTimer2();
    //ConfigPWM_OC1();
    //ConfigPWM_OC3();

    // Configure Timer3 & Input Capture (IC3)
    ConfigTimer3();
    Config_IC1();

    // Initialize Motor Pin state
    LATBbits.LATB2 = 0;
    LATBbits.LATB9 = 0;

    // Set RB11 as input for encoder, mapped to IC3
    TRISBbits.TRISB11 = 1;
    IC2R = 0b0000; //Map IC2 to RA3

    MyPriority = Priority;
    
    ES_Event_t ThisEvent;
    ThisEvent.EventType = ES_INIT;
    return ES_PostToService(MyPriority, ThisEvent);
}

bool PostBeaconIndicatorService(ES_Event_t ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunBeaconIndicatorService(ES_Event_t ThisEvent) {
    ES_Event_t ReturnEvent = {ES_NO_EVENT, 0}; 

    uint8_t dutyCycle = 100;

    switch (ThisEvent.EventType) {
        case ES_REQUEST_SIDE_DETECTION:
            DB_printf("\r Receive Side detection request: Start Aligning\r\n");

            // Start rotating CCW
            LATBbits.LATB2 = 1;
            LATBbits.LATB9 = 0;

            OC1RS = (PR2 + 1) * (100 - dutyCycle) / 100;
            OC3RS = (PR2 + 1) * dutyCycle / 100;

            ES_Timer_InitTimer(BEACON_ALIGN_TIMER, ALIGNMENT_TIMEOUT);
            break;

        case ES_STOP:
            DB_printf("\rMotor: Stop\r\n");
            LATBbits.LATB2 = 0;
            LATBbits.LATB9 = 0;
            OC1RS = 0;
            OC3RS = 0;
            break;

        case ES_TIMEOUT:
            if (ThisEvent.EventParam == BEACON_ALIGN_TIMER) {
                if (aligned){
                    DB_printf("Aligned succeed,R\n");
                    DB_printf("Detected Frequency: %d\n", detectedFreq);
                    ES_Event_t Event2Post = {ES_SIDE_DETECTED, DetectedBeacon};
                    PostPlannerHSM(Event2Post);
                }else{
                  DB_printf("Aligned failed, stopping motor.. R\n"); 
                  ES_Event_t Event2Post = {ES_SIDE_DETECTED, BEACON_UNKNOWN};
                  PostPlannerHSM(Event2Post);
                }
//                ES_Event_t Event2Post = {aligned ? ES_ALIGN_SUCCESS : ES_ALIGN_FAIL, 0};
//                PostRobotFSM(Event2Post);


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

void ConfigTimer2() {
    T2CONbits.ON = 0;
    T2CONbits.TCS = 0;
    T2CONbits.TCKPS = 0b000;
    TMR2 = 0;
    PR2 = PIC_FREQ / (PWM_FREQ * TIMER2_PRESCALE) - 1;

    IFS0CLR = _IFS0_T2IF_MASK;
    IEC0CLR = _IEC0_T2IE_MASK;
}

void ConfigPWM_OC1() {
    RPB3R = 0b0101;
    OC1CON = 0;
    OC1CONbits.OCM = 0b110;
    OC1CONbits.OCTSEL = 0;
    OC1RS = 0;
    OC1R = 0;
    T2CONbits.ON = 1;
    OC1CONbits.ON = 1;
}

void ConfigPWM_OC3() {
    RPB10R = 0b0101;
    OC3CON = 0;
    OC3CONbits.OCM = 0b110;
    OC3CONbits.OCTSEL = 0;
    OC3RS = 0;
    OC3R = 0;
    OC3CONbits.ON = 1;
}

void ConfigTimer3() {
    T3CONbits.ON = 0;
    T3CONbits.TCS = 0;
    T3CONbits.TCKPS = 0b011;
    TMR3 = 0;
    PR3 = 0xFFFF;

    IFS0CLR = _IFS0_T3IF_MASK;
    IPC3bits.T3IP = 6;
    IEC0SET = _IEC0_T3IE_MASK;
    T3CONbits.ON = 1;
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
void Config_IC1() {
    IC1CONbits.ON = 0;
    IC1CONbits.C32 = 0;
    IC1CONbits.ICTMR = 0;
    IC1CONbits.ICI = 0b00;
    IC1CONbits.ICM = 0b011;

    IFS0CLR = _IFS0_IC2IF_MASK;
    IPC1bits.IC1IP = 7;
    IEC0SET = _IEC0_IC1IE_MASK;
    IC1CONbits.ON = 1;
}
/***************************************************************************
 * Interrupt Service Routines
 ***************************************************************************/

void __ISR(_INPUT_CAPTURE_1_VECTOR, IPL7SOFT) IC1ISR(void) {
    CapturedTime = IC1BUF;
    IFS0CLR = _IFS0_IC1IF_MASK;

    if ((IFS0bits.T3IF == 1) && (CapturedTime < 0x8000)) {
        NumRollover++;
        IFS0CLR = _IFS0_T3IF_MASK;
    }

    CurrentVal.ByBytes[0] = CapturedTime;
    CurrentVal.ByBytes[1] = NumRollover;
    PulsePR = CurrentVal.FullTime - PrevVal;

    float freq = TICK_FREQ / PulsePR;
    //DB_printf("PulsePR is: %d\n", PulsePR);
    if (PulsePR > 0 && !aligned) {
        detectedFreq = (int)(freq+0.5);
        DB_printf("Detected Frequency: %d\n", freq);
        
        if (abs(detectedFreq - FREQ_G) <= FreqTolerance) {
            DB_printf("Aligned with BEACON G\n");
            DetectedBeacon = BEACON_G;
            aligned = true;
            ES_Event_t Event2Post = {ES_SIDE_DETECTED, DetectedBeacon};
            PostPlannerHSM(Event2Post);
        } else if (abs(detectedFreq - FREQ_B) <= FreqTolerance) {
            DB_printf("Aligned with BEACON B\n");
            DetectedBeacon = BEACON_B;
            aligned = true;
            ES_Event_t Event2Post = {ES_SIDE_DETECTED, DetectedBeacon};
            PostPlannerHSM(Event2Post);
        } else if (abs(detectedFreq - FREQ_R) <= FreqTolerance) {
            DB_printf("Aligned with BEACON R\n");
            DetectedBeacon = BEACON_R;
            aligned = true;
            ES_Event_t Event2Post = {ES_SIDE_DETECTED, DetectedBeacon};
            PostPlannerHSM(Event2Post);
        } else if (abs(detectedFreq - FREQ_L) <= FreqTolerance) {
            DB_printf("Aligned with BEACON L\n");
            DetectedBeacon = BEACON_L;
            aligned = true;
            ES_Event_t Event2Post = {ES_SIDE_DETECTED, DetectedBeacon};
            PostPlannerHSM(Event2Post);
        }
    }

    PrevVal = CurrentVal.FullTime;
}

void __ISR(_TIMER_3_VECTOR, IPL6SOFT) Timer3_ISR(void) {
    __builtin_disable_interrupts();
    if (IFS0bits.T3IF) {
        NumRollover++;
        IFS0CLR = _IFS0_T3IF_MASK;
    }
    __builtin_enable_interrupts();
}
