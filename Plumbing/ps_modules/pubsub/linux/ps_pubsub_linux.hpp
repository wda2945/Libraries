//
//  ps_pubsub_linux.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_pubsub_linux_hpp
#define ps_pubsub_linux_hpp

#include <vector>

#include "pubsub/ps_pubsub_class.hpp"
#include "transport/linux/ps_transport_linux.hpp"
#include "network/ps_network.hpp"
#include "queue/linux/ps_queue_linux.hpp"

typedef struct {
	void (*messageHandler)(void *, size_t);
	ps_transport_class *transport;
	std::vector<ps_topic_Id_t> topicList;
}psClient_t;

//PubSub Singleton
class ps_pubsub_linux : public ps_pubsub_class {

	std::vector<psClient_t> clientList;

public:
    ps_queue_linux *brokerQueue;

    ps_pubsub_linux();

	void broker_thread_method();

//api

    //suscribe to named topic. Receive a callback when a message is received
    ps_result_enum subscribe(const char * topicName, void (*msgHandler)(void *, size_t));
	ps_result_enum subscribe(ps_topic_Id_t topicId, void (*msgHandler)(void *, size_t));
	ps_result_enum subscribe(ps_topic_Id_t topicId, ps_transport_class *pst);
	ps_result_enum subscribe(const char * topicName, ps_transport_class *pst);
    
    //publish a message to a named topic
    ps_result_enum publish(const char * topicName, void *message, size_t length);

    //send a message to the subscribers
    void send_message(ps_pubsub_prefix_t prefix, uint8_t *message, size_t length);
    
    void copy_message_to_q(uint8_t* message, size_t len);

    friend ps_pubsub_class& the_broker();
};

#endif /* ps_pubsub_linux_hpp */
