#include <xc.h>
#include "PlannerPolicyService.h"
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "SPIMasterService.h"

// Module-level variables
static uint8_t MyPriority;
static uint8_t CurrentPolicyStep=0;
static uint8_t CurrentPolicyIdx=0;


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
uint8_t NAV_POLICIES[][4] = {
    {NAV_CMD_MOVE_BACKWARD, NAV_CMD_TURN_LEFT, NAV_CMD_MOVE_FORWARD, 0}, // NAV_TO_COLUMN_1
    {NAV_CMD_TURN_CW, NAV_CMD_MOVE_FORWARD, NAV_CMD_TURN_LEFT, NAV_CMD_MOVE_FORWARD} // NAV_TO_COLUMN_2
};

// Public function to post events to the service
bool PostPlannerPolicyService(ES_Event_t ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

void NextAction() {
    ES_Event_t newEvent;
    newEvent.EventType = ES_NEW_NAV_CMD;
    newEvent.EventParam = NAV_POLICIES[CurrentPolicyIdx] [CurrentPolicyStep];
    DB_printf("[POLICY] Posting new command %d\r\n", newEvent.EventParam);
    PostSPIMasterService(newEvent);
    CurrentPolicyStep += 1;
    // Check if current policy is finished.
    if (CurrentPolicyStep >= sizeof(NAV_POLICIES[CurrentPolicyIdx]) / sizeof(NAV_POLICIES[CurrentPolicyIdx][0]) || 
        NAV_POLICIES[CurrentPolicyIdx][CurrentPolicyStep] == 0) {
        ES_Event_t finishedEvent;
        finishedEvent.EventType = ES_PLANNER_POLICY_COMPLETE;
        // Post to planner current policy is complete
        PostPlannerHSM(finishedEvent);
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
            NextAction();
            break;

        case ES_CONTINUE_PLANNER_POLICY:
            DB_printf("[POLICY] Continuing policy %d\r\n", CurrentPolicyIdx);
            NextAction();
            break;

        // Add other event cases as needed

        default:
            DB_printf("[POLICY] Received unknown event %d\r\n", ThisEvent.EventType);
            break;
    }

    return ReturnEvent;
}