//
//  ps_pubsub_class.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "pubsub/ps_pubsub_class.hpp"
#include "network/ps_network.hpp"
#include <set>

#include "syslog/ps_syslog_message.h"
#include "registry/ps_registry_message.h"

ps_pubsub_class::ps_pubsub_class() : ps_root_class(std::string("broker"))
{
	int syslog_pkt 		= sizeof(ps_syslog_message_t);
	int registry_pkt 	= sizeof(ps_update_packet_t);
	int user_pkt		= PS_MAX_MESSAGE;

#define max(a,b) (a > b ? a : b)

	max_ps_packet = max(syslog_pkt, max(registry_pkt, user_pkt)) + sizeof(ps_pubsub_header_t) + 8;
}
////////////////////////////// INTERNAL BROKER METHODS

void ps_pubsub_class::broker_thread_method()
{
    PS_DEBUG("pub: broker thread started");
    
    while(1)
    {
        int len;
        //next message from queue
        void *q_message = brokerQueue->get_next_message(-1, &len);

        //access the pubsub prefix
        ps_pubsub_header_t* prefix = static_cast<ps_pubsub_header_t*>(q_message);
        
        if (prefix->packet_type > PACKET_TYPES_COUNT)
        {
            PS_ERROR("pub: packet_type %i!", prefix->packet_type);
        }
        else
        {
//            PS_DEBUG("pub: %s", packet_type_names[prefix->packet_type]);
            
            switch(prefix->packet_type)
            {
                case PUBLISH_PACKET:
                    forward_topic_message(q_message, len);
                    break;
                    
                    //packet for other plumbing function
                default:
                    forward_system_packet(q_message, len);
                    break;
            }
        }
    }
    
}

//called by broker thread to send a message to all subscribers
//user topics
void ps_pubsub_class::forward_topic_message(const void *msg, int len)
{
	//access the pubsub prefix
	const ps_pubsub_header_t* prefix = static_cast<const ps_pubsub_header_t*>(msg);

//	PS_DEBUG("pub: publish_message to topic %i", prefix->packet_type);

	//user message to be published
    void *message = (void*) ((uint8_t*) msg + sizeof(ps_pubsub_header_t));
    int length = len - sizeof(ps_pubsub_header_t);
    
    LOCK_MUTEX(pubsubMtx);
    
	//iterate client list
	for (psClient_t *client : clientList)
	{
		//find the subscribed topic
		auto tId = client->topicList.find(prefix->topic_id);
		if (tId != client->topicList.end())
		{
			//this client is subscribed
            PS_DEBUG("pub: sending topic %i to message handler", prefix->topic_id);
            //pointer to messageHandler C function
            (client->messageHandler)(message, length);
		}
	}
    //iterate transports
    for (auto trns : transports)
    {
        //find the subscribed topic
        auto tId = trns->topicList.find(prefix->topic_id);
        if (tId != trns->topicList.end())
        {
            //this transport is subscribed
            PS_DEBUG("pub: sending topic %i to %s", prefix->topic_id, trns->name.c_str());
            trns->send_packet(msg, len);
        }
    }
	UNLOCK_MUTEX(pubsubMtx);
}
//called by broker thread to send a message to all subscribers
//these messages go to plumbing 
void ps_pubsub_class::forward_system_packet(const void *msg, int len)
{
	//access the pubsub prefix
	ps_pubsub_header_t* prefix = (ps_pubsub_header_t*)(msg);
	ps_packet_type_t packet_type = (prefix->packet_type);

	if (packet_type < PACKET_TYPES_COUNT)
	{
//		PS_DEBUG("pub: publish_system_packet(%s)", packet_type_names[packet_type]);

		LOCK_MUTEX(pubsubMtx);

        if (prefix->packet_source == SOURCE)
        {
            //local source - send to all transports
            for (auto trns : transports)
            {
                PS_DEBUG("pub: sending %s to %s", packet_type_names[packet_type], trns->name.c_str());
                trns->send_packet(msg, len);
            }
        }
        else
        {
            //remote source - send to registered objects
            void *message = (void*) ((uint8_t*) msg + sizeof(ps_pubsub_header_t));
            int length = len - sizeof(ps_pubsub_header_t);
            
            std::set<ps_root_class *> rcl_set = registered_objects[packet_type];
            
            for (ps_root_class *rcl : rcl_set)
            {
                PS_DEBUG("pub: sending %s to %s", packet_type_names[packet_type], rcl->name.c_str());
                rcl->message_handler(prefix->packet_source, packet_type, message, length);
            }
        }
		UNLOCK_MUTEX(pubsubMtx);
	}
	else
	{
		PS_ERROR("pub: packet_type %i!", packet_type);
	}
}

