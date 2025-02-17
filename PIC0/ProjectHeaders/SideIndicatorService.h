#ifndef MOTOR_SERVICE_H
#define MOTOR_SERVICE_H

#include "ES_Configure.h"
#include "ES_Framework.h"
#include <stdint.h>
#include <stdbool.h>

/*----------------------------- Module Defines ----------------------------*/

// Define the available side detection colors using an enum
typedef enum {
    BLUE,  // Corresponds to 3333 Hz
    GREEN  // Corresponds to 2000 Hz
} SideColor_t;

/*---------------------------- Public Functions ---------------------------*/

/**
 * @brief Initializes the Motor Service
 * 
 * @param Priority Priority of this service
 * @return true if initialization successful, false otherwise
 */
bool InitMotorService(uint8_t Priority);

/**
 * @brief Posts an event to the Motor Service queue
 * 
 * @param ThisEvent The event to be posted
 * @return true if event was posted successfully, false otherwise
 */
bool PostMotorService(ES_Event_t ThisEvent);

/**
 * @brief Runs the Motor Service, handling events and controlling the motor
 * 
 * @param ThisEvent The event to process
 * @return ES_Event_t Returns ES_NO_EVENT if no error, ES_ERROR otherwise
 */
ES_Event_t RunMotorService(ES_Event_t ThisEvent);

/*--------------------------- Private Functions --------------------------*/

/**
 * @brief Configures Timer 2 for PWM operation
 */
void ConfigTimer2(void);

/**
 * @brief Configures Output Compare (OC1) for PWM control
 */
void ConfigPWM_OC1(void);

/**
 * @brief Configures Output Compare (OC3) for PWM control
 */
void ConfigPWM_OC3(void);

/**
 * @brief Configures Timer 3 for input capture
 */
void ConfigTimer3(void);

/**
 * @brief Configures Input Capture 3 (IC3) for detecting encoder pulses
 */
void Config_IC3(void);

/**
 * @brief ISR for Input Capture 3, responsible for frequency detection
 */
void __ISR(_INPUT_CAPTURE_3_VECTOR, IPL7SOFT) IC3ISR(void);

/**
 * @brief ISR for Timer 3, responsible for handling timer rollovers
 */
void __ISR(_TIMER_3_VECTOR, IPL6SOFT) Timer3_ISR(void);

#endif // MOTOR_SERVICE_H
