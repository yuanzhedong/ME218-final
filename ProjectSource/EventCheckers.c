/****************************************************************************
 Module
   EventCheckers.c

 Revision
   1.0.1

 Description
   This is the sample for writing event checkers along with the event
   checkers used in the basic framework test harness.

 Notes
   Note the use of static variables in sample event checker to detect
   ONLY transitions.

 History
 When           Who     What/Why
 -------------- ---     --------
 08/06/13 13:36 jec     initial version
****************************************************************************/

// this will pull in the symbolic definitions for events, which we will want
// to post in response to detecting events
#include "ES_Configure.h"
// This gets us the prototype for ES_PostAll
#include "ES_Framework.h"
// this will get us the structure definition for events, which we will need
// in order to post events in response to detecting events
#include "ES_Events.h"
// if you want to use distribution lists then you need those function
// definitions too.
#include "ES_PostList.h"
// This include will pull in all of the headers from the service modules
// providing the prototypes for all of the post functions
#include "ES_ServiceHeaders.h"
// this test harness for the framework references the serial routines that
// are defined in ES_Port.c
#include "ES_Port.h"
// include our own prototypes to insure consistency between header &
// actual functionsdefinition
#include "EventCheckers.h"

// This is the event checking function sample. It is not intended to be
// included in the module. It is only here as a sample to guide you in writing
// your own event checkers
#if 0
/****************************************************************************
 Function
   Check4Lock
 Parameters
   None
 Returns
   bool: true if a new event was detected
 Description
   Sample event checker grabbed from the simple lock state machine example
 Notes
   will not compile, sample only
 Author
   J. Edward Carryer, 08/06/13, 13:48
****************************************************************************/
bool Check4Lock(void)
{
  static uint8_t  LastPinState = 0;
  uint8_t         CurrentPinState;
  bool            ReturnVal = false;

  CurrentPinState = LOCK_PIN;
  // check for pin high AND different from last time
  // do the check for difference first so that you don't bother with a test
  // of a port/variable that is not going to matter, since it hasn't changed
  if ((CurrentPinState != LastPinState) &&
      (CurrentPinState == LOCK_PIN_HI)) // event detected, so post detected event
  {
    ES_Event ThisEvent;
    ThisEvent.EventType   = ES_LOCK;
    ThisEvent.EventParam  = 1;
    // this could be any of the service post functions, ES_PostListx or
    // ES_PostAll functions
    ES_PostAll(ThisEvent);
    ReturnVal = true;
  }
  LastPinState = CurrentPinState; // update the state for next time

  return ReturnVal;
}

#endif

/****************************************************************************
 Function
   Check4Keystroke
 Parameters
   None
 Returns
   bool: true if a new key was detected & posted
 Description
   checks to see if a new key from the keyboard is detected and, if so,
   retrieves the key and posts an ES_NewKey event to TestHarnessService0
 Notes
   The functions that actually check the serial hardware for characters
   and retrieve them are assumed to be in ES_Port.c
   Since we always retrieve the keystroke when we detect it, thus clearing the
   hardware flag that indicates that a new key is ready this event checker
   will only generate events on the arrival of new characters, even though we
   do not internally keep track of the last keystroke that we retrieved.
 Author
   J. Edward Carryer, 08/06/13, 13:48
****************************************************************************/
bool Check4Keystroke(void)
{
  if (IsNewKeyReady()) // new key waiting?
  {
    ES_Event_t ThisEvent;
    ThisEvent.EventType = ES_NEW_KEY;
    ThisEvent.EventParam = GetNewKey();
    ES_PostAll(ThisEvent);
    return true;
  }
  return false;
}

bool Check4CoinSignal(void)
{
  static uint8_t LastPinState = 1; // Last state of RB4, default HIGH
  uint8_t CurrentPinState;
  bool ReturnVal = false;

  CurrentPinState = PORTBbits.RB4; // Read the current state of RB4

  // Check for a falling edge
  if ((CurrentPinState != LastPinState) && (CurrentPinState == 0))
  {
    // Rising edge detected
    ES_Event_t ThisEvent;
    ThisEvent.EventType = ES_NEW_COIN_RISING;
    ThisEvent.EventParam = ES_Timer_GetTime();
    PostCoinLEDService(ThisEvent);
    ReturnVal = false; // have to set to false, otherwise code will stuch, why???
  }
  // Check for a rising edge
  else if ((CurrentPinState != LastPinState) && (CurrentPinState == 1))
  {
    // Falling edge detected
    ES_Event_t ThisEvent;
    ThisEvent.EventType = ES_NEW_COIN_FALLING;
    ThisEvent.EventParam = ES_Timer_GetTime();
    PostCoinLEDService(ThisEvent);
    ReturnVal = true;
  }

  LastPinState = CurrentPinState; // Update last state for the next check

  return ReturnVal;
}

// // Get a low signal when coin drop is detected
// bool Check4CoinSignal(void)
// {
//   uint8_t CurrentPinState;
//   bool ReturnVal = false;

//   CurrentPinState = PORTBbits.RB4; // Read the current state of RB4
//   //DB_printf("%d\n", CurrentPinState);

//   //Check for a falling edge
// //  if (true) {
// //      ReturnVal = true;
// //  }
//   //{
//     //DB_printf("%d\n", CurrentPinState);
//     // Falling edge detected
// //    ES_Event_t ThisEvent;
// //    ThisEvent.EventType = ES_NEW_COIN;
// //    ES_PostAll(ThisEvent);
//    // ReturnVal = true;
//   //}
//   if (CurrentPinState == 0) {
//       //puts("Detect New Coin...\n");
//       ES_Event_t ThisEvent;
//       ThisEvent.EventType = ES_NEW_COIN;
//       ThisEvent.EventParam = ES_Timer_GetTime()
//       PostCoinLEDService(ThisEvent);
//       ReturnVal = false; // can't return true, otherwise program will stuck, why????
//     }

//   return ReturnVal;
// }




#define JOYSTICK_ADC_CHANNEL BIT12HI // Define joystick input channel as AN12 (RB12)
#define JOYSTICK_THRESHOLD 10        // Define threshold for joystick signal change
#define MAX_CHANNELS 8

bool CheckForJoystickSignal(void)
{
    static uint16_t LastJoystickValue = 0; // Store the last joystick ADC value
    uint32_t adcResults[MAX_CHANNELS];     // Array to store ADC results
    uint16_t CurrentJoystickValue;
    bool SignalDetected = false;
    
    // Configure the ADC for joystick input (AN12 / RB12)
    if (!ADC_ConfigAutoScan(JOYSTICK_ADC_CHANNEL))
    {
        return false; // Return false if ADC configuration fails
    }

    // Read the current ADC values for all configured channels
    ADC_MultiRead(adcResults);

    // Get the joystick ADC value from the results (assuming AN12 is the first configured channel)
    CurrentJoystickValue = (uint16_t)adcResults[0];

    // Check if the joystick ADC value has significantly changed (threshold-based detection)
    if (abs(CurrentJoystickValue - LastJoystickValue) > JOYSTICK_THRESHOLD)
    {
        ES_Event_t JoystickEvent;
        JoystickEvent.EventType = ES_JoystickUpdate;
        JoystickEvent.EventParam = CurrentJoystickValue;
        PostJoyStickService(JoystickEvent);
        SignalDetected = true;
        LastJoystickValue = CurrentJoystickValue;
    }

    // Update the last joystick value
    

    return SignalDetected;
}