//request all subscriptions from remote broker
void ps_pubsub_class::request_subscriptions(ps_transport_class *pst)
{
    PS_DEBUG("pub: request_subs from %s", pst->name.c_str());
    ps_pubsub_header_t prefix;
    prefix.packet_type = SEND_SUBS_PACKET;
    prefix.packet_source = SOURCE;                //actually, now a destination to identify the transport
    
    pst->send_packet(static_cast<const void*>(&prefix), (int) sizeof(ps_pubsub_header_t));
}

//send all subscriptions to remote broker
void ps_pubsub_class::send_subscriptions(ps_transport_class *pst)
{
    PS_DEBUG("pub: send_subs to %s", pst->name.c_str());
    
    struct {
        ps_pubsub_header_t prefix;
        ps_subscribe_message_t msg;
    } sub_msg;
    
    sub_msg.prefix.packet_type = SUBSCRIBE_PACKET;
    sub_msg.prefix.packet_source = SOURCE;
    
    //publish topic list for this node
    
    //iterate callout list, collect topics
    std::set<ps_topic_id_t> topicSet;
    
    LOCK_MUTEX(pubsubMtx);
    
    for (psClient_t *client : clientList)
    {
        for (const auto& tId : client->topicList)
        {
            topicSet.insert(tId);		//NB no duplicates allowed in a set
        }
    }
    
    //TODO? Add topics from other transports?
    
    UNLOCK_MUTEX(pubsubMtx);
    
    //send subs list message(s)
    
    int i = 0;
    for (const auto& tId : topicSet)
    {
        sub_msg.msg.topicIds[i++] = tId;
        
        if (i >= PS_MAX_TOPIC_LIST)	//this message is full
        {
            pst->send_packet(static_cast<const void*>(&sub_msg), (int) sizeof(sub_msg));
            i = 0;
        }
    }
    if (i)
    {
        //last message incomplete, more to send
        sub_msg.msg.topicIds[i] = 0;
        pst->send_packet(static_cast<const void*>(&sub_msg), (int) sizeof(sub_msg));
    }
}

void ps_pubsub_class::refresh_network()
{
    PS_DEBUG("pub: refresh_network()");
    the_network().add_event_observer(this);
    the_network().iterate_transports(this);
}

/////////////////////////// C API -> METHOD

//suscribe to topic. Receive a callback when a message is received
ps_result_enum ps_subscribe(ps_topic_id_t topic_id, message_handler_t *msgHandler)
{
    return the_broker().subscribe(topic_id, msgHandler);
}

//publish a message to a topic
ps_result_enum ps_publish(ps_topic_id_t topic_id, const void *message, int length)
{
    return the_broker().publish(topic_id, message, length);
}

/////////////////////////// PLUMBING SERVICES

int get_max_ps_packet()
{
	return the_broker().max_ps_packet;
}

//register for system packets
ps_result_enum ps_pubsub_class::register_object(ps_packet_type_t packet_type, ps_root_class *rcl)
{
    int type = packet_type;
    
    LOCK_MUTEX(pubsubMtx);
    PS_DEBUG("pub: register_object(%s, %s)", packet_type_names[type], rcl->name.c_str());

    if (type < PACKET_TYPES_COUNT)
    {
        registered_objects[type].insert(rcl);
    }
    UNLOCK_MUTEX(pubsubMtx);
    return PS_OK;
}

//called by other plumbing objects to send a system packet
ps_result_enum ps_pubsub_class::publish_packet(ps_packet_type_t packet_type, const void *message, int length)
{
    
    ps_pubsub_header_t prefix;
    prefix.packet_type   = packet_type;  //top bit must be 1
    prefix.packet_source = SOURCE;
    
    PS_DEBUG("pub: publish_packet(%s)", packet_type_names[(packet_type)]);
    
    if (length <= (int) max_ps_packet)
    {
        //queue for broker thread
        brokerQueue->copy_2message_parts_to_q( &prefix, sizeof(ps_pubsub_header_t), message, length);
        return PS_OK;
    }
    else
    {
        PS_ERROR("pub: %s packet too big", packet_type_names[packet_type ]);
        return PS_LENGTH_ERROR;
    }
    
    return PS_OK;
}


//////////////////////////transport api

//suscribe to topic Id. Transport version.
ps_result_enum ps_pubsub_class::subscribe(ps_topic_id_t topicId, ps_transport_class *pst)
{
    PS_DEBUG("pub: subscribe to %i from %s", topicId, pst->name.c_str());
    
    LOCK_MUTEX(pubsubMtx);
    
    //insert the topic
    pst->topicList.insert(topicId);	//add new subscription

    UNLOCK_MUTEX(pubsubMtx);
    return PS_OK;
}

//transport callbacks, data and events

