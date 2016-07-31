//
//  ps_pubsub_class.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_pubsub_class_hpp
#define ps_pubsub_class_hpp

#include "ps_common.h"
#include "common/ps_root_class.hpp"
#include "transport/ps_transport_class.hpp"
#include "queue/ps_queue_class.hpp"
#include <vector>
#include <map>
#include <set>

#include "pubsub_header.h"

//message to communicate subscribed topics
typedef struct {
	ps_topic_id_t topicIds[PS_MAX_TOPIC_LIST];
} ps_subscribe_message_t;

//struct to record subscriptions
typedef struct {
	message_handler_t 		*messageHandler;
	std::set<ps_topic_id_t> topicList;
} psClient_t;

extern const char *packet_type_names[];

//PubSub Singleton
class ps_pubsub_class : public ps_root_class {

public:
    int max_ps_packet;
    
    ////////////////////// PLUMBING CLIENT API
    //register for system packets - (subscribe)
    ps_result_enum register_object(ps_packet_type_t packet_type, ps_root_class *rcl);
    //send a system packet
    ps_result_enum publish_packet(ps_packet_type_t packet_type, const void *message, int length); //message excludes pubsub prefix

    ////////////////////// PUBLIC API
    //suscribe to a topic. Receive a callback when a message is received
    ps_result_enum subscribe(ps_topic_id_t topicId, message_handler_t *msgHandler);
    //publish a message to a topic
    ps_result_enum publish(ps_topic_id_t topicId, const void *message, int length);

    //manage transport link subscriptions
    ps_result_enum subscribe(ps_topic_id_t topicId, ps_transport_class *pst);

    //callbacks from transports
    void process_observed_data(ps_root_class *src, const void *msg, int length) override;
    void process_observed_event(ps_root_class *src, int event) override;

protected:
    ps_pubsub_class();
    ~ps_pubsub_class(){}

	////////////////////// BROKER DATABASE

    //registered plumbing clients
	std::set<ps_root_class *> registered_objects[PACKET_TYPES_COUNT];  //one per packet type

	//registered transport links
    std::set<ps_transport_class*> transports;
    
    //list of client structs
	std::set<psClient_t*> clientList;

	//internal queue of messages
    ps_queue_class *brokerQueue;

    /////////////////////// INTERNAL BROKER METHODS

    //send a message/packet to the subscribers
    //user topic messages
    void forward_topic_message(const void *message, int length);	//packet includes prefix

    //internal plumbing packets
    void forward_system_packet(const void *message, int length);	//packet includes prefix

    //request all subscriptions from remote broker
    void request_subscriptions(ps_transport_class *pst);

    //send all subscriptions to remote broker
    void send_subscriptions(ps_transport_class *pst);

    //broker thread
	void broker_thread_method();

	void refresh_network();

	friend ps_pubsub_class& the_broker();

	//mutex
    DEFINE_MUTEX(pubsubMtx);
    
};

ps_pubsub_class& the_broker();

#endif /* ps_pubsub_class_hpp */
