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

int max_ps_packet {0};

ps_pubsub_class::ps_pubsub_class()
{
	int syslog_pkt 		= sizeof(ps_syslog_message_t);
	int registry_pkt 	= sizeof(ps_update_packet_t);
	int user_pkt		= PS_MAX_MESSAGE;

#define max(a,b) (a > b ? a : b)

	max_ps_packet = max(syslog_pkt, max(registry_pkt, user_pkt));
}
////////////////////////////// INTERNAL BROKER METHODS

void ps_pubsub_class::broker_thread_method()
{
    PS_DEBUG("pub: broker thread started");
    request_subscriptions(0);	//at startup
    
    while(1)
    {
        int len;
        //next message from queue
        void *q_message = brokerQueue->get_next_message(0, &len);
        
        //access the pubsub prefix
        ps_pubsub_header_t* prefix = (ps_pubsub_header_t*)(q_message);

        if ((prefix->packet_type & 0x80) == 0)
        {
            //pass it on
            publish_user_message( q_message, len);
        }
        else
        {
            int packet_type = (prefix->packet_type & 0x7f);
            
        if (packet_type < PACKET_TYPES_COUNT)
        {
            PS_DEBUG("pub: %s", packet_type_names[packet_type]);
        }
        else {
            PS_ERROR("pub: packet_type %i!", packet_type);
        }
        
        switch(prefix->packet_type)
        {
            case SEND_SUBS_PACKET:
                
                //it's a call for subscribed topics
                send_subscriptions(prefix->packet_source);
                break;
            case SUBSCRIBE_PACKET:
            {
                ps_subscribe_message_t *msg = (ps_subscribe_message_t*) ((uint8_t*) q_message + sizeof(ps_pubsub_header_t));
                int i = 0;
                //process subscribe
                while (msg->topicIds[i] && i < PS_MAX_TOPIC_LIST) {
                    //subscribe to each topic listed
                    subscribe(msg->topicIds[i], prefix->packet_source);
                    i++;
                };
            }
                break;
            case TRANSPORT_ADDED_PACKET:
                request_subscriptions(prefix->packet_source);
                send_subscriptions(prefix->packet_source);
                break;
            case TRANSPORT_REMOVED_PACKET:
                break;
            case TRANSPORT_ONLINE_PACKET:
                break;
            case TRANSPORT_OFFLINE_PACKET:
                break;
            default:
            	publish_system_message(q_message, len);
                break;
        }
        }
    }
}

//called by broker thread to send a message to all subscribers
void ps_pubsub_class::publish_user_message(const void *msg, int len)
{
	//access the pubsub prefix
	ps_pubsub_header_t* prefix = (ps_pubsub_header_t*)(msg);

	PS_DEBUG("pub: publish_message to topic %i", prefix->packet_type);

	//user message to be published
    void *message = (void*) ((uint8_t*) msg + sizeof(ps_pubsub_header_t));
    int length = len - sizeof(ps_pubsub_header_t);
    
	//iterate client list
	for (const auto& client : clientList)
	{
		//find the subscribed topic
		auto tId = client.topicList.find(prefix->packet_type);
		if (tId != client.topicList.end())
		{
			//this client is subscribed
			if (client.messageHandler != nullptr)
			{
				PS_DEBUG("pub: sending to message handler");
				//pointer to messageHandler C function
				(*client.messageHandler)(message, length);
			}
			else if (client.transport_tag != prefix->packet_source)
			{
				//pointer to ps_transport subclass
				auto pos = transports.find(client.transport_tag);
				if (pos != transports.end())
				{
					PS_DEBUG("pub: sending to transport %s", pos->second->name.c_str());
					pos->second->send_packet2(prefix, sizeof(ps_pubsub_header_t), message, length);
				}
			}
			break;
		}
	}
	return;
}
//called by broker thread to send a message to all subscribers
void ps_pubsub_class::publish_system_message(const void *msg, int len)
{
	//access the pubsub prefix
	ps_pubsub_header_t* prefix = (ps_pubsub_header_t*)(msg);
	ps_packet_type_t packet_type = (prefix->packet_type & 0x7f);

	PS_DEBUG("pub: publish_system_message(%s)", packet_type_names[packet_type]);

 	switch(packet_type)
	{
	case SUBSCRIBE_PACKET:
	case SEND_SUBS_PACKET:
		//only looking for transports
		for (auto pos = transports.begin(); pos != transports.end(); pos++)
		{
			if ((pos->first == prefix->packet_source) || (prefix->packet_source == 0))
			{
				PS_DEBUG("pub: sending to transport");
				pos->second->send_packet(msg, len);
			}
		}
		break;
	default:
		//plumbing internal message
		if (packet_type < PACKET_TYPES_COUNT)
		{
			void *message = (void*) ((uint8_t*) msg + sizeof(ps_pubsub_header_t));
			int length = len - sizeof(ps_pubsub_header_t);

			ps_root_class *rcl = registered_objects[packet_type];
			if (rcl) rcl->message_handler(prefix->packet_source, packet_type, message, length);
		}
		else
		{
			PS_ERROR("pub: unknown packet %i", packet_type);
		}
		break;
	}
}

