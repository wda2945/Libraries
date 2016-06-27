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

void broker_transport_data_callback(ps_transport_class *pst, void *message, int len);
void broker_transport_status_callback(ps_transport_class *pst, ps_transport_status_enum pstStatus);

/////////////////////////// C API -> METHOD

//suscribe to topic. Receive a callback when a message is received
ps_result_enum ps_subscribe(ps_topic_id_t topic_id, message_handler_t *msgHandler)
{
	return the_broker().subscribe(topic_id, msgHandler);
}

//publish a message to a topic
ps_result_enum ps_publish(ps_topic_id_t topic_id, void *message, int length)
{
	return the_broker().publish(topic_id, message, length);
}

/////////////////////////// PLUMBING SERVICES
//register for system packets
ps_result_enum ps_pubsub_class::register_object(ps_packet_type_t packet_type, ps_root_class *rcl)
{
	if (packet_type < PACKET_TYPES_COUNT)
	{
		registered_objects[packet_type] = rcl;
	}
	return PS_OK;
}

//send a system packet
ps_result_enum ps_pubsub_class::publish_packet(ps_packet_type_t packet_type, void *message, int length)
{
	if (packet_type < PACKET_TYPES_COUNT)
	{
		ps_root_class *rcl = registered_objects[packet_type];
		if (rcl) rcl->message_handler(message, length);
	}
	return PS_OK;
}

////////////////////////////// INTERNAL BROKER METHODS

void ps_pubsub_class::broker_thread_method()
{
	request_subscriptions(0);	//at startup

	while(1)
	{
		int len;
		//next message from queue
		void *q_message = brokerQueue->get_next_message(0, &len);

		//access the pubsub prefix
		ps_pubsub_prefix_t& prefix = (ps_pubsub_prefix_t&)(q_message);

		switch(prefix.packet_type)
		{
		case SEND_SUBS_PACKET:
			//it's a call for subscribed topics
			send_subscriptions(prefix.packet_source);
			break;
		case SUBSCRIBE_PACKET:
		{
			ps_subscribe_message_t *msg = (ps_subscribe_message_t*) ((uint8_t*) q_message + sizeof(ps_pubsub_prefix_t));
			int i = 0;
			//process subscribe
			while (msg->topicIds[i] && i < PS_MAX_TOPIC_LIST) {
				//subscribe to each topic listed
				subscribe(msg->topicIds[i], prefix.packet_source);
				i++;
			};
		}
			break;
		case TRANSPORT_ADDED_PACKET:
			request_subscriptions(prefix.packet_source);
			send_subscriptions(prefix.packet_source);
			break;
		case TRANSPORT_REMOVED_PACKET:
		case TRANSPORT_ONLINE_PACKET:
		case TRANSPORT_OFFLINE_PACKET:
		case PUBLISH_PACKET:
		default:
			//actual message to be published
			void *message = (void*) ((uint8_t*) q_message + sizeof(ps_pubsub_prefix_t));
			int length = len - sizeof(ps_pubsub_prefix_t);

			//pass it on
			publish_message(&prefix, message, length);
			break;
		}
	}
}

//send a message to all subscribers
void ps_pubsub_class::publish_message(ps_pubsub_prefix_t *prefix, void *message, int length)
{
	switch(prefix->packet_type)
	{
	case PUBLISH_PACKET:
		//iterate client list
		for (const auto& client : clientList)
		{
			//find the subscribed topic
			auto tId = client.topicList.find(prefix->topic_id);
			if (tId != client.topicList.end())
			{
				//this client is subscribed
				if (client.messageHandler != nullptr)
				{
					//pointer to messageHandler C function
					(*client.messageHandler)(message, length);
				}
				else if (client.transport_tag != prefix->packet_source)
				{
					//pointer to ps_transport subclass
					auto pos = transports.find(client.transport_tag);
					if (pos != transports.end())
					{
						pos->second->send_packet2(prefix, sizeof(ps_pubsub_prefix_t), message, length);
					}
				}
				break;
			}
		}
		break;
	case SUBSCRIBE_PACKET:
	case SEND_SUBS_PACKET:
		//only looking for transports
		for (auto pos = transports.begin(); pos != transports.end(); pos++)
		{
			if ((pos->first == prefix->packet_source) || (prefix->packet_source == 0))
			{
				pos->second->send_packet2(prefix, sizeof(ps_pubsub_prefix_t), message, length);
			}
		}
		break;
	default:
		//plumbing internal message
		publish_packet(prefix->packet_type, message, length);
		break;
	}
}

