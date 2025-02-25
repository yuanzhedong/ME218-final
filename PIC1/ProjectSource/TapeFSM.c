/****************************************************************************
 Module
   TapeFSM.c

 Revision
   1.0.1

 Description
   This file doesn't initialize OC for motors
   This file initializes the ADC for the reflectance sensor array

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
#include "PIC32_AD_Lib.h"
/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/
static void enterFollowing(int Dir_input, uint16_t targetDutyCycle_input); // the entry function for entering the Following state from Idle_tapeFSM
static void exitFollowing(void);           // the exit function for exiting the Following state back to Idle_tapeFSM

// the configure timer functions don't turn on the timer, they just configure the timer
static void ConfigTimer3(void);
static void ConfigTimer4(void);       // for running control loop
static void ConfigureReflectSensor(); // for the reflectance sensor array

static void AdjustKvalues(uint16_t targetDutyCycle_input);      // adjust K values based on the targetDutyCycle
/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file

#define PIC_freq 20000000
#define PIC_freq_kHz 20000
#define ns_per_tick 50 // nano-sec per tick for the PBC = 1/PIC_freq

#define H_bridge1A_TRIS TRISBbits.TRISB5 // output
#define H_bridge1A_LAT LATBbits.LATB5    // latch
#define H_bridge3A_TRIS TRISBbits.TRISB9  // output
#define H_bridge3A_LAT LATBbits.LATB9     // latch

// prescalars and priorities
#define prescalar_T4 16 // used for running control law
#define priority_control 5

// output compare stuff (PWM)
static uint8_t Dir = 0; // the direction of the motor, 0 = forward, 1 = backward

// control stuff
#define Control_interval 10 // in ms, max value with prescalar of 16 is 65535*16/20MHz = 52.4288ms
//the K values used will be scaled based on commanded targetDutyCycle
//because when the cart is moving faster, K values should be smaller to avoid aggressive control
#define Kp_base 500
#define Ki_base 300
#define Kd_base 0
static uint16_t targetDutyCycle;    // the duty cycle of the motor as passed by the eventparam
//static uint16_t targetOC_ticks; //calculated based on PR2 and the targetDutyCycle
static uint8_t sensorWeights[] = {4, 2, 4, 4, 2, 4}; // weights for the 6 sensors from left to right of sensor array
// K_error is the error in the sensor readings, K_effort is the control effort
volatile int16_t K_error = 0;
volatile int16_t K_error_prev = 0; //for Derivative term
static int16_t K_error_max;
static int16_t K_error_min;
volatile float K_error_sum = 0;
static uint16_t K_error_sum_division_factor; // this factor increases with the frequecy of the control loop to prevent K_error_sum from getting too large
// K_effort is in the units of duty cycle with max = PR2*targetDutyCycle/100
// min = -PR2*targetDutyCycle/100
volatile int16_t K_effort = 0;
static int16_t K_effort_max;
static int16_t K_effort_min;
static uint32_t CurrADVal[6];
// framework stuff
static uint8_t MyPriority;
static TapeState_t CurrentState;
// FSM stuff
static bool moveAllowed = false;
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

  ConfigureReflectSensor(); // this calls the ADC library

  T2CONbits.ON = 1;
  ConfigTimer4();
  // We do not yet turn on T4 because the initial state is Idle state
  // T4CONbits.ON = 1;

  /********enable interrupt globally *******************/
  __builtin_enable_interrupts();
  K_error_max = 1023 * (sensorWeights[0] + sensorWeights[1] + sensorWeights[2]);
  K_error_min = -1023 * (sensorWeights[3] + sensorWeights[4] + sensorWeights[5]);
  
  K_error_sum_division_factor = (float)1000 / Control_interval;
  DB_printf("K_error_sum_division_factor: %d\n", K_error_sum_division_factor);
  ES_Timer_InitTimer(TapeTest_TIMER, 3000);
  ES_Event_t ThisEvent;
  MyPriority = Priority;
  // put us into the Initial PseudoState
  CurrentState = Idle_tapeFSM;
  // post the initial transition event
  ThisEvent.EventType = ES_INIT;
  DB_printf("TapeFSM initialized \n");
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
  if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == TapeTest_TIMER)
  {
    //ES_Timer_InitTimer(TapeTest_TIMER, 1000);
    DB_printf("Tape Test Timer\r\n");
    ES_Event_t Event2Post;
    Event2Post.EventType = ES_TAPE_FOLLOW_REV;
    Event2Post.EventParam = 100;
    //PostTapeFSM(Event2Post);
    // ADC_MultiRead(CurrADVal);
    // DB_printf("%d %d %d  %d %d %d\r\n", CurrADVal[0], CurrADVal[1], CurrADVal[2], CurrADVal[3], CurrADVal[4], CurrADVal[5]);
    // Event2Post.EventType = ES_MOTOR_CW_CONTINUOUS;
    // Event2Post.EventParam = 70;
    //PostMotorService(Event2Post);
  }

  switch (CurrentState)
  {
  case Idle_tapeFSM:
  {
    if (ThisEvent.EventType == ES_TAPE_FOLLOW_FWD)
    {
      CurrentState = Following_tapeFSM;
      enterFollowing(0,ThisEvent.EventParam);
    }else if (ThisEvent.EventType == ES_TAPE_FOLLOW_REV)
    {
      DB_printf("eventparam is %d \n", ThisEvent.EventParam);
      CurrentState = Following_tapeFSM;
      enterFollowing(1,ThisEvent.EventParam);
    }
    
  }
  break;

  case Following_tapeFSM: // If current state is state one
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
  } // end switch on Current State
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
static void enterFollowing(int Dir_input, uint16_t targetDutyCycle_input)
{
  // step1: allow motor movement
  moveAllowed = true;
  // step2: set the direction of the motors
  Dir = Dir_input;
  // step3: get the duty cycle from the eventparam
  targetDutyCycle = targetDutyCycle_input;
  K_effort_max = (float)PR2 * targetDutyCycle / 100; // PR2 is the max value for OCxRS
  K_effort_min = -K_effort_max;
  //targetOC_ticks = (float)targetDutyCycle / 100 * PR2; //same as K_effort_max so don't need this
  //DB_printf("targetOC_ticks: %d\n", targetOC_ticks);
  DB_printf("K_effort_max: %d\n", K_effort_max);
  DB_printf("K_effort_min: %d\n", K_effort_min);
  DB_printf("targetDutyCycle is %d \n", targetDutyCycle);
  //step4: set the K values based on the targetDutyCycle
  AdjustKvalues(targetDutyCycle);
  // step5: start the motor

  if (Dir == 0)
  {
    H_bridge1A_LAT = 0;
    H_bridge3A_LAT = 0;
    OC4RS = (float)targetDutyCycle / 100 * PR2;
    OC3RS = (float)targetDutyCycle / 100 * PR2;
    DB_printf("dir is 0 \n");
    DB_printf("OC4RS is %d \n", OC4RS);
    DB_printf("OC3RS is %d \n", OC3RS);
  }
  else if (Dir == 1)
  {
    H_bridge1A_LAT = 1;
    H_bridge3A_LAT = 1;
    OC4RS = (float)(100 - targetDutyCycle) / 100 * PR2;
    OC3RS = (float)(100 - targetDutyCycle) / 100 * PR2;
    DB_printf("dir is 1 \n");
    DB_printf("OC4RS is %d \n", OC4RS);
    DB_printf("OC3RS is %d \n", OC3RS);

  }
//step6: start the control ISR
  T4CONbits.ON = 1;
  return;
}
static void exitFollowing()
{
  // step1: turn off the motors
  moveAllowed = false;
  H_bridge1A_LAT = 0;
  H_bridge3A_LAT = 0;
  OC4RS = 0;
  OC3RS = 0;
  // step2: stop control ISR
  T4CONbits.ON = 0;
  // step3: clean error integral
  K_error_sum = 0;
  return;
}



