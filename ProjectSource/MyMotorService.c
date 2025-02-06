/****************************************************************************
 Module
   TemplateService.c

 Revision
   1.0.1

 Description
   This is a template file for implementing a simple service under the
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/16/12 09:58 jec      began conversion from TemplateFSM.c
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/

//#include "../ProjectHeaders/ADService.h"
#include "../ProjectHeaders/MyMotorService.h"
#include "../ProjectHeaders/PIC32_AD_Lib.h"

// #include "ADService.h"

// Hardware
#include <xc.h>
//#include <proc/p32mx170f256b.h>

// Event & Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_Port.h"
#include "terminal.h"
#include "dbprintf.h"
/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
// these times assume a 10.000mS/tick timing
#define ONE_SEC 1000
#define TENTH_SEC (ONE_SEC * 0.1)

#define ELEC_TIME_CONS 148 // in microsec
#define PWM_FREQ 6756 // Set PWM freq at least 1/tao, where tao was measured to be 148 microsec
#define PIC_FREQ 20000000 // PIC 20MHz
#define TIMER2_PRESCALE 1 // Assume 1:1 prescale value

static uint8_t MyPriority;
static MotorState_t CurrentState; 
// static uint32_t ConversionRatio; 
static uint32_t DCtick; 
static float DC; 


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitTemplateService

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
  clrScrn();
  puts("\rStarting Motor Service for \r");
  DB_printf( "the 2nd Generation Events & Services Framework V2.4\r\n");
  DB_printf( "compiled at %s on %s\n", __TIME__, __DATE__);
  DB_printf( "\n\r\n");

  /* Set RB2, RB9 to be digital outputs */
  TRISBbits.TRISB2 = 0;
  ANSELBbits.ANSB2 = 0;

  TRISBbits.TRISB9 = 0;
