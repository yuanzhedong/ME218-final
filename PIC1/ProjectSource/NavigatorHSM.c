/****************************************************************************
 Module
   NavigatorHSM.c

 Revision
   1.0.1

 Description
   This is the implementation of the Navigator state machine.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 10/10/23       GitHub Copilot  Initial implementation
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "NavigatorHSM.h"

/*----------------------------- Module Defines ----------------------------*/
#define ENTRY_STATE Init

/*---------------------------- Module Variables ---------------------------*/
static NavigatorState_t CurrentState;
static uint8_t MyPriority;

/*---------------------------- Module Functions ---------------------------*/
static ES_Event_t DuringInit(ES_Event_t Event);
static ES_Event_t DuringIdle(ES_Event_t Event);
static ES_Event_t DuringLineFollow(ES_Event_t Event);
static ES_Event_t DuringAlignBeacon(ES_Event_t Event);
static ES_Event_t DuringCheckIntersection(ES_Event_t Event);
static ES_Event_t DuringTurnLeft(ES_Event_t Event);
static ES_Event_t DuringTurnRight(ES_Event_t Event);
static ES_Event_t DuringLineDiscover(ES_Event_t Event);
static ES_Event_t DuringCheckCrate(ES_Event_t Event);
static void StartMotors(void);
static void StopMotors(void);
static void CheckSensors(void);
static void ReadIntersectionSensors(void);
static void StartRecoveryPattern(void);

bool InitNavigatorHSM(uint8_t Priority)
{
    MyPriority = Priority;
    ES_Event_t ThisEvent;

    CurrentState = ENTRY_STATE;
    ThisEvent.EventType = ES_INIT;

    return ES_PostToService(MyPriority, ThisEvent);
}

