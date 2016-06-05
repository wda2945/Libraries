//
//  ps_pubsub_rtos.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_pubsub_rtos.hpp"

//setup

//add a new transport object
ps_result_enum ps_pubsub_rtos::add_transport(ps_transport_class *transport)
{
    return PS_OK;
}

//close down a transport
ps_result_enum ps_pubsub_rtos::remove_transport(ps_transport_class *transport)
{
    return PS_OK;
}

//api

//suscribe to named topic. Receive a callback when a message is received
ps_result_enum ps_pubsub_rtos::subscribe(const char * topicName, void (*msgHandler)(void *, size_t))
{
    return PS_OK;
}

//publish a message to a named topic
ps_result_enum ps_pubsub_rtos::publish(const char * topicName, void *message, size_t length)
{
    return PS_OK;
}
