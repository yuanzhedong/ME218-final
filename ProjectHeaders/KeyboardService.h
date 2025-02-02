#ifndef KEYBOARD_SERVICE_H
#define KEYBOARD_SERVICE_H

#include "ES_Types.h"

bool InitKeyboardService(uint8_t Priority);
bool PostKeyboardService(ES_Event_t ThisEvent);
ES_Event_t RunKeyboardService(ES_Event_t ThisEvent);

#endif /* KEYBOARD_SERVICE_H */