bool PostNavigatorHSM(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    StartNavigatorHSM

 Parameters
   ES_Event_t: the event to process

 Returns
   None

 Description
   Initializes the state machine for the Navigator.
 Notes
   Uses nested switch/case to implement the machine.
****************************************************************************/
void StartNavigatorHSM(ES_Event_t CurrentEvent)
{
    if (ES_ENTRY_HISTORY != CurrentEvent.EventType)
    {
        CurrentState = ENTRY_STATE;
    }
    RunNavigatorHSM(CurrentEvent);
}

/****************************************************************************
 Function
    QueryNavigatorHSM

 Parameters
   None

 Returns
   NavigatorState_t: the current state of the Navigator state machine

 Description
   Returns the current state of the Navigator state machine.
 Notes
   Uses nested switch/case to implement the machine.
****************************************************************************/
NavigatorState_t QueryNavigatorHSM(void)
{
    return CurrentState;
}

/***************************************************************************
 private functions
 ***************************************************************************/

static ES_Event_t DuringInit(ES_Event_t Event)
{
    if (Event.EventType == ES_ENTRY)
    {
        // Initialize sensors and motors
    }
    else if (Event.EventType == ES_EXIT)
    {
        // Cleanup if necessary
    }
    else
    {
        // During actions for Init state
    }
    return Event;
}

static ES_Event_t DuringIdle(ES_Event_t Event)
{
    if (Event.EventType == ES_ENTRY)
    {
        // Stop motors and wait for commands
    }
    else if (Event.EventType == ES_EXIT)
    {
        // Cleanup if necessary
    }
    else
    {
        // During actions for Idle state
    }
    return Event;
}

static ES_Event_t DuringLineFollow(ES_Event_t Event)
{
    if (Event.EventType == ES_ENTRY)

    {
        // post to TapeFollowerHSM
    }

    else if (Event.EventType == ES_EXIT)
    {
    }
    else
    {
        // Post to TapeFollowerHSM
    }
    return Event;
}

static ES_Event_t DuringAlignBeacon(ES_Event_t Event)
{
    if (Event.EventType == ES_ENTRY)
    {
        // Start aligning with beacon
    }
    else if (Event.EventType == ES_EXIT)
    {
        // Stop aligning with beacon
    }
    else
    {
        // During actions for AlignBeacon state
    }
    return Event;
}

static ES_Event_t DuringCheckIntersection(ES_Event_t Event)
{
    if (Event.EventType == ES_ENTRY)
    {
        StopMotors(); // Set duty cycle to zero
        ReadIntersectionSensors();
    }
    else if (Event.EventType == ES_EXIT)
    {
        // Cleanup if necessary
    }
    return Event;
}

static ES_Event_t DuringTurnLeft(ES_Event_t Event)
{
    if (Event.EventType == ES_ENTRY)
    {
        // Start turning left
    }
    else if (Event.EventType == ES_EXIT)
    {
        // Stop turning left
    }
    else
    {
        // During actions for TurnLeft state
    }
    return Event;
}

static ES_Event_t DuringTurnRight(ES_Event_t Event)
{
    if (Event.EventType == ES_ENTRY)
    {
        // Start turning right
    }
    else if (Event.EventType == ES_EXIT)
    {
        // Stop turning right
    }
    else
    {
        // During actions for TurnRight state
    }
    return Event;
}

static ES_Event_t DuringLineDiscover(ES_Event_t Event)
{
    ES_Event_t ReturnEvent = Event;
    if (Event.EventType == ES_ENTRY)
    {
        StartRecoveryPattern();
    }
    else
    {
        if (CheckLineSensors())
        {
            ReturnEvent.EventType = ES_RECOVERED;
        }
    }
    return ReturnEvent;
}

static ES_Event_t DuringCheckCrate(ES_Event_t Event)
{
    if (Event.EventType == ES_ENTRY)
    {
        // Check Crate and decide next action
    }
    else if (Event.EventType == ES_EXIT)
    {
        // Cleanup if necessary
    }
    else
    {
        // During actions for CheckCrate state
    }
    return Event;
}

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
    RunNavigatorHSM

 Parameters
   ES_Event_t: the event to process

 Returns
   ES_Event_t: an event to return

 Description
   Runs the state machine for the Navigator.
 Notes
   Uses nested switch/case to implement the machine.
****************************************************************************/
ES_Event_t RunNavigatorHSM(ES_Event_t CurrentEvent)
{
    bool MakeTransition = false;
    NavigatorState_t NextState = CurrentState;
    ES_Event_t EntryEventKind = {ES_ENTRY, 0};
    ES_Event_t ReturnEvent = {ES_NO_EVENT, 0};

    switch (CurrentState)
    {
    case Init:
        ReturnEvent = DuringInit(CurrentEvent);
        if (ReturnEvent.EventType == ES_INIT)
        {
            NextState = Idle;
            MakeTransition = true;
        }
        break;

    case Idle:
        ReturnEvent = DuringIdle(CurrentEvent);
        if (ReturnEvent.EventType != ES_NO_EVENT)
        {
            switch (CurrentEvent.EventType)
            {
            case ES_FORWARD:
                NextState = LineFollow;
                MakeTransition = true;
                break;
            case ES_ALIGN_BEACON:
                NextState = AlignBeacon;
                MakeTransition = true;
                break;
            }
        }
        break;

    case LineFollow:
        ReturnEvent = DuringLineFollow(CurrentEvent);
        if (ReturnEvent.EventType != ES_NO_EVENT)
        {
            switch (CurrentEvent.EventType)
            {
            case ES_CRATE_DETECTED:
                NextState = CheckCrate;
                MakeTransition = true;
                break;
            case ES_CROSS_DETECTED:
                NextState = CheckIntersection;
                MakeTransition = true;
                break;
            case ES_TJUNCTION_DETECTED:
                NextState = CheckIntersection;
                MakeTransition = true;
                break;
            case ES_STOP:
                NextState = Idle;
                MakeTransition = true;
                break;
            case ES_ERROR:
                NextState = LineDiscover;
                MakeTransition = true;
                break;
            }
        }
        break;

    case AlignBeacon:
        ReturnEvent = DuringAlignBeacon(CurrentEvent);
        if (ReturnEvent.EventType == ES_STOP || ReturnEvent.EventType == ES_ALIGNED)
        {
            NextState = Idle;
            MakeTransition = true;
        }
        break;

    case CheckIntersection:
        ReturnEvent = DuringCheckIntersection(CurrentEvent);
        if (ReturnEvent.EventType != ES_NO_EVENT)
        {
            switch (CurrentEvent.EventType)
            {
            case ES_TURN_LEFT:
                NextState = TurnLeft;
                MakeTransition = true;
                break;
            case ES_TURN_RIGHT:
                NextState = TurnRight;
                MakeTransition = true;
                break;
            case ES_FORWARD:
            case ES_BACKWARD:
                NextState = LineFollow;
                MakeTransition = true;
                break;
            case ES_STOP:
                NextState = Idle;
                MakeTransition = true;
                break;
            }
        }
        break;

    case TurnLeft:
        ReturnEvent = DuringTurnLeft(CurrentEvent);
        if (ReturnEvent.EventType == ES_TURN_COMPLETE || ReturnEvent.EventType == ES_STOP)
        {
            NextState = Idle;
            MakeTransition = true;
        }
        break;

    case TurnRight:
        ReturnEvent = DuringTurnRight(CurrentEvent);
        if (ReturnEvent.EventType == ES_TURN_COMPLETE || ReturnEvent.EventType == ES_STOP)
        {
            NextState = Idle;
            MakeTransition = true;
        }
        break;

    case LineDiscover:
        ReturnEvent = DuringLineDiscover(CurrentEvent);
        if (ReturnEvent.EventType == ES_RECOVERED)
        {
            NextState = LineFollow;
            MakeTransition = true;
        }
        else if (ReturnEvent.EventType == ES_STOP)
        {
            NextState = Idle;
            MakeTransition = true;
        }
        break;

    case CheckCrate:
        ReturnEvent = DuringCheckCrate(CurrentEvent);
        if (ReturnEvent.EventType == ES_FORWARD || ReturnEvent.EventType == ES_BACKWARD)
        {
            NextState = LineFollow;
            MakeTransition = true;
        }
        else if (ReturnEvent.EventType == ES_STOP)
        {
            NextState = Idle;
            MakeTransition = true;
        }
        break;
    }

    if (MakeTransition)
    {
        CurrentEvent.EventType = ES_EXIT;
        RunNavigatorHSM(CurrentEvent);

        CurrentState = NextState;

        RunNavigatorHSM(EntryEventKind);
    }

    return ReturnEvent;
}
