//
//  ps_pubsub.h
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_pubsub_h
#define ps_pubsub_h

#include "ps_types.h"

//pubsub client api

#ifdef __cplusplus
extern "C" {
#endif

//suscribe to names topic. Receive a callback when a message is received
ps_result_enum ps_subscribe(const char * topicName, void (*msgHandler)(void *, size_t));

//publish a message to a named topic
ps_result_enum ps_publish(const char * topicName, void *messageRef, size_t length);

#ifdef __cplusplus
}
#endif


#endif /* ps_pubsub_h */
