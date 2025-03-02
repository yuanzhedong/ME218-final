#include <xc.h>
#include "PlannerPolicyService.h"
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "SPIMasterService.h"

// Module-level variables
static uint8_t MyPriority;
static uint8_t CurrentPolicyStep=0;
static uint8_t CurrentPolicyIdx=0;
static uint8_t PrevSentCmd = 0;


// Public function to initialize the service
bool InitPlannerPolicyService(uint8_t Priority) {
    MyPriority = Priority;
    // Post the initial transition event
    ES_Event_t ThisEvent;
    ThisEvent.EventType = ES_INIT;
    if (ES_PostToService(MyPriority, ThisEvent) == true) {
        return true;
    } else {
        return false;
    }
}

void SetPolicy(uint8_t policy_idx) {
    // Initialize any parameters here
    CurrentPolicyStep = 0;
    CurrentPolicyIdx = policy_idx;
}

// 0 for padding
uint8_t NAV_POLICIES[][6][2] = {
    {{NAV_CMD_MOVE_BACKWARD, 10}, {NAV_CMD_TURN_LEFT, 10}, {NAV_CMD_MOVE_FORWARD, 10}, {NAV_CMD_STOP, 0}, {0, 0}, {0, 0}}, // NAV_TO_COLUMN_1
    {{NAV_CMD_TURN_CW, 2}, {NAV_CMD_MOVE_FORWARD, 6}, {NAV_CMD_TURN_LEFT, 3}, {NAV_CMD_MOVE_FORWARD, 4}, {NAV_CMD_STOP, 0}, {0, 0}} // NAV_TO_COLUMN_2
};

// Public function to post events to the service
bool PostPlannerPolicyService(ES_Event_t ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

void NextAction() {    
    ES_Event_t newEvent;
    newEvent.EventType = ES_NEW_NAV_CMD;
    newEvent.EventParam = NAV_POLICIES[CurrentPolicyIdx] [CurrentPolicyStep][0];
    DB_printf("[POLICY] Posting command: %s\r\n", TranslateNavCmdToStr(newEvent.EventParam));
    PrevSentCmd = newEvent.EventParam;
    PostSPIMasterService(newEvent);
    CurrentPolicyStep += 1;
    // Check if current policy is finished.
    if (NAV_POLICIES[CurrentPolicyIdx][CurrentPolicyStep][0] == 0) {
        ES_Event_t finishedEvent;
        finishedEvent.EventType = ES_PLANNER_POLICY_COMPLETE;
        // Post to planner current policy is complete
        DB_printf("[POLICY] Current policy is complete\r\n");
        PostPlannerHSM(finishedEvent);
        ES_Timer_StopTimer(PLANNER_POLICY_TIMER);
    }
}

// Public function to run the service
ES_Event_t RunPlannerPolicyService(ES_Event_t ThisEvent) {
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // Assume no errors
    uint8_t policy_idx; // Move declaration here
    switch (ThisEvent.EventType) {
        case ES_INIT:
            // Initialize the service
            //SetPolicy();
            break;

        case ES_REQUEST_NEW_PLANNER_POLICY:
            policy_idx = ThisEvent.EventParam;
            if (policy_idx >= sizeof(NAV_POLICIES) / sizeof(NAV_POLICIES[0])) {
                DB_printf("Invalid policy index %d\r\n", policy_idx);
                ReturnEvent.EventType = ES_ERROR;
                ReturnEvent.EventParam = INVALID_POLICY_IDX;
                return ReturnEvent;
            }
            SetPolicy(policy_idx);
            ES_Timer_InitTimer(PLANNER_POLICY_TIMER, NAV_POLICIES[CurrentPolicyIdx] [CurrentPolicyStep][1] * 1000);
            NextAction();
            break;

        case ES_NAVIGATOR_STATUS_CHANGE:
            if (ThisEvent.EventParam == (PrevSentCmd + 1)) {
                DB_printf("[POLICY] Received status: %s\r\n", TranslateNavStatusToStr(ThisEvent.EventParam));
                NextAction();
            }
            break;

        case ES_TIMEOUT:
            DB_printf("[POLICY] Timeout event received\r\n");
            ES_Timer_InitTimer(PLANNER_POLICY_TIMER, NAV_POLICIES[CurrentPolicyIdx][CurrentPolicyStep][1] * 1000);
            NextAction();
            
        // Add other event cases as needed

        default:
            DB_printf("[POLICY] Received unknown event %d\r\n", ThisEvent.EventType);
            break;
    }

    return ReturnEvent;
}