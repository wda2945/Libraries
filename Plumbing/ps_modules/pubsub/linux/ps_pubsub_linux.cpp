//
//  ps_pubsub_linux.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_pubsub_linux.hpp"
#include "ps_types.h"
#include "ps_config.h"
#include "network/ps_network.hpp"
#include <set>

ps_pubsub_class& the_broker()
{
	static ps_pubsub_linux p;
	return p;
}

void ps_pubsub_linux::send_message(ps_pubsub_prefix_t prefix, uint8_t *message, size_t length)
{
	//publish
	//iterate callout list
	for (const auto& client : clientList)
	{
		for (const auto& tId : client.topicList)
		{
			if (prefix.topicId == tId)
			{
				if (client.messageHandler)
						(*client.messageHandler)(message, length);
				if (client.transport && client.transport->tag != prefix.source)
						client.transport->send_packet(message, length);
				break;
			}
		}
	}
}
void ps_pubsub_linux::broker_thread_method()
{
	ps_subscribe_message_t msg;
	msg.msgId = PS_SUBSCRIBE_MESSAGE_ID;

	//TODO: publish start-up subscription message

	while(1)
	{
		//next message
		size_t q_length;
		uint8_t *q_message = brokerQueue->get_next_message(0, &q_length);

		uint8_t *message = (q_message + PS_MESSAGE_OFFSET);	//actual message to be sent
		ssize_t length = (q_length - PS_MESSAGE_OFFSET);

		//process subscribes, etc
		ps_pubsub_prefix_t& prefix = (ps_pubsub_prefix_t&)(message);
		ps_topic_Id_t topicId = prefix.topicId;

		if (topicId == subscriptionTopic)
		{
			//call for subscribed topics
			ps_subscribe_message_t msg;
			msg.msgId == PS_SUBSCRIBE_MESSAGE_ID;

			//publish topic list for this node

			std::set<ps_topic_Id_t> topicSet;

			//iterate callout list, collect topics
			for (const auto& client : clientList)
			{
				for (const auto& tId : client.topicList)
				{
					topicSet.insert(tId);		//NB no duplicates allowed in a set
				}
			}

			//send subscriber messages
			ps_pubsub_prefix_t prefix;
			prefix.topicId = subscriptionTopic;
			int i = 0;
			for (const auto& tId : topicSet)
			{
				msg.topicIds[i++] = tId;

				if (i >= PS_MAX_TOPIC_LIST)	//this message is full
				{
					send_message(prefix, (uint8_t *) &msg, sizeof(ps_subscribe_message_t));
					i = 0;
				}
			}
			if (i)
			{
				//last message incomplete, more to send
				msg.topicIds[i] = 0;
				send_message(prefix, (uint8_t *) &msg, sizeof(ps_subscribe_message_t));
			}
		}
	}
}

void *broker_thread_wrapper(void *arg)
{
	//non-member >> member function
	ps_pubsub_linux *ps = (ps_pubsub_linux*) arg;
	ps->broker_thread_method();
	//never returns
	return 0;
}

void broker_data_callback(ps_transport_class *pst, void *message, size_t len)
{
	//incoming from Transport
	//first process subscribes,
	ps_pubsub_prefix_t& prefix = (ps_pubsub_prefix_t&)(message);
	ps_topic_Id_t topicId = prefix.topicId;

	if (topicId == subscribeTopic)
	{
		//verify message type
		ps_subscribe_message_t *msg = (ps_subscribe_message_t *)((uint8_t*)message + PS_MESSAGE_OFFSET);
		if (msg->msgId == PS_SUBSCRIBE_MESSAGE_ID)
		{
			int i = 0;
			//process subscribe
			while (msg->topicIds[i] && i < PS_MAX_TOPIC_LIST) {
				//subscribe to each topic listed
				the_broker().subscribe(msg->topicIds[i], pst);
				i++;
			};
		}
	}
	else
	{
		//queue it for the broker thread
		prefix.source = pst->tag;
		the_broker().copy_message_to_q( (uint8_t*) message, len);
	}
}

void ps_broker_status_callback(ps_transport_class *pst, ps_transport_status_enum pstStatus)
{
	switch(pstStatus)
	{
	case PS_TRANSPORT_ONLINE:
		//TODO
		break;
	case PS_TRANSPORT_OFFLINE:
		//TODO
		break;
	case PS_TRANSPORT_ADDED:
		the_broker().transports.push_back(pst);

		pst->set_data_callback(&broker_data_callback);
		pst->set_status_callback(&ps_broker_status_callback);

		the_broker().subscribe(PS_SUBSCRIBE_TOPIC, pst);		//subscribe to subscribe updates
		the_broker().subscribe(PS_SUBSCRIPTION_TOPIC, pst);	//subscribe to subscription requests
		break;
	default:
		break;
	}
}