//request all subscriptions from remote broker
void ps_pubsub_class::request_subscriptions(ps_packet_source_t tag)
{
	ps_pubsub_prefix_t prefix;
	prefix.packet_type = SEND_SUBS_PACKET;
	prefix.packet_source = tag;

	publish_message(&prefix, nullptr, 0);
}

//send all subscriptions to remote broker
void ps_pubsub_class::send_subscriptions(ps_packet_source_t tag)
{
	ps_subscribe_message_t msg;

	ps_pubsub_prefix_t prefix;
	prefix.packet_type = SUBSCRIBE_PACKET;
	prefix.packet_source = tag;

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
		msg.topicIds[i++] = tId;

		if (i >= PS_MAX_TOPIC_LIST)	//this message is full
		{
			publish_message(&prefix, (uint8_t *) &msg, sizeof(ps_subscribe_message_t));
			i = 0;
		}
	}
	if (i)
	{
		//last message incomplete, more to send
		msg.topicIds[i] = 0;
		publish_message(&prefix, (uint8_t *) &msg, sizeof(ps_subscribe_message_t));
	}
}


void ps_pubsub_class::refresh_network()
{
	the_network().add_network_event_listener(&broker_transport_status_callback);
	the_network().iterate_transports(&broker_transport_status_callback);
}

//////////////////////////transport api

//suscribe to topic Id. Transport version.
ps_result_enum ps_pubsub_class::subscribe(const ps_topic_id_t topicId, const ps_packet_source_t tag)
{
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
ps_result_enum ps_pubsub_class::unsubscribe_all(const ps_packet_source_t tag)
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

void broker_transport_data_callback(ps_transport_class *pst, void *message, int len)
{
	//incoming from Transport
	ps_pubsub_prefix_t& prefix = (ps_pubsub_prefix_t&)(message);
	prefix.packet_source = pst->tag;

	//queue it for the broker thread
	the_broker().brokerQueue->copy_message_to_q(message, len);
}

void broker_transport_status_callback(ps_transport_class *pst, ps_transport_status_enum pstStatus)
{
	ps_pubsub_prefix_t prefix;
	prefix.packet_source = pst->tag;

	switch(pstStatus)
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
		pst->set_data_callback(broker_transport_data_callback);
		pst->set_status_callback(broker_transport_status_callback);
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
	the_broker().brokerQueue->copy_message_to_q(&prefix, sizeof(ps_pubsub_prefix_t));
}

////////////////////////////public api

//suscribe to topic Id. Receive a callback when a message is received
ps_result_enum ps_pubsub_class::subscribe(const ps_topic_id_t topicId, message_handler_t *msgHandler)
{
	//find the client
	for (auto& client : clientList)
	{
		if (client.messageHandler == msgHandler)
		{
			std::set<ps_topic_id_t> *topics = &client.topicList;
			topics->insert(topicId);	//add new subscription
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
	}
	return PS_OK;
}

//publish a message to a topic id
ps_result_enum ps_pubsub_class::publish(const ps_topic_id_t topicId, void *message, int length)
{
	ps_pubsub_prefix_t prefix;
	prefix.packet_type   = PUBLISH_PACKET;
	prefix.topic_id 	 = topicId;
	prefix.packet_source = 0;

	if (length <= PS_DEFAULT_MAX_PACKET)
	{
		//queue for broker thread
		brokerQueue->copy_2message_parts_to_q( &prefix, sizeof(ps_pubsub_prefix_t), message, length);
		return PS_OK;
	}
	else
	{
		PS_ERROR("pubsub: packet to topic %i too big", topicId);
		return PS_LENGTH_ERROR;
	}
}


