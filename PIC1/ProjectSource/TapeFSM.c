/****************************************************************************
 Module
   TapeFSM.c

 Revision
   1.0.1

 Description
   This is a Tape file for implementing flat state machines under the
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/15/12 11:12 jec      revisions for Gen2 framework
 11/07/11 11:26 jec      made the queue static
 10/30/11 17:59 jec      fixed references to CurrentEvent in RunTapeSM()
 10/23/11 18:20 jec      began conversion from SMTape.c (02/20/07 rev)
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "TapeFSM.h"
#include "dbprintf.h"
#include <sys/attribs.h> 
#include "terminal.h"
/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/
static void enterFollowing();  
static void exitFollowing(void); //the exit function for exiting the Following state back to Idle_tapeFSM
static void ConfigPWM_OC1(void);
static void ConfigPWM_OC3(void);
static void ConfigTimer2(void); //time base for OC
static void ConfigTimer3(void);
static void ConfigTimer4(void); //for running control loop
static void ConfigureReflectSensor();//for the reflectance sensor array
/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file

#define PWM_freq  10000 //wheel motor PWM frquency in Hz
#define PIC_freq 20000000
#define PIC_freq_kHz 20000
#define ns_per_tick 50 //nano-sec per tick for the PBC = 1/PIC_freq

#define H_bridge1A_TRIS TRISBbits.TRISB11//output
#define H_bridge1A_LAT LATBbits.LATB11 //latch
#define H_bridge3A_TRIS TRISBbits.TRISB9//output
#define H_bridge3A_LAT LATBbits.LATB9 //latch
#define IC_TIMER_period 50000

//prescalars and priorities
#define prescalar_T2 2 //for PWM for the 2 motors
#define prescalar_T4 4 //used for running control law
#define priority_control 5

//output compare stuff (PWM)
static bool Dir0 = 0; //the direction of the motor, 0 = positive dir
static bool Dir1 = 0; 


//control stuff
#define Control_interval 2 //in ms
#define Ki 0.3
#define Kp 0.1
#define initialDutyCycle 70 //initial duty cycle (in %) at which the car starts to follow the line
volatile int K_error = 0;
volatile int K_error_sum = 0;
volatile int K_effort = 200;//in the unit of ticks for PR2
// framework stuff
static uint8_t MyPriority;
static TapeState_t CurrentState;
/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitTapeFSM

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 18:55
****************************************************************************/
bool InitTapeFSM(uint8_t Priority)
{
  ConfigureReflectSensor();//this calls the ADC library
  //TRIS and LAT for direction control pins
  H_bridge1A_TRIS = 0; //Outputs
  H_bridge1A_LAT = 0;
  H_bridge3A_TRIS = 0; //Outputs
  H_bridge3A_LAT = 0;
  ConfigTimer2();
  ConfigPWM_OC1();
  ConfigPWM_OC3();
  ConfigTimer4();
  
  /********enable interrupt globally *******************/
  __builtin_enable_interrupts();
  ES_Event_t ThisEvent;
  MyPriority = Priority;
  // put us into the Initial PseudoState
  CurrentState = Idle_tapeFSM;
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

/****************************************************************************
 Function
     PostTapeFSM

 Parameters
     EF_Event_t ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostTapeFSM(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunTapeFSM

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event_t, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   uses nested switch/case to implement the machine.
 Author
   J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event_t RunTapeFSM(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

  switch (CurrentState)
  {
    case Idle_tapeFSM:        
    {
      if (ThisEvent.EventType == ES_TAPE_FOLLOW)
      {
        CurrentState = Following_tapeFSM;
        enterFollowing();
      }
      
    }
    break;

    case Following_tapeFSM:        // If current state is state one
    {
      if (ThisEvent.EventType == ES_TAPE_STOP)
      {
        CurrentState = Idle_tapeFSM;
        exitFollowing();
      }
    }
    break;
    default:
    break;
  }                                   // end switch on Current State
  return ReturnEvent;
}

/****************************************************************************
 Function
     QueryTapeSM

 Parameters
     None

 Returns
     TapeState_t The current state of the Tape state machine

 Description
     returns the current state of the Tape state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:21
****************************************************************************/
TapeState_t QueryTapeFSM(void)
{
  return CurrentState;
}

/***************************************************************************
 private functions
 ***************************************************************************/
static void enterFollowing(){
  //step1: turn on the motors
  OC1RS = PR2 * initialDutyCycle/100;
  OC3RS = PR2 * initialDutyCycle/100;
  // step2: start control ISR
  T4CONbits.ON = 1;
  return;
}
static void exitFollowing(){
  //step1: turn off the motors
  H_bridge1A_LAT = 0;
  H_bridge3A_LAT = 0;
  Dir0 = 0;
  Dir1 = 0;
  OC1RS = 0;
  OC3RS = 0;
  // step2: stop control ISR
  T4CONbits.ON = 0;
  // step3: clean error integral
  K_error_sum = 0;
  return;
}
static void ConfigTimer2() {
  // Clear the ON control bit to disable the timer. 
  T2CONbits.ON = 0;
  // Clear the TCS control bit to select the internal PBCLK source.
  T2CONbits.TCS = 0;
  // Set input clock prescale to be 1:1. 
  T2CONbits.TCKPS = 0b000; 
  // Clear the timer register TMR2 
  TMR2 = 0;
  // Load PR2 with desired 16-bit match value
  PR2 = PIC_freq / (PWM_freq * prescalar_T2) - 1;
  DB_printf("PR2 is set to %d \n", PR2);

  // Clear the T2IF interrupt flag bit in the IFS2 register
  IFS0CLR = _IFS0_T2IF_MASK;
  // Disable interrupts on Timer 2
  IEC0CLR = _IEC0_T2IE_MASK;
  // Hold on to turn on the timer
  // T2CONbits.ON = 1;
  return; 
}
static void ConfigPWM_OC1() {

    // map OC1 to RB3
    RPB3R = 0b0101;

    //Clear OC1CON register: 
    OC1CON = 0;

    // Configure the Output Compare module for one of two PWM operation modes
    OC1CONbits.ON = 0;         // Turn off Output Compare module
    OC1CONbits.OCM = 0b110;    // PWM mode without fault pin
    OC1CONbits.OCTSEL = 0;     // Use Timer2 as the time base

    // Set the PWM duty cycle by writing to the OCxRS register
    OC1RS = PR2 * 0;       // Secondary Compare Register (for duty cycle)
    OC1R = PR2 * 0;        // Primary Compare Register (initial value)

    // Turn on Timer 2 after OC2 setup
    T2CONbits.ON = 1;
    // Turn ON the Output Compare module
    OC1CONbits.ON = 1;         // Enable Output Compare module

    return;
}


static void ConfigPWM_OC3() {

    // map OC3 to RB10
    RPB10R = 0b0101;

    //Clear OC3CON register: 
    OC3CON = 0;

    // Configure the Output Compare module for one of two PWM operation modes
    OC3CONbits.ON = 0;         // Turn off Output Compare module
    OC3CONbits.OCM = 0b110;    // PWM mode without fault pin
    OC3CONbits.OCTSEL = 0;     // Use Timer2 as the time base

    // Set the PWM duty cycle by writing to the OCxRS register
    OC3RS = PR2 * 0;       // Secondary Compare Register (for duty cycle)
    OC3R = PR2 * 0;        // Primary Compare Register (initial value)

    // // Turn on Timer 2 after OC2 setup
    // T2CONbits.ON = 1;
    // Turn ON the Output Compare module
    OC3CONbits.ON = 1;         // Enable Output Compare module

    return;
}


static void ConfigTimer3() {
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

static void ConfigTimer4() {
  //choose Timer4 for control ISR
// Disable timer4 before configuring
 T4CONbits.ON = 0;
 // Choose the internal clock as the source
 T4CONbits.TCS = 0;
 // Disable external gating
 T4CONbits.TGATE = 0;
 // Set the prescalar to 4
 T4CONbits.TCKPS = 0b010; 
 // set the period according to the ck frequency and pre-scalar to get 200Hz
PR4 = Control_interval*1000000/ns_per_tick/prescalar_T4 - 1;
DB_printf("PR4 is set to %d \n",PR4);
//clear the current timer count
TMR4=0;
T4CONbits.ON = 1;
//enable interrupt for the timer
//clear the interrupt flag for TIMER4
IFS0CLR = _IFS0_T4IF_MASK;
// Set the priority of the interrupt for TIMER4
IPC4bits.T4IP = priority_control;
// Enable interrupt for TIMER4
IEC0SET = _IEC0_T4IE_MASK; 

  return;
}
static void ConfigureReflectSensor(){
  static uint32_t CurrADVal[6];//6 sensors

  //Sensors' 0-5 ports: A0, A1, B12, B13, B15, B2
  ANSELAbits.ANSA0 = 1; // Configure A0 as analog IO
  TRISAbits.TRISA0 = 0; // Configure A0 as output
  ADC_ConfigAutoScan(BIT0HI);// AN0/RA0
  ANSELAbits.ANSA1 = 1; // Configure A1 as analog IO
  TRISAbits.TRISA1 = 0; // Configure A1 as output
  ADC_ConfigAutoScan(BIT1HI);// AN0/RA0
  ANSELBbits.ANSB12 = 1; // Configure RB12 as analog IO
  TRISBbits.TRISB12 = 0; // Configure RB12 as output
  ADC_ConfigAutoScan(BIT12HI);// AN12/RB12
  ANSELBbits.ANSB13 = 1; // Configure RB13 as analog IO
  TRISBbits.TRISB13 = 0; // Configure RB13 as output
  ADC_ConfigAutoScan(BIT13HI);// AN13/RB13
  ANSELBbits.ANSB15 = 1; // Configure RB15 as analog IO
  TRISBbits.TRISB15 = 0; // Configure RB15 as output
  ADC_ConfigAutoScan(BIT15HI);// AN15/RB15
  ANSELBbits.ANSB2 = 1; // Configure RB2 as analog IO
  TRISBbits.TRISB2 = 0; // Configure RB2 as output
  ADC_ConfigAutoScan(BIT2HI);// AN2/RB2
  

  return;
}
/***********************
 * ******ISR*************************
*/
void __ISR(_TIMER_4_VECTOR, IPL5SOFT) control_update_ISR(void) {
  
    IFS0CLR = _IFS0_T4IF_MASK;// Clear the Timer 4 interrupt flag
    K_error = 0;

    ADC_MultiRead(CurrADVal);
    DB_printf("%d %d %d %d %d %d \n",CurrADVal[0],CurrADVal[1],CurrADVal[2],CurrADVal[3],CurrADVal[4],CurrADVal[5]);
    // //anti-windup
    // if (control_effort<PR2 && control_effort > 2)
    // {
    //   error_sum+= error_RPM100/10;
    // }
    // control_effort = (float)error_RPM100/3*Kp + (float)error_sum/30*Ki;
    // if (control_effort > 999)
    // {
    //   control_effort = PR2;
    // }else if (control_effort < 2)
    // {
    //   control_effort = 2;
    // }
    // OC1RS = control_effort;
    
}