//request all subscriptions from remote broker
void ps_pubsub_class::request_subscriptions(ps_packet_source_t tag)
{
    PS_DEBUG("pub: request_subscriptions()");
    ps_pubsub_header_t prefix;
    prefix.packet_type = SEND_SUBS_PACKET;
    prefix.packet_source = tag;
    
    publish_system_message(&prefix, sizeof(ps_pubsub_header_t));
}

//send all subscriptions to remote broker
void ps_pubsub_class::send_subscriptions(ps_packet_source_t tag)
{
    PS_DEBUG("pub: send_subscriptions()");

    struct {
    ps_pubsub_header_t prefix;
    ps_subscribe_message_t msg;
    } sub_msg;
    
    sub_msg.prefix.packet_type = SUBSCRIBE_PACKET;
    sub_msg.prefix.packet_source = tag;
    
    //publish topic list for this node
    
    //iterate callout list, collect topics
    std::set<ps_topic_id_t> topicSet;
    
    for (const auto& client : clientList)
    {
        for (const auto& tId : client.topicList)
        {
            topicSet.insert(tId);		//NB no duplicates allowed in a set
        }
    }
    
    //send subs list message(s)
    
    int i = 0;
    for (const auto& tId : topicSet)
    {
    	sub_msg.msg.topicIds[i++] = tId;
        
        if (i >= PS_MAX_TOPIC_LIST)	//this message is full
        {
        	publish_system_message(&sub_msg, sizeof(sub_msg));
            i = 0;
        }
    }
    if (i)
    {
        //last message incomplete, more to send
    	sub_msg.msg.topicIds[i] = 0;
        publish_system_message(&sub_msg, sizeof(sub_msg));
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
	return max_ps_packet;
}

//register for system packets
ps_result_enum ps_pubsub_class::register_object(ps_packet_type_t packet_type, ps_root_class *rcl)
{
    PS_DEBUG("pub: register_object(%s, %s)", packet_type_names[packet_type], rcl->name.c_str());

    if (packet_type < PACKET_TYPES_COUNT)
    {
        registered_objects[packet_type] = rcl;
    }
    return PS_OK;
}

//clled by other plumbing objects to send a system packet
ps_result_enum ps_pubsub_class::publish_system_packet(ps_packet_type_t packet_type, const void *message, int length)
{
    ps_pubsub_header_t prefix;
    prefix.packet_type   = packet_type | 0x80;  //top bit must be 1
    prefix.packet_source = SOURCE;
    
    PS_DEBUG("pub: publish_system_packet(%s)", packet_type_names[packet_type]);
    
    if (length <= (int) max_ps_packet)
    {
        //queue for broker thread
        brokerQueue->copy_2message_parts_to_q( &prefix, sizeof(ps_pubsub_header_t), message, length);
        return PS_OK;
    }
    else
    {
        PS_ERROR("pub: %s packet too big", packet_type_names[packet_type]);
        return PS_LENGTH_ERROR;
    }
    
    return PS_OK;
}


//////////////////////////transport api

//suscribe to topic Id. Transport version.
ps_result_enum ps_pubsub_class::subscribe(ps_topic_id_t topicId, ps_packet_source_t tag)
{
    PS_DEBUG("pub: subscribe to %i from tag %i", topicId, tag);
    //find the client
    for (auto& client : clientList)
    {
        if (client.transport_tag == tag)
        {
            //insert the topic
            std::set<ps_topic_id_t> *topics = &client.topicList;
            topics->insert(topicId);	//add new subscription
            return PS_OK;
        }
    }
    //add new client
    {
        psClient_t newClient;
        newClient.transport_tag = tag;
        newClient.messageHandler = nullptr;
        std::set<ps_topic_id_t> topics = newClient.topicList;
        topics.insert(topicId);	//add new subscription
        
        clientList.push_back(newClient);
    }
    return PS_OK;
}
//remove transport from client list
ps_result_enum ps_pubsub_class::unsubscribe_all(ps_packet_source_t tag)
{
    //find the client
    for (auto pos = clientList.begin(); pos != clientList.end(); pos++)
    {
        if (pos->transport_tag == tag)
        {
            clientList.erase(pos);
            return PS_OK;
        }
    }
    return PS_OK;
}

//transport callbacks

void ps_pubsub_class::process_observed_data(ps_transport_class *pst, const void *message, int len)
{
    PS_DEBUG("pub: data from %s", pst->name.c_str());
    //incoming from Transport
    ps_pubsub_header_t& prefix = (ps_pubsub_header_t&)(message);
    prefix.packet_source = pst->tag;
    
    //queue it for the broker thread
    brokerQueue->copy_message_to_q(message, len);
}

void ps_pubsub_class::process_observed_event(ps_transport_class *pst, ps_transport_event_enum ev)
{
    PS_DEBUG("pub: status %s from %s", transport_event_names[ev], pst->name.c_str());
    
    ps_pubsub_header_t prefix;
    prefix.packet_source = pst->tag;
    
    switch(ev)
    {
        case PS_TRANSPORT_ONLINE:
            prefix.packet_type = TRANSPORT_ONLINE_PACKET;
            break;
        case PS_TRANSPORT_OFFLINE:
            prefix.packet_type = TRANSPORT_OFFLINE_PACKET;
            break;
        case PS_TRANSPORT_ADDED:
        {
            prefix.packet_type = TRANSPORT_ADDED_PACKET;
            ps_packet_source_t tag = the_broker().next_source_tag++;
            
            the_broker().transports.insert(std::make_pair(tag, pst));
            
            prefix.packet_source = tag;
            pst->tag = tag;
            pst->add_data_observer(this);
            pst->add_event_observer(this);
        }
            break;
        case PS_TRANSPORT_REMOVED:
        {
            prefix.packet_type = TRANSPORT_REMOVED_PACKET;
            
            //remove transports entry
            auto pos = the_broker().transports.find(pst->tag);
            if (pos != the_broker().transports.end())
            {
                the_broker().transports.erase(pos);
            }
        }
            break;
        default:
            return;
            break;
    }
    
    //queue it for the broker thread
    brokerQueue->copy_message_to_q(&prefix, sizeof(ps_pubsub_header_t));
}

////////////////////////////public api

//suscribe to topic Id. Receive a callback when a message is received
ps_result_enum ps_pubsub_class::subscribe(ps_topic_id_t topicId, message_handler_t *msgHandler)
{
    //find the client
    for (auto& client : clientList)
    {
        if (client.messageHandler == msgHandler)
        {
            std::set<ps_topic_id_t> *topics = &client.topicList;
            topics->insert(topicId);	//add new subscription
            
            PS_DEBUG("pub: subscribed to topic %i", topicId);
            return PS_OK;
        }
    }
    //add new client
    {
        psClient_t newClient;
        newClient.messageHandler 	= msgHandler;
        newClient.transport_tag 	= 0;
        std::set<ps_topic_id_t> topics = newClient.topicList;
        topics.insert(topicId);	//add new subscription
        
        clientList.push_back(newClient);
        
        PS_DEBUG("pub: new client subscribed to topic %i", topicId);
    }
    return PS_OK;
}

//publish a message to a topic id
ps_result_enum ps_pubsub_class::publish(ps_topic_id_t topicId, const void *message, int length)
{
    ps_pubsub_header_t prefix;
    prefix.packet_type   = topicId & 0x7f;  //top bit must be 0
    prefix.packet_source = SOURCE;
    
    PS_DEBUG("pub: publish to topic %i", topicId);
    
    if (length <= (int) max_ps_packet)
    {
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
