#include <xc.h>
#include "PlannerPolicyService.h"
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_ShortTimer.h"
#include "SPIMasterService.h"

// Module-level variables
static uint8_t MyPriority;
static uint8_t CurrentPolicyStep=0;
static uint8_t CurrentPolicyIdx=0;


// Public function to initialize the service
bool InitPlannerPolicyService(uint8_t Priority) {
    MyPriority = Priority;
    initPlannerPolicyService();
    // Post the initial transition event
    ES_Event_t ThisEvent;
    ThisEvent.EventType = ES_INIT;
    if (ES_PostToService(MyPriority, ThisEvent) == True) {
        return True;
    } else {
        return False;
    }
}

void SetPolicy(uinit8_t policy_idx=0) {
    // Initialize any parameters here
    CurrentPolicyStep = 0;
    CurrentPolicyIdx = 0;
}

// 0 for padding
uint8_t NAV_POLICIES[][4] = {
    {NAV_CMD_MOVE_FORWARD, 0, 0, 0}, // NAV_TO_COLUMN_1
    {NAV_CMD_TURN_180, NAV_CMD_MOVE_FORWARD, NAV_CMD_TURN_LEFT, NAV_CMD_MOVE_FORWARD} // NAV_TO_COLUMN_2
};

// Public function to post events to the service
uint8_t PostPlannerPolicyService(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

void NextAction() {
    ES_Event_t newEvent;
    newEvent.EventType = ES_REQUEST_NEW_NAV_CMD;
    newEvent.EventParam = NAV_POLICIES[CurrentPolicyIdx] [CurrentPolicyStep];
    PostSPIMasterService(newEvent);
    CurrentPolicyStep += 1;
    // Check if current policy is finished.
    if (CurrentPolicyStep >= sizeof(NAV_POLICIES[CurrentPolicyIdx]) / sizeof(NAV_POLICIES[CurrentPolicyIdx][0]) || 
        NAV_POLICIES[CurrentPolicyIdx][CurrentPolicyStep] == 0) {
        ES_Event_t finishedEvent;
        finishedEvent.EventType = ES_PLANNER_POLICY_COMPLETE;
        // Post to planner current policy is complete
        PostPlannerPolicyService(finishedEvent);
    }
}

// Public function to run the service
ES_Event_ RunPlannerPolicyService(ES_Event_t ThisEvent) {
    ES_Event ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // Assume no errors

    switch (ThisEvent.EventType) {
        case ES_INIT:
            // Initialize the service
            SetPolicy()
            break;

        case ES_REQUEST_NEW_PLANNER_POLICY:
            uint8_t policy_idx = ThisEvent.EventParam;
            if (policy_idx >= sizeof(NAV_POLICIES) / sizeof(NAV_POLICIES[0])) {
                // Invalid policy index, return an error event
                ReturnEvent.EventType = ES_ERROR;
                ReturnEvent.EventParam = INVALID_POLICY_IDX;
                return ReturnEvent;
            }
            SetPolicy(policy_idx);
            NextAction();
        case ES_CONTINUE_PLANNER_POLICY:
            NextAction()
            break;

        // Add other event cases as needed

        default:
            break;
    }

    return ReturnEvent;
}