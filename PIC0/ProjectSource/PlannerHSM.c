/*----------------------------- Include Files -----------------------------*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "PlannerHSM.h"
#include "dbprintf.h"
#include "BeaconIndicatorService.h"
#include "SPIMasterService.h"
#include "PlannerPolicyService.h"
#include "ServoService.h"

/*----------------------------- Module Variables ---------------------------*/
static PlannerState_t CurrentState;
static uint8_t MyPriority;
static uint8_t CURRENT_COLUMN = 1;
static uint8_t drop_crate_count = 1;
static PlannerState_t ProcessColumnSubState = GO_TO_STACK; // Start substate
static tape_aligned = false;
static side_detected = false;
/*---------------------------- Helper Functions ---------------------------*/
const char* GetStateName(PlannerState_t state) {
    switch (state) {
        case INIT_PLANNER: return "INIT_PLANNER";
        case TAPE_AND_SIDE_DETECTION: return "TAPE_AND_SIDE_DETECTION";
        case NAVIGATE_TO_COLUMN_1: return "NAVIGATE_TO_COLUMN_1";
        case PROCESS_COLUMN: return "PROCESS_COLUMN";
        case GO_TO_STACK: return "GO_TO_STACK";
        case DROP_CRATE: return "DROP_CRATE";
        case CHECK_ROBO_STATUS: return "CHECK_ROBO_STATUS";
        case GO_TO_CRATE: return "GO_TO_CRATE";
        case PICKUP_CRATE: return "PICKUP_CRATE";
        case NAVIGATE_TO_COLUMN_2: return "NAVIGATE_TO_COLUMN_2";
        case GAME_OVER: return "GAME_OVER";
        default: return "UNKNOWN_STATE";
    }
}

/*---------------------------- During Function ---------------------------*/
static ES_Event_t DuringPROCESS_COLUMN(ES_Event_t Event) {
    ES_Event_t ReturnEvent = Event;

    if (Event.EventType == ES_ENTRY || Event.EventType == ES_ENTRY_HISTORY) {
        DB_printf("Entering PROCESS_COLUMN, CURRENT_COLUMN: %d, Drop Crate Count: %d\n",
                  CURRENT_COLUMN, drop_crate_count);
        ProcessColumnSubState = GO_TO_STACK; // Always start here
    } else if (Event.EventType == ES_EXIT) {
        DB_printf("Exiting PROCESS_COLUMN\n");
    } else if (Event.EventType != ES_NEW_KEY){
        // Handle substate transitions
        
        DB_printf("Current State: %s, Current Column: %d, Drop Crate Count: %d\r\n", GetStateName(ProcessColumnSubState), CURRENT_COLUMN, drop_crate_count);
        switch (ProcessColumnSubState) {
            case GO_TO_STACK:
                if (Event.EventType == ES_AT_STACK) {
                    DB_printf("Crate reached the stack\n");
                    ProcessColumnSubState = DROP_CRATE;
                }
                break;
            
            case DROP_CRATE:
                if (Event.EventType == ES_DROPPED) {
                    DB_printf("Crate dropped\n");
                    drop_crate_count++;
//                    ProcessColumnSubState = UPDATE_PROGRESS2;
                    if (CURRENT_COLUMN == 2 && drop_crate_count == 3) {
                        DB_printf("Finished processing column\n");
                        ES_Event_t ColumnDoneEvent;
                        ColumnDoneEvent.EventType = ES_COLUMN_DONE;
                        PostPlannerHSM(ColumnDoneEvent);
                        return ColumnDoneEvent;  
                    }else{
                       DB_printf("More crates available, moving to next crate\n");
                        ProcessColumnSubState = GO_TO_CRATE; 
                    }                    
                }
                break;     
//            case UPDATE_PROGRESS2:
//
//                break;
            
            case GO_TO_CRATE:
                if (Event.EventType == ES_AT_CRATE) {
                    DB_printf("Arrived at a new crate\n");
                    ProcessColumnSubState = PICKUP_CRATE;
                }
                break;
            
            case PICKUP_CRATE:
                if (Event.EventType == ES_HAS_CRATE) {
                    DB_printf("Picked up a crate\n");
//                    ProcessColumnSubState = UPDATE_PROGRESS1;
                    if (CURRENT_COLUMN == 1 && drop_crate_count == 3) {
                        DB_printf("Column processing completed\n");
                        ES_Event_t ColumnDoneEvent;
                        ColumnDoneEvent.EventType = ES_COLUMN_DONE;
                        PostPlannerHSM(ColumnDoneEvent);
                        return ColumnDoneEvent;  
                    }else{
                        DB_printf("Continuing column process\n");
                        ProcessColumnSubState = GO_TO_STACK;
                    }
                }
                break;
//            
//            case UPDATE_PROGRESS1:
//                if (CURRENT_COLUMN == 1 && drop_crate_count == 3) {
//                    DB_printf("Column processing completed\n");
//                }else{
//                    DB_printf("Continuing column process\n");
//                    ProcessColumnSubState = GO_TO_STACK;
//                }
//                break;

            default:
                break;
        }
    }
    return ReturnEvent;
}

