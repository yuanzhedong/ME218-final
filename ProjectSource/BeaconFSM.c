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
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Port.h"
#include "BeaconFSM.h"
#include "dbprintf.h"
#include <proc/p32mx170f256b.h>
#include <sys/attribs.h>
#include <stdio.h>

/*----------------------------- Module Defines ----------------------------*/
#define IO_LINE LATBbits.LATB4
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static BeaconState_t CurrentState;
static volatile uint16_t CapturedTimeIR = 0;
static volatile Period_32BitCount CurrentRecordedTimeIR;
static volatile uint32_t LastRecordedTimeIR = 0;
static volatile uint16_t PeriodLength = 0; // CurrentRecordedTime.LowByteRolloverCount + CurrentRecordedTime.ByBytes.HighByteRolloverCount
static volatile int aligned_counter = 0;
static bool Searching4Beacon = 0;
static uint16_t FirstDelta; // determines the duration of high pulse
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
bool InitBeaconFSM(uint8_t Priority)
{
  ES_Event_t ThisEvent;
  MyPriority = Priority;
  /********************************************
   in here you write your initialization code
   *******************************************/
//  clrScrn();
  puts("\r\n[Beacon FSM] Starting Beacon FSM \n\r");
  TRISBbits.TRISB11 = 1; // input
  TRISBbits.TRISB4 = 0; // IO Line Output
  IO_LINE = 0;
  // Setup Timer 3 for input capture
  T3CONbits.ON = 0;
  T3CONbits.TCS = 0;
  T3CONbits.TGATE = 0;
  T3CONbits.TCKPS = 0b000; // 1:1 Pre-scale. IR beacon flashes at 1427 Hz --> 700 us. 700 us / 50 ns = 14000 < 65535 (16 bits)
  TMR3 = 0;
  PR3 = 0xFFFF; // max - 1 ????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????
  IFS0CLR = _IFS0_T3IF_MASK; // clear interrupt flag
  IEC0SET = _IEC0_T3IE_MASK; // Enable Timer 3 interrupts
  T3CONbits.ON = 1;
  IPC3bits.T3IP = 6; // timer 3 interrupt Priority level 6 (to avoid losing encoder ticks)
//  
   //Initialize Input capture and Interrupts 
   __builtin_disable_interrupts(); // enable global interrupts
   IC3CONbits.ON=0;
   IC3Rbits.IC3R = 0b0011; // map IC3 to RB11
   IC3CONbits.ICTMR = 0; // Timer3 is the counter source for capture
   IC3CONbits.SIDL = 0; // Continue to operate in Idle mode ? let it run in idle mode
   //INTCONbits.MVEC = 1; // Multi-vector mode enabled
   IC3CONbits.ICM = 0b011; // Simple Capture Event mode - every rising edge
   IC3CONbits.C32 = 0; //  16-bit timer resource capture
   IC3CONbits.ICI = 00; //  Interrupt on every capture event
   IPC3bits.IC3IP = 7; // Interrupt priority level 7
   IEC0SET = _IEC0_IC3IE_MASK;
   IFS0CLR = _IFS0_IC3IF_MASK;
   IC3CONbits.ON = 1;
   __builtin_enable_interrupts();
  
  // post the initial transition event
  ThisEvent.EventType = ES_INIT;
  if (ES_PostToService(MyPriority, ThisEvent) == true)
  {
      CurrentState = InitPState;
      FirstDelta = 0;
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
bool PostBeaconFSM(ES_Event_t ThisEvent)
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
ES_Event_t RunBeaconFSM(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  BeaconState_t NextState = CurrentState;
//  DB_printf("Running Beacon FSM\n");
//  DB_printf("%c",CurrentState);
  /********************************************
   in here you write your service code
   *******************************************/
  switch(CurrentState)
  {
    case InitPState:        // If current state is initial Psedudo State
    {
      if (ThisEvent.EventType == ES_INIT)    // only respond to ES_Init
      {
          DB_printf("Initial Psuedo-State \r\n");
        // this is where you would put any actions associated with the
        // transition from the initial pseudo-state into the actual
        // initial state
          NextState = Waiting4Beacon;
      }
    }
    break;
    
    case Waiting4Beacon:
    {
        DB_printf("Searching4Beacon = %d\r\n", Searching4Beacon);
        DB_printf("No Beacon Detected, searching...\r\n");

        if(ThisEvent.EventType == ES_LOOKING_4_BEACON)
        {
            Searching4Beacon = 1; // enable ISR
            NextState = BeaconDetected;
            IEC0SET = _IEC0_IC3IE_MASK;
        }
    }
    break;
    
    case BeaconDetected: // Idle State
    {
        DB_printf("Beacon Detected\n");
        if(ThisEvent.EventType == ES_LOOKING_4_BEACON)
        {
            PostBeaconFSM(ThisEvent);
            NextState = Waiting4Beacon;
        }
    }
    break;
      
  }
  CurrentState = NextState;
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/
BeaconState_t QueryBeaconFSM(void)
{
  return CurrentState;
}
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
void __ISR(_TIMER_3_VECTOR, IPL6SOFT) MultiShotIntResponse(void){
    // start by clearing the source of the interrupt
    IEC0CLR = _IEC0_T3IE_MASK; // Enable Timer 3 interrupts; //disable this interrupt, so that it won't occur until after exiting ISR

    if(IFS0bits.T3IF == 1){
        ++CurrentRecordedTimeIR.ByBytes.HighByteRolloverCount;
        IFS0CLR = _IFS0_T3IF_MASK; // clear interrupt source again (in case))
    }
//    DB_printf("Entered Timer ISR\n");
    // grab the end time to be able to confirm that we got the correct delay
//    EndTime = ES_Timer_GetTime();
    IEC0SET = _IEC0_T3IE_MASK; // re-enable this interrupt
}

void __ISR(_INPUT_CAPTURE_3_VECTOR, IPL7SOFT) InputCaptureISR(void)
{
    IFS0CLR = _IFS0_IC3IF_MASK; // clear  source of interrupt
//    DB_printf("ISR Entered \r\n");
//    IO_LINE = 1;
    if(1 == Searching4Beacon)
    {
//        DB_printf("ISR Entered \r\n");
//        DB_printf("Searching4Beacon = %d\r\n", Searching4Beacon);
//        IO_LINE = 1;
        do{
            CapturedTimeIR = (uint16_t)IC3BUF;
            CurrentRecordedTimeIR.ByBytes.LowByteRolloverCount = CapturedTimeIR;
            PeriodLength = CurrentRecordedTimeIR.TotalTime - LastRecordedTimeIR;
            LastRecordedTimeIR = CurrentRecordedTimeIR.TotalTime;
    //        printf("%d\n", PeriodLength);
        }while(IC3CONbits.ICBNE != 0);

        if((IFS0bits.T3IF == 1) && (CapturedTimeIR < 0x8000))
        {
            CurrentRecordedTimeIR.ByBytes.HighByteRolloverCount++;
            IFS0CLR = _IFS0_T3IF_MASK; // clear again in case
        }
        //printf("%d\n", abs(PeriodLength - 14000));
        if((abs(PeriodLength - 14000) <= 50))
        {
            aligned_counter++;
            
            //printf("%d\n",aligned_counter);

//            if (aligned_counter == 10)
//            {
               aligned_counter = 0;
               Searching4Beacon = 0; // disable ISR
               IEC0CLR = _IEC0_IC3IE_MASK;
               ES_Event_t NewEdge;
               NewEdge.EventType = ES_BEACON;
               PostLab8_SM(NewEdge);
               printf("Aligned\n");
//            }
        }
        else
        {
            if(aligned_counter != 0){
                aligned_counter = 0;
            }
        }
    }
    IO_LINE = 0;
 }

//void TestHarnessBeacon_RisingEdge(void)
//{
//    ES_Event_t RisingEdge;
//    RisingEdge.EventType = ES_BEACON_DETECTED;
//    RisingEdge.EventParam = ES_Timer_GetTime();
////    TimeOfLastRise = RisingEdge.EventParam;
////    DB_printf("Rise Detected at Time: %d \r \n", TimeOfLastRise);
//    PostBeaconFSM(RisingEdge);
//}
//
//void TestHarnessBeacon_FallingEdge(void)
//{
//    ES_Event_t FallingEdge;
//    FallingEdge.EventType = ES_NO_BEACON_DETECTED;
//    FallingEdge.EventParam = ES_Timer_GetTime();
////    TimeOfLastFall = FallingEdge.EventParam;
////    DB_printf("Fall Detected at Time: %d \r \n", TimeOfLastFall);
//    PostBeaconFSM(FallingEdge);
//}
