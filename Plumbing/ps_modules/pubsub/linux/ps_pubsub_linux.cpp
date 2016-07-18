//
//  ps_pubsub_linux.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_common.h"
#include "ps_pubsub_linux.hpp"
#include <set>

ps_pubsub_class& the_broker()
{
	static ps_pubsub_linux p;
	return p;
}

ps_pubsub_linux::ps_pubsub_linux() : ps_pubsub_class()
{
	//queues for publish and admin messages
	brokerQueue = new ps_queue_linux(max_ps_packet + sizeof(ps_pubsub_header_t), 100);

	//start thread for messages
    broker_thread = new thread([this](){broker_thread_method();});

	//iterate transports
	//to set data and event callbacks
	refresh_network();
}

ps_pubsub_linux::~ps_pubsub_linux()
{
    delete broker_thread;
}