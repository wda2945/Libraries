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

typedef void (message_handler_t)(void *, int);

//suscribe to topic. Receive a callback when a message is received
ps_result_enum ps_subscribe(ps_topic_id_t topic_id, message_handler_t*);

//publish a message to a topic
ps_result_enum ps_publish(ps_topic_id_t topic_id, void *message, int length);

#ifdef __cplusplus
}
#endif


#endif /* ps_pubsub_h */
