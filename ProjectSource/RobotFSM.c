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
#include "RobotFSM.h"
#include "MyMotorService.h"
#include "dbprintf.h"

/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
#define TapeSenorReading PORTAbits.RA2
#define TapeSensorTris TRISAbits.TRISA2 
static uint8_t MyPriority;
static RobotState_t state;
static AlignSubState_t AlignState;
//varibles for beacon alignment and tape detection
static uint16_t ScanInterval = 30; //time between each AD read
static uint32_t CurrADVal[1];
static uint32_t PrevADVal[1];
static uint16_t min_ADVal;
static uint32_t beaconAlignTime = 0;
static uint16_t alignError;
static uint16_t alignError_prev;
static bool rotateDir;//0 means it's rotating CCW
static bool errorIsIncr = 1; //if the error is increasing in the current dir
static uint8_t IncrCount = 0; //number of increasing counts
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
bool InitRobotFSM(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  /********************************************
   in here you write your initialization code
   *******************************************/
  puts("Start Initializing Robot FSM\n");
  state = NormalMove;
  /*******************************
  ***************Sensor stuff*/
  TapeSensorTris = 1; 
  //A2 doesn't have analog capability
  ADC_ConfigAutoScan(BIT12HI);//bit5 corresponds to AN12/PMD0/RB12
  TRISBbits.TRISB12 = 1; // configure the pin as input
  ANSELBbits.ANSB12 = 1; // Configure RB15 as analog IO
  DB_printf("AD service initialized \n");
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
bool PostRobotFSM(ES_Event_t ThisEvent)
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
ES_Event_t RunRobotFSM(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    /********************************************
    in here you write your service code
    *******************************************/
    ES_Event_t MotorEvent;
    switch(state)
    {
        case NormalMove:
            if(ES_GEN == ThisEvent.EventType){
//              DB_printf("Inside ES_GEN\n");
                switch(ThisEvent.EventParam)
                {
                    case STOP_CMD:
                        DB_printf("Command received: 0x%x\n",ThisEvent.EventParam);
                        DB_printf("Stopping motors\n");
                        MotorEvent.EventType = ES_STOP;
                        PostMotorService(MotorEvent);
                    break;
                    case CW_90_CMD:
                        DB_printf("Command received: 0x%x\n",ThisEvent.EventParam);
                        DB_printf("Turning CW 90 degrees\n");
                        MotorEvent.EventType = ES_RIGHT90;
                        PostMotorService(MotorEvent);
                        break;
                    case CW_45_CMD:
                        DB_printf("Command received: 0x%x\n",ThisEvent.EventParam);
                        DB_printf("Turning CW 45 degrees\n");
                        MotorEvent.EventType = ES_RIGHT45;
                        PostMotorService(MotorEvent);
                        break;
                    case CCW_90_CMD:
                        DB_printf("Command received: 0x%x\n",ThisEvent.EventParam);
                        DB_printf("Turning CCW 90 degrees\n");
                        MotorEvent.EventType = ES_LEFT90;
                        PostMotorService(MotorEvent);
                        break;
                    case CCW_45_CMD:
                        DB_printf("Command received: 0x%x\n",ThisEvent.EventParam);
                        DB_printf("Turning CCW 45 degrees\n");
                        MotorEvent.EventType = ES_LEFT45;
                        PostMotorService(MotorEvent);
                        break;
                    case FWD_FULL_CMD:
                        DB_printf("Command received: 0x%x\n",ThisEvent.EventParam);
                        DB_printf("Driving forwards at full speed\n");
                        MotorEvent.EventType = ES_FWDFULL;
                        PostMotorService(MotorEvent);
                        break;
                    case FWD_HALF_CMD:
                        DB_printf("Command received: 0x%ScanIntervalx\n",ThisEvent.EventParam);
                        DB_printf("Driving forwards at half speed\n");
                        MotorEvent.EventType = ES_FWDHALF;
                        PostMotorService(MotorEvent);
                        break;
                    case BWD_FULL_CMD:
                        DB_printf("Command received: 0x%x\n",ThisEvent.EventParam);
                        DB_printf("Driving backwards at full speed\n");
                        MotorEvent.EventType = ES_BWDFULL;
                        PostMotorService(MotorEvent);
                        break;
                    case BWD_HALF_CMD:
                        DB_printf("Command received: 0x%x\n",ThisEvent.EventParam);
                        DB_printf("Driving backwards at half speed\n");
                        MotorEvent.EventType = ES_BWDHALF;
                        PostMotorService(MotorEvent);
                        break;
                    case ALIGN_CMD:
                        state = BeaconAlign;
                        AlignState = FindingPeak;
                        MotorEvent.EventType = ES_CW360;
                        PostMotorService(MotorEvent);
                        DB_printf("Command: 0x%x\n",ThisEvent.EventParam);
                        DB_printf("Aligning with beacon\n");
                        ES_Timer_InitTimer(ADTimer,ScanInterval);
                        break;
                    
                    default:
                        break;
                }
            }
        break;
        case BeaconAlign:
            if (ThisEvent.EventParam == ADTimer)//AD value gets read no matter what
            {
                    ES_Timer_InitTimer(ADTimer,ScanInterval);
                    ADC_MultiRead(CurrADVal);
                }
            switch (AlignState)
            {
            case FindingPeak:
                if (CurrADVal[0]<PrevADVal[0])
                {
                    min_ADVal = CurrADVal[0];
                    }
                PrevADVal[0] = CurrADVal[0];    
                if (ThisEvent==Rotate360Complete)
                {
                    AlignState = AlignPeak;
                    alignError_prev = 1023;
                    ES_Event_t Event2Post;
                    Event2Post.EventType = ES_CCW_continuous;
                    rotateDir = 0; //0 means it's rotating CCW
                    PostMotorService(Event2Post);
                    
                }
                
            break;
            case AlignPeak:
            if (CurrADVal[0]<min_ADVal + 10 || ThisEvent.EventType == ES_GEN)
            {
                ES_Event_t Event2Post;
                Event2Post.EventType = ES_STOP;
                PostMotorService(Event2Post);
                state = NormalMove;
                AlignState = FindingPeak;
                if (ThisEvent.EventType == ES_GEN)//meaning aligning took too much time and the next command is already here
                {
                    PostRobotFSM(ThisEvent);
                }
                
            }
            
                // alignError = CurrADVal[0] - min_ADVal;
                // if (alignError > alignError_prev)//error increased
                // {
                //     errorIsIncr = true;
                //     IncrCount+=1;
                // }else
                // {
                //     errorIsIncr = false;
                //     IncrCount = 0;
                // }
                // if (IncrCount>=4)
                // {
                //     IncrCount = 0;//reset the count
                //     rotateDir = !rotateDir;
                //     ES_Event_t Event2Post;
                //     if (rotateDir)
                //     {
                //         Event2Post.EventType = ES_CW_continuous;
                //     }else
                //     {
                //         Event2Post.EventType = ES_CCW_continuous;
                //     }
                //     PostMotorService(Event2Post);    
                // }
                
                
            break;
            case FindTape:
            break;
            default:
                break;
            }
            

                
                
        break;
        case FindTape:
        break;
        default:
        break;
    }
//  DB_printf("ThisEvent.EventType = %u\n", ThisEvent.EventType);
    return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