void ps_pubsub_class::process_observed_data(ps_root_class *_pst, const void *message, int len)
{
    ps_transport_class *pst = dynamic_cast<ps_transport_class *>(_pst);
    
    //incoming from Transport
    const ps_pubsub_header_t *prefix = static_cast<const ps_pubsub_header_t*>(message);
    
    pst->transport_source = static_cast<Source_t>(prefix->packet_source);
    
    if (prefix->packet_type < PACKET_TYPES_COUNT)
    {
        if (prefix->packet_type == PUBLISH_PACKET)
        {
            PS_DEBUG("pub: rx topic %i from %s", prefix->topic_id, pst->name.c_str());
        }
        else
        {
            PS_DEBUG("pub: rx %s from %s",packet_type_names[prefix->packet_type], pst->name.c_str());
        }
        
        switch(prefix->packet_type)
        {
            case SEND_SUBS_PACKET:
                send_subscriptions(pst);
                break;
            case SUBSCRIBE_PACKET:
                //its a list of topics needed
            {
                ps_subscribe_message_t *msg = (ps_subscribe_message_t*) ((uint8_t*) message + sizeof(ps_pubsub_header_t));
                int i = 0;
                //process subscribe
                while (msg->topicIds[i] && i < PS_MAX_TOPIC_LIST) {
                    //subscribe to each topic listed
                    subscribe(msg->topicIds[i], pst);
                    i++;
                };
            }
                break;
                
            default:
                //queue it for the broker thread
                brokerQueue->copy_message_to_q(message, len);
                break;
        }
    }
    else
    {
        PS_DEBUG("pub: rx bad pkt %i from %s", prefix->packet_type, pst->name.c_str());
        //discard
    }
}

void ps_pubsub_class::process_observed_event(ps_root_class *_pst, int _ev)
{
    ps_transport_class *pst = dynamic_cast<ps_transport_class *>(_pst);
    ps_transport_event_enum ev = (ps_transport_event_enum) _ev;
    
    PS_DEBUG("pub: status %s from %s", transport_event_names[ev], pst->name.c_str());
    
    bool send_subs {false};
    
    LOCK_MUTEX(pubsubMtx);
    
    switch(ev)
    {
        case PS_TRANSPORT_ONLINE:
            send_subs = true;
            break;
        case PS_TRANSPORT_OFFLINE:
            break;
        case PS_TRANSPORT_ADDED:
        {
            transports.insert(pst);
            
            pst->add_data_observer(this);
            send_subs = true;
        }
            break;
        case PS_TRANSPORT_REMOVED:
        {
             //remove transports entry
            auto pos = transports.find(pst);
            if (pos != transports.end())
            {
                transports.erase(pos);
            }
        }
            break;
        default:
            break;
    }
    
    UNLOCK_MUTEX(pubsubMtx);
    
    if (send_subs)
    {
        request_subscriptions(pst);
        send_subscriptions(pst);
    }
}

////////////////////////////public api

//suscribe to topic Id. Receive a callback when a message is received
ps_result_enum ps_pubsub_class::subscribe(ps_topic_id_t topicId, message_handler_t *msgHandler)
{
    LOCK_MUTEX(pubsubMtx);
    
    //find the client
    for (psClient_t *client : clientList)
    {
        if (client->messageHandler == msgHandler)
        {
            client->topicList.insert(topicId);	//add new subscription
            
            PS_DEBUG("pub: subscribed to topic %i", topicId);
            UNLOCK_MUTEX(pubsubMtx);
            return PS_OK;
        }
    }
    //add new client
    {
        psClient_t *newClient = new psClient_t();
        newClient->messageHandler 	= msgHandler;
        newClient->topicList.insert(topicId);	//add new subscription
        
        clientList.insert(newClient);
        
        PS_DEBUG("pub: new client subscribed to topic %i", topicId);
    }
    UNLOCK_MUTEX(pubsubMtx);
    return PS_OK;
}

//publish a user message to a topic
ps_result_enum ps_pubsub_class::publish(ps_topic_id_t topicId, const void *message, int length)
{
    ps_pubsub_header_t prefix;
    prefix.packet_type   = PUBLISH_PACKET;
    prefix.topic_id   = topicId;
    prefix.packet_source = SOURCE;
    
    if (length <= (int) max_ps_packet)
    {
        PS_DEBUG("pub: publish to topic %i", topicId);
        //queue for broker thread
        brokerQueue->copy_2message_parts_to_q( &prefix, sizeof(ps_pubsub_header_t), message, length);
        return PS_OK;
    }
    else
    {
        PS_ERROR("pub: packet to topic %i too big", topicId);
        return PS_LENGTH_ERROR;
    }
}

#define packet_macro(e, name, qos) name,
const char *packet_type_names[] = {
#include "common/ps_packet_macros.h"
};
#undef packet_macro