static void ConfigTimer3()
{
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
  return;
}

static void ConfigTimer4()
{
  // choose Timer4 for control ISR
  // Disable timer4 before configuring
  T4CONbits.ON = 0;
  // Choose the internal clock as the source
  T4CONbits.TCS = 0;
  // Disable external gating
  T4CONbits.TGATE = 0;
  // Set the prescalar to 16
  T4CONbits.TCKPS = 0b100;
  // set the period according to the ck frequency and pre-scalar to get 200Hz
  PR4 = Control_interval * 1000000 / ns_per_tick / prescalar_T4 - 1;
  DB_printf("PR4 is set to %d \n", PR4);
  // clear the current timer count
  TMR4 = 0;
  // enable interrupt for the timer
  // clear the interrupt flag for TIMER4
  IFS0CLR = _IFS0_T4IF_MASK;
  // Set the priority of the interrupt for TIMER4
  IPC4bits.T4IP = priority_control;
  // Enable interrupt for TIMER4
  IEC0SET = _IEC0_T4IE_MASK;

  return;
}
static void ConfigureReflectSensor()
{

  // Sensors' 0-5 ports: AN0/RA0, AN1/RA1, AN4/RB2, AN9/RB15, AN11/RB13, AN12/RB12
ANSELAbits.ANSA0 = 1; // set RA0 as analog
TRISAbits.TRISA0 = 1; // set RA0 as input
ANSELAbits.ANSA1 = 1; // set RA1 as analog
TRISAbits.TRISA1 = 1; // set RA1 as input
ANSELBbits.ANSB2 = 1; // set RB2 as analog
TRISBbits.TRISB2 = 1; // set RB2 as input
ANSELBbits.ANSB3 = 1; // set RB3 as analog
TRISBbits.TRISB3 = 1; // set RB3 as input
ANSELBbits.ANSB13 = 1; // set RB13 as analog
TRISBbits.TRISB13 = 1; // set RB13 as input
ANSELBbits.ANSB12 = 1; // set RB12 as analog
TRISBbits.TRISB12 = 1; // set RB12 as input
  ADC_ConfigAutoScan(BIT0HI | BIT1HI | BIT4HI  | BIT5HI |BIT11HI |BIT12HI); // AN0/RA0, AN1/RA1, AN4/RB2, AN9/RB9, AN11/RB13, AN12/RB12

  return;
}
static void AdjustKvalues(uint16_t targetDutyCycle_input)
{
  //the K values used will be scaled based on commanded targetDutyCycle
  //because when the cart is moving faster, K values should be smaller to avoid aggressive control
  Kp = (float) Kp_base * 100 / targetDutyCycle_input;
  Ki = (float)Ki_base * 100 / targetDutyCycle_input;
  Kd = (float)Kd_base * 100 / targetDutyCycle_input;
  DB_printf("Kp: %d, Ki: %d, Kd: %d \n", Kp, Ki, Kd);
  return;
}
/***********************
 * ******ISR*************************
 */