/*------------------------------ Module Code ------------------------------*/
bool InitPlannerHSM(uint8_t Priority) {
    ES_Event_t ThisEvent;
    MyPriority = Priority;
    CurrentState = INIT_PLANNER;
    ThisEvent.EventType = ES_ENTRY;
    StartPlannerHSM(ThisEvent);
    DB_printf("Entering the FSM\r\n");
    return true;
}

bool PostPlannerHSM(ES_Event_t ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event_t RunPlannerHSM(ES_Event_t CurrentEvent) {
    bool MakeTransition = false;
    PlannerState_t NextState = CurrentState;
    ES_Event_t EntryEventKind = { ES_ENTRY, 0 };
    ES_Event_t ReturnEvent = { ES_NO_EVENT, 0 };
    ES_Event_t ThisEvent; // Move declaration here

    if (CurrentEvent.EventType != ES_EXIT && CurrentEvent.EventType != ES_ENTRY && CurrentEvent.EventType != ES_NEW_KEY) {
        DB_printf("Current State: %s, Current Column: %d, Drop Crate Count: %d\r\n",
                  GetStateName(CurrentState), CURRENT_COLUMN, drop_crate_count);
    }

    switch (CurrentState) {
        case INIT_PLANNER:
            if (CurrentEvent.EventType == ES_ENTRY) {
                CURRENT_COLUMN = 1;
                drop_crate_count = 1;
                DB_printf("[Planner] Initializing....\r\n");
                ThisEvent.EventType = ES_INIT_COMPLETE;
                PostPlannerHSM(ThisEvent);
            } else if (CurrentEvent.EventType == ES_INIT_COMPLETE) {
                DB_printf("[Planner] Waiting for Navigator....\r\n");
            }
            // for debug purposes, disable this when deploy
            if (CurrentEvent.EventType == ES_START_PLANNER) {
                DB_printf("Exiting INIT_PLANNER\r\n");
                MakeTransition = true;
                NextState = SEARCH_PICKUP_CRATE;
            }

            // wait for nav status idle use this for check off
            if (CurrentEvent.EventType == ES_NAVIGATOR_HEALTH_CHECK && CurrentEvent.EventParam == NAV_STATUS_IDLE) {
                DB_printf("Navigator is idle, moving to next, search for first crate\r\n");
                NextState = SEARCH_PICKUP_CRATE;
                MakeTransition = true;
            }
            break;

        // pickup the first crate
        case SEARCH_PICKUP_CRATE:
            switch (CurrentEvent.EventType) {
                case ES_ENTRY:
                    //TODO: add code to pick up the first crate
                    DB_printf("Entering SEARCH_PICKUP_CRATE\r\n");
                    //NextState = TAPE_AND_SIDE_DETECTION;
                    //MakeTransition = true;
                    break;
                case ES_CRATE_PICKED:
                    DB_printf("Crate picked up, moving to next state\r\n");
                    NextState = TAPE_AND_SIDE_DETECTION;
                    MakeTransition = true;
                    break;
            }
            break;
        case TAPE_AND_SIDE_DETECTION:
            switch (CurrentEvent.EventType) {
                case ES_ENTRY:
                    ThisEvent.EventType = ES_REQUEST_SIDE_DETECTION;
                    PostBeaconIndicatorService(ThisEvent);
                    // Request chassis to turn CCW
                    ThisEvent.EventType = ES_NEW_NAV_CMD;
                    ThisEvent.EventParam = NAV_CMD_ALIGN;
                    PostSPIMasterService(ThisEvent);
                    break;

                case ES_SIDE_DETECTED:
                    if (CurrentEvent.EventParam == BEACON_UNKNOWN) {
                        puts("Fail to identify side! from put\r\n");
                        //NextState = GAME_OVER;
                        //MakeTransition = true;
                    } else {
                        if (CurrentEvent.EventParam == BEACON_L) {
                            puts("We are at green side!\r\n");
                        } else {
                            puts("We are at blue side!\r\n");
                        }
                    }
                    // TODO: for debug purposes, disable this when deploy
                    side_detected = true;
                    break;
                
                //TODO: Assume beacon detection always success before tape detection
                case ES_TAPE_ALIGNED:
                    tape_aligned = true;
                    if (tape_aligned && side_detected) {
                        DB_printf("Tape and side detected, moving to next state\r\n");
                        NextState = NAVIGATE_TO_COLUMN_1;
                        MakeTransition = true;
                    }
                    break;
                
                case ES_NAV_ERROR:
                    puts("Navigation error! from put\r\n");
                    NextState = GAME_OVER;
                    MakeTransition = true;
                    break;
                default:
                    break;
            }

            break;

        case NAVIGATE_TO_COLUMN_1:
            switch (CurrentEvent.EventType) {
                case ES_ENTRY:
                    ThisEvent.EventType = ES_REQUEST_NEW_PLANNER_POLICY;
                    ThisEvent.EventParam = NAV_TO_COLUMN_1_POLICY;
                    PostPlannerPolicyService(ThisEvent);
                    break;

                case ES_NAVIGATOR_STATUS_CHANGE:
                    PostPlannerPolicyService(CurrentEvent);
                    break;

                case ES_PLANNER_POLICY_COMPLETE:
                    DB_printf("Policy complete, moving to next state\r\n");
                    //NextState = PROCESS_COLUMN;
                    // TODO: for checkoff purposes, disable this when deploy
                    NextState = GAME_OVER;
                    MakeTransition = true;
                    break;

                default:
                    // shouldn't go here
                    // add code to deal with exception
                    
                    break;
            }
            break;

        case NAVIGATE_TO_COLUMN_2:
            switch (CurrentEvent.EventType) {
                case ES_ENTRY:
                    ThisEvent.EventType = ES_REQUEST_NEW_PLANNER_POLICY;
                    ThisEvent.EventParam = NAV_TO_COLUMN_2_POLICY;
                    PostPlannerPolicyService(ThisEvent);
                    break;
                
                case ES_AT_COLUMN_INTERSECTION:
                case ES_TURN_COMPLETE:
                    ThisEvent.EventType = ES_CONTINUE_PLANNER_POLICY;
                    PostPlannerPolicyService(ThisEvent);
                    break;

                case ES_PLANNER_POLICY_COMPLETE:
                    NextState = PROCESS_COLUMN;
                    MakeTransition = true;
                    break;
                default:
                    break;
            }
        

        case PROCESS_COLUMN:
            CurrentEvent = DuringPROCESS_COLUMN(CurrentEvent);
            if (CurrentEvent.EventType == ES_COLUMN_COMPLETE) {
                if (CURRENT_COLUMN == 1) {
                    NextState = NAVIGATE_TO_COLUMN_2; // Move to next column
                } else {
                    NextState = GAME_OVER; // All processing complete
                }
                MakeTransition = true;
            }
            break;
        
        case GAME_OVER:
            // Final state, FSM terminates
            break;
    }

    if (MakeTransition) {
        DB_printf("Transitioning from State: %s to State: %s\r\n", GetStateName(CurrentState), GetStateName(NextState));
        DB_printf("\r\n");  
        CurrentEvent.EventType = ES_EXIT;
        RunPlannerHSM(CurrentEvent);
        CurrentState = NextState;
        RunPlannerHSM(EntryEventKind);
    }
    return ReturnEvent;
}

void StartPlannerHSM(ES_Event_t CurrentEvent) {
    CurrentState = INIT_PLANNER;
    RunPlannerHSM(CurrentEvent);
}

PlannerState_t QueryPlannerHSM(void) {
    return CurrentState;
}


