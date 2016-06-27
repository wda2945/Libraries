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

void *broker_thread_wrapper(void *arg);

ps_pubsub_linux::ps_pubsub_linux()
{
	//queues for publish and admin messages
	brokerQueue = new ps_queue_linux(PS_DEFAULT_MAX_PACKET + sizeof(ps_pubsub_prefix_t), 100);

	//start thread for messages
	pthread_t thread;
	int s = pthread_create(&thread, NULL, broker_thread_wrapper, (void*) this);
	if (s != 0)
	{
		PS_ERROR("pubsub: Thread error %i\n", s);
	}

	//iterate transports
	//to set data and event callbacks
	refresh_network();
}

void *broker_thread_wrapper(void *arg)
{
	//non-member >> member function
	ps_pubsub_linux *ps = (ps_pubsub_linux*) arg;
	ps->broker_thread_method();
	//never returns
	return 0;
}
