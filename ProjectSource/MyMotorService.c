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
    case ES_FORWARD:
    {   
      DB_printf("\rFSFSDFFDFDSFDSd\r\n");
      LATBbits.LATB2 = 0;
      LATBbits.LATB9 = 0;
      dutyCycle = ThisEvent.EventParam;
      OC1RS = (PR2+1) * dutyCycle/100;       // Secondary Compare Register (for duty cycle)
      OC3RS = (PR2+1) * dutyCycle/100;
    }
    break;
    
    case ES_BACKWARD:
    {
        
        DB_printf("\rFSFSDFDSFDSd\r\n");
        LATBbits.LATB2 = 1;
        LATBbits.LATB9 = 1;
        dutyCycle = 100-ThisEvent.EventParam;
        OC1RS = (PR2+1) * dutyCycle/100;       // Secondary Compare Register (for duty cycle)
        OC3RS = (PR2+1) * dutyCycle/100;
    }
    break;
    
    case ES_LEFT:
    {
        
        dutyCycle = ThisEvent.EventParam;
        OC1RS = 0;
        OC3RS = (PR2+1) * dutyCycle/100;
    }
    break;
    
    case ES_RIGHT:
    {
        dutyCycle = ThisEvent.EventParam;
        OC3RS = 0;
        OC1RS = (PR2+1) * dutyCycle/100;
    }
    break;
    
    case ES_STOP:
    {
        OC1RS = (PR2+1) *100*LATBbits.LATB2;
        OC3RS = OC1RS;
    }
    break;
    
    default:
    {}
    break;
  }
  /*Adjust Speed here*/
  
//  if (ThisEvent.EventType == ES_Read_Pot) {
//    // DCtick = PR2 * ThisEvent.EventParam / 1023;
//    DCtick = PR2 * (1023 * 1) / 1023;
//    DB_printf("\r POT:  %d\r\n", ThisEvent.EventParam);
//    DB_printf("\r DCtick:   %d\r\n", DCtick);
//    DB_printf("\r PR2:   %d\r\n", PR2);
//  }
  
//    OC1RS = PR2 * 0.9;       // Secondary Compare Register (for duty cycle)
//    OC3RS = PR2 * 0.4;
//    
//    OC1R = PR2 * 0.2;  
  // switch (CurrentState) {
  //   case NOT_INIT:
  //   {
  //     if (ThisEvent.EventType == ES_INIT) { 
  //       DB_printf("\rES_INIT received in Service %d\r\n", MyPriority); // should print ES_INIT received in Service 0
  //       // Your code goes here
  //       CurrentState = CWRot;
  //     }
  //   }
  //   break;
  //   case CWRot:
  //   {
  //     puts("\rCurrent Direction is CW\r\n"); 
  //     if (ThisEvent.EventType == ES_BUTTON_PRESSED) {
  //       puts("\rSwitch from CW to CWW\r\n"); 
  //       LATAbits.LATA0 = 1;
  //       OC1RS = PR2 - DCtick;
  //       CurrentState = CCWRot;
  //     }
  //     else {
  //       OC1RS = DCtick;
  //     }
      
  //   }
  //   break;
  //   case CCWRot:
  //   { 
  //     puts("\rCurrent Direction is CCW\r\n"); 
  //     if (ThisEvent.EventType == ES_BUTTON_PRESSED) {
  //       puts("\rSwitch from CWW to CW\r\n"); 
  //       LATAbits.LATA0 = 0;
  //       OC1RS = DCtick;
  //       CurrentState = CWRot;
  //     }
  //     else {
  //       OC1RS = PR2 - DCtick;
  //     }
  //   }
  //   break;

  //   case Pause:
  //   {
  //   }
  //   break;

  //   default:
  //   {}
  //   break;

  // }
  
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

void MotorCommand(uint16_t WhichMove)
{
//    // array structure goes WhichMove [OC2RS, OC2 polarity, OC3RS, OC3 polarity, time]
//    
//    // start at no speed
//    OC2RS = 0;
//    OC3RS = 0;
//    
//    // send commands to left side motor
//    if (MotorSettings[WhichMove][3] == 0) // if going clockwise
//    {
//        OC3RS = MotorSettings[WhichMove][2]; // set pwm normally
//        LATBbits.LATB12 = 0; // set polarity pin
//    }
//    else
//    {
//        OC3RS = (TimerPeriod - MotorSettings[WhichMove][2]); // inverse pwm
//        LATBbits.LATB12 = 1; // set polarity pin
//    }
//    
//    
//    // send PWM commands to right side motor
//    if (MotorSettings[WhichMove][1] == 0) // if going clockwise
//    {
//        OC2RS = (TimerPeriod - MotorSettings[WhichMove][0]); // inverse pwm
//        LATBbits.LATB10 = 1; // set polarity pin
//    }
//    else
//    {
//        OC2RS = MotorSettings[WhichMove][0]; // set pwm normally
//        LATBbits.LATB10 = 0; // set polarity pin
//    }
//    
//    // if we need to start a timer
//    if (MotorSettings[WhichMove][4] != 0)
//    {
//        uint16_t time = MotorSettings[WhichMove][4];
//        ES_Timer_InitTimer(MOTOR_TIMER, time);
//    }
    
}
