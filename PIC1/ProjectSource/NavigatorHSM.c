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
#include "SPIFollowerService.h"

/*----------------------------- Module Defines ----------------------------*/
#define ENTRY_STATE Init


/*---------------------------- Module Variables ---------------------------*/
static NavigatorState_t CurrentState, NextState, PrevState;
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

static bool search_tap_ccw = false;
static bool search_tap_cw = false;

void NavCMDComplete(uint8_t nav_cmd) {
    ES_Event_t ThisEvent;
    ThisEvent.EventType = ES_NEW_NAV_STATUS;
    ThisEvent.EventParam = nav_cmd + 1;
    PostSPIFollowerService(ThisEvent);
}
void StopTapeFollow(void) {
    ES_Event_t ThisEvent;
    ThisEvent.EventType = ES_TAPE_STOP;
    PostTapeFSM(ThisEvent);
}
void ForwardTapeFollow(uint8_t speed) {
    ES_Event_t ThisEvent;
    ThisEvent.EventType = ES_TAPE_FOLLOW_FWD;
    ThisEvent.EventParam = speed;
    PostTapeFSM(ThisEvent);
}

void ReverseTapeFollow(uint8_t speed) {
    ES_Event_t ThisEvent;
    ThisEvent.EventType = ES_TAPE_FOLLOW_REV;
    ThisEvent.EventParam = speed;
    PostTapeFSM(ThisEvent);
}

void UpdateNavStatus(uint8_t status) {
    ES_Event_t ThisEvent;
    ThisEvent.EventType = ES_NEW_NAV_STATUS;
    ThisEvent.EventParam = status;
    PostSPIFollowerService(ThisEvent);
}

bool InitNavigatorHSM(uint8_t Priority) {
    ES_Event_t ThisEvent;
    MyPriority = Priority;

    CurrentState = Init;
    ThisEvent.EventType = ES_INIT;

    ES_Timer_InitTimer(NAV_STATE_DEBUG_TIMER, 2000);

    if (ES_PostToService(MyPriority, ThisEvent) == true) {
        DB_printf("Navigator HSM initialized\r\n");
        return true;
    } else {
        return false;
    }
}

