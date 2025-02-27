/****************************************************************************
 Module
   MotorService.c

 Revision
   1.0.1

 Description
  The service initializes the motor and accepts commands to move the motor to make the cart move forward, 
  backward, or rotate.
  EventParam passed to this service is the duty cycle of the motor, which is the speed of the motor.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/16/12 09:58 jec      began conversion from MotorFSM.c
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "MotorService.h"

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
// the configure timer functions don't turn on the timer, they just configure the timer
static void ConfigTimer2(void); // time base for OC,
// the configure PWM functions turns on OC at the end: OCxCON.ON = 1
static void ConfigPWM_OC4(void);
static void ConfigPWM_OC3(void);
static void StopMotor(void);
/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
#define PWM_freq 10000 // wheel motor PWM frquency in Hz
#define PIC_freq 20000000
#define PIC_freq_kHz 20000
#define ns_per_tick 50 // nano-sec per tick for the PBC = 1/PIC_freq
#define H_bridge1A_TRIS TRISBbits.TRISB5 // output
#define H_bridge1A_LAT LATBbits.LATB5    // latch
#define H_bridge3A_TRIS TRISBbits.TRISB9  // output
#define H_bridge3A_LAT LATBbits.LATB9     // latch
// prescalars and priorities
#define prescalar_T2 2  // for PWM for the 2 motors
#define Turn90TIME 1300 // time to turn 90 degrees in ms
static uint8_t MyPriority;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitMotorService

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any
     other required initialization for this service
 Notes

 Author
     J. Edward Carryer, 01/16/12, 10:00
****************************************************************************/
bool InitMotorService(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  /********************************************
   in here you write your initialization code
   *******************************************/
    // TRIS and LAT for direction control pins
  H_bridge1A_TRIS = 0; // Outputs
  H_bridge1A_LAT = 0;
  H_bridge3A_TRIS = 0; // Outputs
  H_bridge3A_LAT = 0;

  ConfigTimer2();
  // OC4 and OC3 use Timer2 as the time base
  ConfigPWM_OC4();
  ConfigPWM_OC3();
  // post the initial transition event
  ThisEvent.EventType = ES_INIT;
  DB_printf("MotorService initialized \n");
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
     PostMotorService

 Parameters
     EF_Event_t ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostMotorService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunMotorService

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes

 Author
   J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event_t RunMotorService(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  switch (ThisEvent.EventType)
  {
  case ES_TIMEOUT:
    if (ThisEvent.EventParam == Motor_Turning_TIMER)//meaning that turning has completed
    {
      StopMotor();
    }
    break;
  case ES_MOTOR_STOP:
    StopMotor();
    break;
  case ES_MOTOR_FWD:
    H_bridge1A_LAT = 0;
    H_bridge3A_LAT = 0;
    OC4RS = (float)PR2 * ThisEvent.EventParam /100;
    OC3RS = (float)PR2 * ThisEvent.EventParam /100;
    break;
  case ES_MOTOR_REV:
    H_bridge1A_LAT = 1;
    H_bridge3A_LAT = 1;
    OC4RS = (float)PR2 * (100 - ThisEvent.EventParam) /100;
    OC3RS = (float)PR2 * (100 - ThisEvent.EventParam) /100;
    break;
  case ES_MOTOR_CW_CONTINUOUS:
    H_bridge1A_LAT = 1;
    H_bridge3A_LAT = 0;
    OC4RS = (float)PR2 * (100 - ThisEvent.EventParam) /100;
    OC3RS = (float)PR2 * ThisEvent.EventParam /100;
    break;
  case ES_MOTOR_CCW_CONTINUOUS:

    H_bridge1A_LAT = 0;
    H_bridge3A_LAT = 1;
    OC4RS = (float)PR2 * ThisEvent.EventParam /100;
    OC3RS = (float)PR2 * (100 - ThisEvent.EventParam) /100;
    break;
  case ES_MOTOR_CW90:
    ES_Timer_InitTimer(Motor_Turning_TIMER, Turn90TIME);
    H_bridge1A_LAT = 0;
    H_bridge3A_LAT = 1;
    OC4RS = (float)PR2 * 80 /100;
    OC3RS = (float)PR2 * (100 - 80) /100;
    break;
  case ES_MOTOR_CW180:
  break;
  case ES_MOTOR_CCW90:
  break;
  case ES_MOTOR_CCW180:
  break;
  default:
    break;
  }
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/
static void ConfigTimer2()
{
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
  return;
}
static void ConfigPWM_OC4() 
{

  // map OC4 to RB14
  RPA4R = 0b0101;

  // Clear OC4CON register:
  OC4CON = 0;

  // Configure the Output Compare module for one of two PWM operation modes
  OC4CONbits.ON = 0;      // Turn off Output Compare module
  OC4CONbits.OCM = 0b110; // PWM mode without fault pin
  OC4CONbits.OCTSEL = 0;  // Use Timer2 as the time base

  // Set the PWM duty cycle by writing to the OCxRS register
  OC4RS = PR2 * 0; // Secondary Compare Register (for duty cycle)
  OC4R = PR2 * 0;  // Primary Compare Register (initial value)


  // Turn ON the Output Compare module
  OC4CONbits.ON = 1; // Enable Output Compare module

  return;
}

static void ConfigPWM_OC3()
{

  // map OC3 to RB10
  RPB10R = 0b0101;

  // Clear OC3CON register:
  OC3CON = 0;

  // Configure the Output Compare module for one of two PWM operation modes
  OC3CONbits.ON = 0;      // Turn off Output Compare module
  OC3CONbits.OCM = 0b110; // PWM mode without fault pin
  OC3CONbits.OCTSEL = 0;  // Use Timer2 as the time base

  // Set the PWM duty cycle by writing to the OCxRS register
  OC3RS = PR2 * 0; // Secondary Compare Register (for duty cycle)
  OC3R = PR2 * 0;  // Primary Compare Register (initial value)

  // Turn ON the Output Compare module
  OC3CONbits.ON = 1; // Enable Output Compare module

  return;
}
static void StopMotor()
{
  H_bridge1A_LAT = 0;
  H_bridge3A_LAT = 0;
  OC4RS = 0;
  OC3RS = 0;
}
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