ps_pubsub_linux::ps_pubsub_linux()
{
	subscribeTopic 		= ps_topic_hash(PS_SUBSCRIBE_TOPIC);
	subscriptionTopic 	= ps_topic_hash(PS_SUBSCRIPTION_TOPIC);

	//queues for publish and admin messages
	brokerQueue = new ps_queue_linux(PS_DEFAULT_MAX_PACKET + sizeof(ps_pubsub_prefix_t), 100);

	//start thread for messages
    pthread_t thread;
    int s = pthread_create(&thread, NULL, broker_thread_wrapper, (void*) this);
    if (s != 0)
    {
        //        ERRORPRINT("responder: Thread error %i\n", s);
    }

	//iterate transports
	//set data and event callbacks
    the_network().add_network_event_listener(&ps_broker_status_callback);
    the_network().iterate_transports(&ps_broker_status_callback);

}

//suscribe to named topic. Receive a callback when a message is received
ps_result_enum ps_pubsub_linux::subscribe(const char * topicName, void (*msgHandler)(void *, size_t))
{
	ps_topic_Id_t topicId = ps_topic_hash(topicName);
	subscribe(topicId, msgHandler);
}
//suscribe to topic Id. Receive a callback when a message is received
ps_result_enum ps_pubsub_linux::subscribe(ps_topic_Id_t topicId, void (*msgHandler)(void *, size_t))
{
	//find the client
	for (const auto& client : clientList)
	{
		if (client.messageHandler == msgHandler)
		{
			//find the topic
			std::vector<ps_topic_Id_t> topics = client.topicList;
			for (const auto& tId : topics)
			{
				if (topicId == tId) return PS_OK;	//already subscribed
			}
			topics.push_back(static_cast<ps_topic_Id_t>(topicId));	//add new subscription
			return PS_OK;
		}
	}
	//add new client
	{
		psClient_t newClient;
		newClient.messageHandler = msgHandler;
		newClient.transport = nullptr;
		newClient.topicList.push_back(topicId);

		clientList.push_back(newClient);
	}
    return PS_OK;
}
//suscribe to topic Id. Transport version.
ps_result_enum ps_pubsub_linux::subscribe(ps_topic_Id_t topicId, ps_transport_class *pst)
{
	//find the client
	for (const auto& client : clientList)
	{
		if (client.transport == pst)
		{
			//find the topic
			std::vector<ps_topic_Id_t> topics = client.topicList;
			for (const auto& tId : topics)
			{
				if (topicId == tId) return PS_OK;	//already subscribed
			}
			topics.push_back(static_cast<ps_topic_Id_t>(topicId));	//add new subscription
			return PS_OK;
		}
	}
	//add new client
	{
		psClient_t newClient;
		newClient.transport = pst;
		newClient.messageHandler = nullptr;
		newClient.topicList.push_back(topicId);

		clientList.push_back(newClient);
	}
    return PS_OK;
}
//suscribe to named topic. Transport version.
ps_result_enum ps_pubsub_linux::subscribe(const char * topicName, ps_transport_class *pst)
{
	ps_topic_Id_t topicId = ps_topic_hash(topicName);
	subscribe(topicId, pst);
    return PS_OK;
}

//publish a message to a named topic
ps_result_enum ps_pubsub_linux::publish(const char * topicName, void *message, size_t _length)
{
	ps_pubsub_prefix_t prefix;
	prefix.topicId = ps_topic_hash(topicName);

    size_t length = (_length > PS_DEFAULT_MAX_PACKET ? PS_DEFAULT_MAX_PACKET : _length);

    //queue for broker thread
    brokerQueue->copy_2message_parts_to_q((uint8_t*) &prefix, sizeof(ps_pubsub_prefix_t),
                                    (uint8_t*) message, length);

    return PS_OK;
}

void ps_pubsub_linux::copy_message_to_q(uint8_t* message, size_t len)
{
	brokerQueue->copy_message_to_q(message, len);
}

//c api

//suscribe to names topic. Receive a callback when a message is received
ps_result_enum ps_subscribe(const char * topicName, void (*msgHandler)(void *, size_t))
{
	return the_broker().subscribe(topicName, msgHandler);
}

//publish a message to a named topic
ps_result_enum ps_publish(const char * topicName, void *message, size_t length)
{
	return the_broker().publish(topicName, message, length);
}

ps_topic_Id_t subscribeTopic;		//publish subscription list
ps_topic_Id_t subscriptionTopic;	//topic to request subscrition list