//  ANSELAbits.ANSB9 = 0;

  //TRISAbits.TRISA2 = 0;

  // Configure Timer2
  ConfigTimer2();
  // Configure OC1 & OC3
  ConfigPWM_OC1();
  ConfigPWM_OC3();

  // Initialize Motor Pin state
  LATBbits.LATB2 = 0;
  LATBbits.LATB10 = 0;
  LATBbits.LATB3 = 0;
  LATBbits.LATB9 = 0;

  
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  /********************************************
   in here you write your initialization code
   *******************************************/
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
     PostTemplateService

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
    RunTemplateService

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
  uint8_t dutyCycle;
  
  switch(ThisEvent.EventType){
    case ES_TIMEOUT:
    {
      ES_Event_t myEvent;
      myEvent.EventType = ES_STOP;
      PostMotorService(myEvent);
    }
    case ES_FWDFULL:
    {   
      puts("\rMotor: Moving forward at full speed\r\n");
      LATBbits.LATB2 = 0;
      LATBbits.LATB9 = 0;
      //dutyCycle = ThisEvent.EventParam;
      dutyCycle = 100; 
      OC1RS = (PR2+1) * dutyCycle/100;       // Secondary Compare Register (for duty cycle)
      OC3RS = (PR2+1) * dutyCycle/100;
    }
    break;
    case ES_FWDHALF:
    {   
      puts("\rMotor: Moving forward at half speed\r\n");
      LATBbits.LATB2 = 0;
      LATBbits.LATB9 = 0;
      //dutyCycle = ThisEvent.EventParam;
      dutyCycle = 60;
      OC1RS = (PR2+1) * dutyCycle/100;       // Secondary Compare Register (for duty cycle)
      OC3RS = (PR2+1) * dutyCycle/100;
    }
    break;
    
    case ES_BWDFULL:
    {
      puts("\rMotor: Moving Backward at full speed\r\n");
      LATBbits.LATB2 = 1;
      LATBbits.LATB9 = 1;
      //dutyCycle = 100-ThisEvent.EventParam;
      dutyCycle = 0; 
      OC1RS = (PR2+1) * dutyCycle/100;       // Secondary Compare Register (for duty cycle)
      OC3RS = (PR2+1) * dutyCycle/100;
    }
    break;

    case ES_BWDHALF:
    {
      puts("\rMotor: Moving Backward at half speed\r\n");
      LATBbits.LATB2 = 1;
      LATBbits.LATB9 = 1;
      //dutyCycle = 100-ThisEvent.EventParam;
      dutyCycle = 40;
      OC1RS = (PR2+1) * dutyCycle/100;       // Secondary Compare Register (for duty cycle)
      OC3RS = (PR2+1) * dutyCycle/100;
    }
    break;
    
    case ES_LEFT90:
    {
        puts("\rMotor: Turning Left 90\r\n");
        LATBbits.LATB2 = 1;
        LATBbits.LATB9 = 0;
        // dutyCycle = ThisEvent.EventParam;
        dutyCycle = 100;
        OC1RS = (PR2+1) * (100-dutyCycle)/100;
        OC3RS = (PR2+1) * dutyCycle/100;
        ES_Timer_InitTimer(MOTOR_TIMER, 2000); // 
    }
    break;

    case ES_LEFT45:
    {
        puts("\rMotor: Turning Left 45\r\n");
        LATBbits.LATB2 = 1;
        LATBbits.LATB9 = 0;
        // dutyCycle = ThisEvent.EventParam;
        dutyCycle = 100;
        OC1RS = (PR2+1) * (100-dutyCycle)/100;
        OC3RS = (PR2+1) * dutyCycle/100;
        ES_Timer_InitTimer(MOTOR_TIMER, 1000);
    }
    break;
    
    case ES_RIGHT90:
    {
        puts("\rMotor: Turning Right 90\r\n");
        LATBbits.LATB2 = 0;
        LATBbits.LATB9 = 1;
        // dutyCycle = ThisEvent.EventParam;
        dutyCycle = 100;
        OC3RS = (PR2+1) * (100-dutyCycle)/100;
        OC1RS = (PR2+1) * dutyCycle/100;
        ES_Timer_InitTimer(MOTOR_TIMER, 2000);
    }
    break;

    case ES_RIGHT45:
    {
        puts("\rMotor: Turning Right 45\r\n");
        LATBbits.LATB2 = 0;
        LATBbits.LATB9 = 1;
        // dutyCycle = ThisEvent.EventParam;
        dutyCycle = 100;
        OC3RS = (PR2+1) * (100-dutyCycle)/100;
        OC1RS = (PR2+1) * dutyCycle/100;
        ES_Timer_InitTimer(MOTOR_TIMER, 1000);
    }
    break;
    
    case ES_STOP:
    {
        puts("\rMotor: Action Stop or Turning Complete\r\n");
        LATBbits.LATB2 = 0;
        LATBbits.LATB9 = 0;
        OC1RS = 0;
        OC3RS = 0;
    }
    break;
    /*
    case ES_CW360:
        puts("\rMotor: Turning CW 360 degree\r\n");
        LATBbits.LATB2 = 1;
        LATBbits.LATB9 = 0;
        // dutyCycle = ThisEvent.EventParam;
        dutyCycle = 100;
        OC1RS = (PR2+1) * (100-dutyCycle)/100;
        OC3RS = (PR2+1) * dutyCycle/100;

    break;
    */
    default:
    {}
    break;
  }
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/
void ConfigTimer2() {
  // Clear the ON control bit to disable the timer. 
  T2CONbits.ON = 0;
  // Clear the TCS control bit to select the internal PBCLK source.
  T2CONbits.TCS = 0;
  // Set input clock prescale to be 1:1. 
  T2CONbits.TCKPS = 0b000; 
  // Clear the timer register TMR2 
  TMR2 = 0;
  // Load PR2 with desired 16-bit match value
  PR2 = PIC_FREQ / (PWM_FREQ * TIMER2_PRESCALE) - 1;
  DB_printf("PR2 is set to %d \n", PR2);

  // Clear the T2IF interrupt flag bit in the IFS2 register
  IFS0CLR = _IFS0_T2IF_MASK;
  // Disable interrupts on Timer 2
  IEC0CLR = _IEC0_T2IE_MASK;
  // Hold on to turn on the timer
  // T2CONbits.ON = 1;
  return; 
}
void ConfigPWM_OC1() {

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


void ConfigPWM_OC3() {

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

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
