//
//  ps_pubsub_class.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_pubsub_class_hpp
#define ps_pubsub_class_hpp

#include "ps_types.h"
#include "common/ps_root_class.hpp"
#include "transport/ps_transport_class.hpp"
#include <vector>

typedef uint16_t ps_topic_Id_t;
typedef uint16_t ps_message_Id_t;

#define PS_SUBSCRIBE_TOPIC "subscribe"
#define PS_SUBSCRIPTION_TOPIC "subscriptions"

extern ps_topic_Id_t subscribeTopic;		//subscription list
extern ps_topic_Id_t subscriptionTopic;	//topic to request subscrition list

#ifdef __cplusplus
extern "C" {
#endif

ps_topic_Id_t ps_topic_hash(const char *topicName);	//char[] to uint16_t

#ifdef __cplusplus
}
#endif

typedef struct {
	ps_topic_Id_t topicId;
	uint8_t flags;
	uint8_t	source;
} ps_pubsub_prefix_t;

const int PS_TOPIC_OFFSET = 0;
const int PS_FLAGS_OFFSET = sizeof(ps_topic_Id_t);
const int PS_MESSAGE_OFFSET = PS_FLAGS_OFFSET + 2;

const int PS_SUBSCRIBE_MESSAGE_ID = 0xAA;
typedef struct {
	ps_message_Id_t msgId;		//PS_SUBSCRIBE_MESSAGE_ID
	ps_topic_Id_t topicIds[PS_MAX_TOPIC_LIST];
} ps_subscribe_message_t;

//PubSub Singleton
class ps_pubsub_class : public ps_root_class {
public:

    std::vector<ps_transport_class*> transports;

//api methods
    
    //suscribe to named topic. Receive a callback when a message is received
    virtual ps_result_enum subscribe(const char * topicName, void (*msgHandler)(void *, size_t)) = 0;
    virtual ps_result_enum subscribe(ps_topic_Id_t topicId, void (*msgHandler)(void *, size_t)) = 0;
    virtual ps_result_enum subscribe(ps_topic_Id_t topicId, ps_transport_class *pst) = 0;
    virtual ps_result_enum subscribe(const char * topicName, ps_transport_class *pst) = 0;
    
    //publish a message to a named topic
    virtual ps_result_enum publish(const char * topicName, void *message, size_t length) = 0;

    virtual void copy_message_to_q(uint8_t* message, size_t len) = 0;
};

ps_pubsub_class& the_broker();

#endif /* ps_pubsub_class_hpp */
