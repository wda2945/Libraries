//
//  ps_notify.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/22/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include <unistd.h>
#include <vector>
#include <map>
#include <mutex>
#include "notify/linux/ps_notify_linux.hpp"
#include "pubsub/ps_pubsub_class.hpp"

mutex eventMtx;
mutex conditionMtx;

ps_notify_class& the_notifier()
{
    static ps_notify_linux the_notifier;
    return the_notifier;
}

ps_notify_linux::ps_notify_linux()
{
    conditionsThread  = new thread([this](){notify_conditions_thread();});
}

ps_notify_linux::~ps_notify_linux()
{
    
}

void ps_notify_linux::notify_conditions_thread()
{
    bitset<PS_CONDITIONS_COUNT> current {conditions[SOURCE]};
    
    while(1)
    {
        if (current != conditions[SOURCE])
        {
            unique_lock<mutex> lck {conditionMtx};
            
            ps_conditions_message_t msg = conditions[SOURCE].to_ullong();
            
            the_broker().publish_system_packet(CONDITIONS_PACKET, &msg, (int) sizeof(ps_conditions_message_t));
            
            current = conditions[SOURCE];
        }
        
        usleep(250000);
    }
}

void ps_notify_linux::message_handler(ps_packet_source_t packet_source,
                     ps_packet_type_t   packet_type,
                     const void *msg, int length)
{
    if (packet_source == SOURCE) return;
    
    switch(packet_type)
    {
        case EVENT_PACKET:
            if (length >= (int) sizeof(ps_event_id_t))
        {
            unique_lock<mutex> lck {eventMtx};
            
            const ps_event_id_t *event = (ps_event_id_t*) msg;
            
            notify_event_observer(*event);
        }
            break;
        case CONDITIONS_PACKET:
            if (length >= (int) sizeof(ps_conditions_message_t))
        {
            unique_lock<mutex> lck {conditionMtx};
            
            ps_conditions_message_t *c_msg = (ps_conditions_message_t*) msg;
            
            process_conditions_packet(packet_source, *c_msg);
        }
            break;
        default:
            break;
    }
}



/////////////////////// C API

//Raise an event
ps_result_enum ps_notify_event(ps_event_id_t event)
{
    unique_lock<mutex> lck {eventMtx};
    
    return the_notifier().ps_notify_event_method(event);
}

ps_result_enum ps_add_event_observer(ps_event_id_t event, ps_event_observer_callback_t *callback, void *arg)
{
    unique_lock<mutex> lck {eventMtx};
    
    return the_notifier().ps_add_event_observer_method(event, callback, arg);
}


//Set/cancel a condition
ps_result_enum ps_set_condition(ps_condition_id_t condition)
{
    unique_lock<mutex> lck {conditionMtx};
    
    return the_notifier().ps_set_condition_method(condition);
}
ps_result_enum ps_cancel_condition(ps_condition_id_t condition)
{
    unique_lock<mutex> lck {conditionMtx};
    
    return the_notifier().ps_cancel_condition_method(condition);
}

bool ps_test_condition(Source_t src, ps_condition_id_t condition)
{
    unique_lock<mutex> lck {conditionMtx};
    
    return the_notifier().ps_test_condition_method(src, condition);
}

ps_result_enum ps_add_condition_observer(Source_t src, ps_condition_id_t condition, ps_condition_observer_callback_t *callback, void *arg)
{
    unique_lock<mutex> lck {conditionMtx};
    
    return the_notifier().ps_add_condition_observer_method(src, condition, callback, arg);
}
