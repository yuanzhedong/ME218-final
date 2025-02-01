#ifndef ENCODER_SERVICE_H
#define ENCODER_SERVICE_H

#include "ES_Events.h"
#include "ES_Port.h"

// Public Function Prototypes
uint8_t InitEncoderService(uint8_t Priority);
uint8_t PostEncoderService(ES_Event_t ThisEvent);
ES_Event_t RunEncoderService(ES_Event_t ThisEvent);

#endif /* ENCODER_SERVICE_H */