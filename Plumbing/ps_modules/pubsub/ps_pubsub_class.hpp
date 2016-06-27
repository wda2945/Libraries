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


//pubsub message prefix
typedef struct {
	ps_packet_source_t		packet_source;
	ps_packet_type_t 		packet_type;
	ps_topic_id_t 			topic_id;
} ps_pubsub_prefix_t;

//message to communicate subscribed topics
typedef struct {
	ps_topic_id_t topicIds[PS_MAX_TOPIC_LIST];
} ps_subscribe_message_t;

//struct to record subscriptions
typedef struct {
	message_handler_t 		*messageHandler;	//used for local clients
	ps_packet_source_t  	transport_tag;			//used for remote clients
	std::set<ps_topic_id_t> topicList;
}psClient_t;

//PubSub Singleton
class ps_pubsub_class : public ps_root_class {

public:
    ////////////////////// PLUMBING CLIENT API
    //register for system packets
    ps_result_enum register_object(const ps_packet_type_t packet_type, ps_root_class *rcl);
    //send a system packet
    ps_result_enum publish_packet(const ps_packet_type_t packet_type, void *message, int length);

    ////////////////////// PUBLIC API
    //suscribe to A topic. Receive a callback when a message is received
    ps_result_enum subscribe(const ps_topic_id_t topicId, message_handler_t *msgHandler);
    //publish a message to a topic
    ps_result_enum publish(const ps_topic_id_t topicId, void *message, int length);

    friend void broker_transport_data_callback(ps_transport_class *pst, void *message, int len);
    friend void broker_transport_status_callback(ps_transport_class *pst, ps_transport_status_enum pstStatus);

protected:
    ps_pubsub_class(){}
    ~ps_pubsub_class(){}
	////////////////////// BROKER DATABASE
	ps_packet_source_t	next_source_tag = 1;

	//registered plumbing clients
	ps_root_class *registered_objects[PACKET_TYPES_COUNT];

	//registered transport links
    std::map<ps_packet_source_t, ps_transport_class*> transports;

    //list of client structs
	std::vector<psClient_t> clientList;

	//internal queue of messages
    ps_queue_class *brokerQueue;

    /////////////////////// INTERNAL BROKER METHODS
    //manage link subscriptions
    ps_result_enum subscribe(const ps_topic_id_t topicId, const ps_packet_source_t tag);
    ps_result_enum unsubscribe_all(const ps_packet_source_t tag);						//remove gone transport

    //send a message to the subscribers
    void publish_message(ps_pubsub_prefix_t *prefix, void *message, int length);

    //request all subscriptions from remote broker
    void request_subscriptions(const ps_packet_source_t tag);
    //send all subscriptions to remote broker
    void send_subscriptions(const ps_packet_source_t tag);

    //broker thread
	void broker_thread_method();

	void refresh_network();

	friend ps_pubsub_class& the_broker();
};

ps_pubsub_class& the_broker();

#endif /* ps_pubsub_class_hpp */