void __ISR(_TIMER_4_VECTOR, IPL5SOFT) control_update_ISR(void)
{
  IFS0CLR = _IFS0_T4IF_MASK;             // Clear the Timer 4 interrupt flag
  static uint16_t K_commandedOC4; // OC4RS commanded for left motor as determined appropriate by the control law
  static uint16_t K_commandedOC3;
  ADC_MultiRead(CurrADVal);
  // DB_printf("T4 ISR entered \n");
  //DB_printf("%d %d %d  %d %d %d\r\n", CurrADVal[0], CurrADVal[1], CurrADVal[2], CurrADVal[3], CurrADVal[4], CurrADVal[5]);
  K_error = CurrADVal[0]*sensorWeights[0] + CurrADVal[1]*sensorWeights[1] + CurrADVal[2]*sensorWeights[2] - CurrADVal[3]*sensorWeights[3] - CurrADVal[4]*sensorWeights[4] - CurrADVal[5]*sensorWeights[5];

 // If K_error changed sign, clear the integral term
  if (K_error * K_error_prev < 0)
  {
    K_error_sum = 0;
  }
  
  //this temp variable is for preventing the overflow of K_effort
  static int K_effort_temp;
  K_effort_temp = (float)K_error / K_error_max * Kp + (float)K_error_sum / K_error_max * Ki + (float)(K_error - K_error_prev) / K_error_max * Kd;
  K_effort = 0;
  // anti-windup
  if (K_effort_temp < K_effort_max && K_effort_temp > K_effort_min)
  {
    //DB_printf("error sum is summing \n");
    K_effort = K_effort_temp;
    K_error_sum += (float)K_error / K_error_sum_division_factor; // K_error_sum is a float
  }
  K_error_prev = K_error;

  
  // OC4RS is the left motor and OC3RS is the right motor
  
  switch (Dir)
  {
  case 0://meaning we are moving forward
  if (K_effort >= 0)
    {
    // if K_effort is positive, that means the sensors on the left read more black than the right
    //Cart has to turn right
    // so the right motor should slow down
      K_commandedOC4 =  K_effort_max;
      K_commandedOC3 = K_effort_max - K_effort ;
    }
    else if (K_effort < 0)
    {
      // K_effort is negative, that means the sensors on the right read more black than the left
      // so the left motor should slow down
      K_commandedOC4 = K_effort_max + K_effort;
      K_commandedOC3 = K_effort_max;
    }
  break;
  case 1://meaning we are moving backward
    if (K_effort >= 0)
    {
      K_commandedOC4 =  K_effort_max - K_effort;
      K_commandedOC3 = K_effort_max ;
    }
    else if (K_effort < 0)
    {
      // K_effort is negative, that means the sensors on the right read more black than the left
      // so the left motor should slow down
      K_commandedOC4 = K_effort_max;
      K_commandedOC3 = K_effort_max  + K_effort;
    }
    break;
  default:
    break;
  }

  DB_printf("K_error: %d,K_error_sum: %d, K_effort: %d, OC4: %d, OC3: %d \n", K_error, (int)K_error_sum, K_effort, K_commandedOC4, K_commandedOC3);
//DB_printf("K_effort_max: %d, K_effort_min: %d, OC4: %d, OC3: %d \n", K_effort_max, K_effort_min, K_commandedOC4, K_commandedOC3);
 //actuate the motors 
 if (moveAllowed){
  //DB_printf("move is allowed, commanding the motors \n");
  switch (Dir)
  {
  case 0://meaning we are moving forward
    OC4RS = K_commandedOC4;
    OC3RS = K_commandedOC3;
    // OC4RS = (float)PR2 * targetDutyCycle / 100;
    // OC3RS = (float)PR2 * targetDutyCycle / 100;
    break;
  case 1://meaning we are moving backward
    OC4RS = PR2 - K_commandedOC4;
    OC3RS = PR2 - K_commandedOC3;
    // OC4RS = (float)PR2 * (100 - targetDutyCycle) / 100;
    // OC3RS = (float)PR2 * (100 - targetDutyCycle) / 100;
    break;
  default:
    break;
  }
  }
}