bool PostNavigatorHSM(ES_Event_t ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

const char* StateToString(NavigatorState_t state) {
    switch (state) {
        case Init:
            return "Init";
        case Idle:
            return "Idle";
        case LineFollowForward:
            return "LineFollowForward";
        case LineFollowBackward:
            return "LineFollowBackward";
        case AlignTape:
            return "AlignTape";
        case LineDiscoverFail:
            return "LineDiscoverFail";
        case CheckCrate:
            return "CheckCrate";
        case CheckIntersection:
            return "CheckIntersection";
        case TurnLeft:
            return "TurnLeft";
        case TurnRight:
            return "TurnRight";
        case LineDiscover:
            return "LineDiscover";
        default:
            return "Unknown";
    }
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
void StartNavigatorHSM(ES_Event_t CurrentEvent) {
    if (ES_ENTRY_HISTORY != CurrentEvent.EventType) {
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
NavigatorState_t QueryNavigatorHSM(void) {
    return CurrentState;
}

/***************************************************************************
 private functions
 ***************************************************************************/


static ES_Event_t DuringInit(ES_Event_t Event) {
    if (Event.EventType == ES_ENTRY) {
        // Initialize sensors and motors
        DB_printf("Navigator initialized\r\n");
    } else if (Event.EventType == ES_EXIT) {
        // Cleanup if necessary
        DB_printf("Navigator init exited\r\n");
    } else {
        // During actions for Init state
        DB_printf("Navigator running init\r\n");
    }
    return Event;
}

static ES_Event_t DuringIdle(ES_Event_t Event) {
    if (Event.EventType == ES_ENTRY) {
        // Stop motors and wait for commands
    } else if (Event.EventType == ES_EXIT) {
        // Cleanup if necessary
    } else {
        // During actions for Idle state
    }
    return Event;
}

static ES_Event_t DuringLineFollow(ES_Event_t Event) {
    if (Event.EventType == ES_ENTRY) {
        // Start line following
    } else if (Event.EventType == ES_EXIT) {
        // Stop line following
    } else {
        // During actions for LineFollow state
    }
    return Event;
}

static ES_Event_t DuringAlignBeacon(ES_Event_t Event) {
    if (Event.EventType == ES_ENTRY) {
        // Start aligning with beacon
    } else if (Event.EventType == ES_EXIT) {
        // Stop aligning with beacon
    } else {
        // During actions for AlignBeacon state
    }
    return Event;
}

static ES_Event_t DuringCheckIntersection(ES_Event_t Event) {
    if (Event.EventType == ES_ENTRY) {
        // Check intersection and decide next action
    } else if (Event.EventType == ES_EXIT) {
        // Cleanup if necessary
    } else {
        // During actions for CheckIntersection state
    }
    return Event;
}

static ES_Event_t DuringTurnLeft(ES_Event_t Event) {
    if (Event.EventType == ES_ENTRY) {
        // Start turning left
    } else if (Event.EventType == ES_EXIT) {
        // Stop turning left
    } else {
        // During actions for TurnLeft state
    }
    return Event;
}

static ES_Event_t DuringTurnRight(ES_Event_t Event) {
    if (Event.EventType == ES_ENTRY) {
        // Start turning right
    } else if (Event.EventType == ES_EXIT) {
        // Stop turning right
    } else {
        // During actions for TurnRight state
    }
    return Event;
}

static ES_Event_t DuringLineDiscover(ES_Event_t Event) {
    if (Event.EventType == ES_ENTRY) {
        // Start line discovery
    } else if (Event.EventType == ES_EXIT) {
        // Stop line discovery
    } else {
        // During actions for LineDiscover state
    }
    return Event;
}

static ES_Event_t DuringCheckCrate(ES_Event_t Event) {
    if (Event.EventType == ES_ENTRY) {
        // Check Crate and decide next action
    } else if (Event.EventType == ES_EXIT) {
        // Cleanup if necessary
    } else {
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
ES_Event_t RunNavigatorHSM(ES_Event_t CurrentEvent) {
    //ES_Event_t EntryEventKind = {ES_ENTRY, 0}; // default to normal entry to new state
    bool MakeTransition = false; /* are we making a state transition? */
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    if (CurrentEvent.EventType == ES_TIMEOUT && CurrentEvent.EventParam == NAV_STATE_DEBUG_TIMER) {
        DB_printf("[NAV HSM] Current state: %s\r\n", StateToString(CurrentState));
        ES_Timer_InitTimer(NAV_STATE_DEBUG_TIMER, 2000);
    }
    switch (CurrentState) {
        case Init:
            //ReturnEvent = DuringInit(CurrentEvent);
            if (CurrentEvent.EventType == ES_INIT) {
                NextState = Idle;
                MakeTransition = true;
            }

            if (CurrentEvent.EventType == ES_EXIT) {
                DB_printf("[NAV HSM] Exit\r\n");
                UpdateNavStatus(NAV_STATUS_OK);
            }
            break;

        case Idle:
            if (CurrentEvent.EventType == ES_ENTRY) {
                DB_printf("[NAV HSM] Idle\r\n");
                UpdateNavStatus(NAV_STATUS_IDLE);
            } else {
                if (CurrentEvent.EventType == ES_NEW_NAV_CMD) {
                    DB_printf("[NAV HSM] Received new command: %d\r\n", CurrentEvent.EventParam);
                    uint8_t command = CurrentEvent.EventParam;
                    switch (command) {
                        case NAV_CMD_MOVE_FORWARD:
                            NextState = LineFollowForward;
                            MakeTransition = true;
                            break;
                        case NAV_CMD_MOVE_BACKWARD:
                            NextState = LineFollowBackward;
                            MakeTransition = true;
                            break;
                        case NAV_CMD_ALIGN:
                            NextState = AlignTape;
                            MakeTransition = true;
                            break;
    
                        // for debug purposes, disable this when deploy
    
                        case NAV_CMD_STOP:
                            NextState = Idle;
                            MakeTransition = true;
                            break;
                        case NAV_CMD_TURN_LEFT:
                            NextState = TurnLeft;
                            MakeTransition = true;
                            break;
                        case NAV_CMD_TURN_RIGHT:
                            NextState = TurnRight;
                            MakeTransition = true;
                            break;
                    }
                }
                // for debug purposes, disable this when deploy
                if (CurrentEvent.EventType == ES_CROSS_DETECTED) {
                    NextState = CheckIntersection;
                    MakeTransition = true;
                }
            }
            break;

        case AlignTape:
            if (CurrentEvent.EventType == ES_ENTRY) {
                DB_printf("[NAV HSM] Aligning with tape\r\n");
                UpdateNavStatus(NAV_STATUS_ALIGN_TAPE);
                ES_Event_t ThisEvent;
                ThisEvent.EventType = ES_MOTOR_CCW_180;
                search_tap_ccw = true;
                PostMotorService(ThisEvent);
            } else {
                switch (CurrentEvent.EventType) {
                    case ES_TAPE_ALIGNED:
                        NextState = LineFollowForward;
                        MakeTransition = true;
                        break;
                    case ES_TURN_COMPLETE:
                        DB_printf("[NAV HSM] Motor complete \r\n");
                        if (search_tap_ccw && search_tap_cw) {
                            NextState = LineDiscoverFail;
                            MakeTransition = true;
                        } else {
                            ES_Event_t ThisEvent;
                            ThisEvent.EventType = ES_MOTOR_CW_180;
                            search_tap_cw = true;
                            PostMotorService(ThisEvent);
                        }
                        break;
                    default:
                        break;
                }
            }
            break;
        case LineDiscoverFail:
            DB_printf("[NAV HSM] LineDiscoverFail \r\n");
            // TODO: For debug purposes, disable this when deploy
            if (CurrentEvent.EventType == ES_NEW_NAV_CMD) {
                StopTapeFollow();
                switch (CurrentEvent.EventParam) {
                    case NAV_CMD_MOVE_FORWARD:
                        //ForwardTapeFollow(70);
                        NextState = LineFollowForward;
                        MakeTransition = true;
                        break;
                    case NAV_CMD_MOVE_BACKWARD:
                        NextState = LineFollowBackward;
                        MakeTransition = true;
                        //ReverseTapeFollow(70);
                        break;
                    case NAV_CMD_STOP:
                        StopTapeFollow();
                        break;
                    default:
                        break;
                }
            }
            
            break;
        case LineFollowForward:
            if (CurrentEvent.EventType == ES_ENTRY) {
                DB_printf("[NAV HSM] Line following Forward\r\n");
                UpdateNavStatus(NAV_STATUS_LINE_FOLLOW);
                ForwardTapeFollow(70);
            } else {
                switch (CurrentEvent.EventType) {
                    case ES_CRATE_DETECTED:
                        NextState = CheckCrate;
                        MakeTransition = true;
                        StopTapeFollow();
                        NavCMDComplete(NAV_CMD_MOVE_FORWARD);
                        break;
                    case ES_CROSS_DETECTED:
                        DB_printf("[NAV HSM LINE_FOLLOW] Cross detected \r\n");
                        NextState = CheckIntersection;
                        MakeTransition = true;
                        StopTapeFollow();
                        NavCMDComplete(NAV_CMD_MOVE_FORWARD);
                        break;
                    case ES_TJUNCTION_DETECTED:
                        NextState = CheckIntersection;
                        MakeTransition = true;
                        StopTapeFollow();
                        NavCMDComplete(NAV_CMD_MOVE_FORWARD);
                        break;
                    case ES_STOP:
                        NextState = Idle;
                        MakeTransition = true;
                        StopTapeFollow();
                        NavCMDComplete(NAV_CMD_MOVE_FORWARD);
                        break;
                    case ES_ERROR:
                        NextState = LineDiscover;
                        MakeTransition = true;
                        StopTapeFollow();
                        NavCMDComplete(NAV_CMD_MOVE_FORWARD);
                        break;
                    default:
                        break;
                }
            }
            break;
        case LineFollowBackward:
            if (CurrentEvent.EventType == ES_ENTRY) {
                DB_printf("[NAV HSM] Line following Backward\r\n");
                UpdateNavStatus(NAV_STATUS_LINE_FOLLOW);
                ReverseTapeFollow(70);
            } else {
                switch (CurrentEvent.EventType) {
                    case ES_CRATE_DETECTED:
                        NextState = CheckCrate;
                        MakeTransition = true;
                        StopTapeFollow();
                        NavCMDComplete(NAV_CMD_MOVE_BACKWARD);
                        break;
                    case ES_CROSS_DETECTED:
                        DB_printf("[NAV HSM LINE_FOLLOW] Cross detected \r\n");
                        NextState = CheckIntersection;
                        MakeTransition = true;
                        StopTapeFollow();
                        NavCMDComplete(NAV_CMD_MOVE_BACKWARD);
                        break;
                    case ES_TJUNCTION_DETECTED:
                        NextState = CheckIntersection;
                        MakeTransition = true;
                        StopTapeFollow();
                        NavCMDComplete(NAV_CMD_MOVE_BACKWARD);
                        break;
                    case ES_STOP:
                        NextState = Idle;
                        MakeTransition = true;
                        StopTapeFollow();
                        NavCMDComplete(NAV_CMD_MOVE_BACKWARD);
                        break;
                    case ES_ERROR:
                        NextState = LineDiscover;
                        MakeTransition = true;
                        StopTapeFollow();
                        NavCMDComplete(NAV_CMD_MOVE_BACKWARD);
                        break;
                    default:
                        break;
                }
            }
            break;

        case CheckCrate:
            DB_printf("[NAV HSM] CheckCrate \r\n");
            break;
        case CheckIntersection:
            switch (CurrentEvent.EventType) {
                case ES_NEW_NAV_CMD:
                    switch (CurrentEvent.EventParam) {
                        case NAV_CMD_TURN_LEFT:
                            NextState = TurnLeft;
                            MakeTransition = true;
                            break;
                        case NAV_CMD_TURN_RIGHT:
                            NextState = TurnLeft;   
                            MakeTransition = true;
                            break;
                        case NAV_CMD_MOVE_FORWARD:
                            NextState = LineFollowForward;
                            MakeTransition = true;
                            break;
                        case NAV_CMD_MOVE_BACKWARD:
                            NextState = LineFollowBackward;
                            MakeTransition = true;
                            break;
                        case NAV_CMD_STOP:
                            NextState = Idle;
                            MakeTransition = true;
                            break;
                        default:
                            break;
                    }
                    break;
                case ES_STOP:
                    NextState = Idle;
                    MakeTransition = true;
                    break;
                default:
                    break;
            }
            break;
        case TurnLeft:
            DB_printf("[NAV HSM] TurnLeft \r\n");
            if (CurrentEvent.EventType == ES_ENTRY) {
                ES_Event_t ThisEvent;
                ThisEvent.EventType = ES_MOTOR_CCW_90;
                PostMotorService(ThisEvent);
            } else {
                switch (CurrentEvent.EventType) {
                    case ES_TURN_COMPLETE:
                        NextState = Idle;
                        NavCMDComplete(NAV_CMD_TURN_LEFT);
                        MakeTransition = true;
                        break;
                    case ES_STOP:
                        NextState = Idle;
                        MakeTransition = true;
                        NavCMDComplete(NAV_CMD_TURN_LEFT);
                        break;
                    default:
                        break;
                }
            }
            break;
        case TurnRight:
            DB_printf("[NAV HSM] TurnRight \r\n");
            if (CurrentEvent.EventType == ES_ENTRY) {
                ES_Event_t ThisEvent;
                ThisEvent.EventType = ES_MOTOR_CW_90;
                PostMotorService(ThisEvent);
            } else {
                switch (CurrentEvent.EventType) {
                    case ES_TURN_COMPLETE:
                        NextState = Idle;
                        NavCMDComplete(NAV_CMD_TURN_RIGHT);
                        MakeTransition = true;
                        break;
                    case ES_STOP:
                        NextState = Idle;
                        MakeTransition = true;
                        NavCMDComplete(NAV_CMD_TURN_RIGHT);
                        break;
                    default:
                        break;
                }
            }
            break;
        case LineDiscover:
            DB_printf("[NAV HSM] LineDiscover \r\n");
            break;
        default:
            break;
        }

    //     case Idle:
    //         ReturnEvent = DuringIdle(CurrentEvent);
    //         if (ReturnEvent.EventType != ES_NO_EVENT) {
    //             switch (CurrentEvent.EventType) {
    //                 case ES_FORWARD:
    //                     NextState = LineFollow;
    //                     MakeTransition = true;
    //                     break;
    //                 case ES_ALIGN_BEACON:
    //                     NextState = AlignBeacon;
    //                     MakeTransition = true;
    //                     break;
    //             }
    //         }
    //         break;

    //     case LineFollow:
    //         ReturnEvent = DuringLineFollow(CurrentEvent);
    //         if (ReturnEvent.EventType != ES_NO_EVENT) {
    //             switch (CurrentEvent.EventType) {
    //                 case ES_CRATE_DETECTED:
    //                     NextState = CheckCrate;
    //                     MakeTransition = true;
    //                     break;
    //                 case ES_CROSS_DETECTED:
    //                     NextState = CheckIntersection;
    //                     MakeTransition = true;
    //                     break;
    //                 case ES_TJUNCTION_DETECTED:
    //                     NextState = CheckIntersection;
    //                     MakeTransition = true;
    //                     break;
    //                 case ES_STOP:
    //                     NextState = Idle;
    //                     MakeTransition = true;
    //                     break;
    //                 case ES_ERROR:
    //                     NextState = LineDiscover;
    //                     MakeTransition = true;
    //                     break;
    //             }
    //         }
    //         break;

    //     case AlignBeacon:
    //         ReturnEvent = DuringAlignBeacon(CurrentEvent);
    //         if (ReturnEvent.EventType == ES_STOP || ReturnEvent.EventType == ES_ALIGNED) {
    //             NextState = Idle;
    //             MakeTransition = true;
    //         }
    //         break;

    //     case CheckIntersection:
    //         ReturnEvent = DuringCheckIntersection(CurrentEvent);
    //         if (ReturnEvent.EventType != ES_NO_EVENT) {
    //             switch (CurrentEvent.EventType) {
    //                 case ES_TURN_LEFT:
    //                     NextState = TurnLeft;
    //                     MakeTransition = true;
    //                     break;
    //                 case ES_TURN_RIGHT:
    //                     NextState = TurnRight;
    //                     MakeTransition = true;
    //                     break;
    //                 case ES_FORWARD:
    //                 case ES_BACKWARD:
    //                     NextState = LineFollow;
    //                     MakeTransition = true;
    //                     break;
    //                 case ES_STOP:
    //                     NextState = Idle;
    //                     MakeTransition = true;
    //                     break;
    //             }
    //         }
    //         break;

    //     case TurnLeft:
    //         ReturnEvent = DuringTurnLeft(CurrentEvent);
    //         if (ReturnEvent.EventType == ES_TURN_COMPLETE || ReturnEvent.EventType == ES_STOP) {
    //             NextState = Idle;
    //             MakeTransition = true;
    //         }
    //         break;

    //     case TurnRight:
    //         ReturnEvent = DuringTurnRight(CurrentEvent);
    //         if (ReturnEvent.EventType == ES_TURN_COMPLETE || ReturnEvent.EventType == ES_STOP) {
    //             NextState = Idle;
    //             MakeTransition = true;
    //         }
    //         break;

    //     case LineDiscover:
    //         ReturnEvent = DuringLineDiscover(CurrentEvent);
    //         if (ReturnEvent.EventType == ES_RECOVERED) {
    //             NextState = LineFollow;
    //             MakeTransition = true;
    //         } else if (ReturnEvent.EventType == ES_STOP) {
    //             NextState = Idle;
    //             MakeTransition = true;
    //         }
    //         break;

    //     case CheckCrate:
    //         ReturnEvent = DuringCheckCrate(CurrentEvent);
    //         if (ReturnEvent.EventType == ES_FORWARD || ReturnEvent.EventType == ES_BACKWARD) {
    //             NextState = LineFollow;
    //             MakeTransition = true;
    //         } else if (ReturnEvent.EventType == ES_STOP) {
    //             NextState = Idle;
    //             MakeTransition = true;
    //         }
    //         break;
    // }

    if (MakeTransition) {
        CurrentEvent.EventType = ES_EXIT;
        RunNavigatorHSM(CurrentEvent);
        PrevState = CurrentState;
        CurrentState = NextState;
        CurrentEvent.EventType = ES_ENTRY;
        RunNavigatorHSM(CurrentEvent);
    }

    return ReturnEvent;
}

