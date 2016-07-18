//
//  ps_notify.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/22/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//


#include <string.h>
#include "notify/ps_notify.hpp"
#include "pubsub/ps_pubsub_class.hpp"

ps_notify_class::ps_notify_class(){
    the_broker().register_object(CONDITIONS_PACKET, this);
    the_broker().register_object(EVENT_PACKET, this);
}
ps_notify_class::~ps_notify_class(){
    
}


/////////////////////// C API Methods

////////////////////////// EVENTS

//Raise an event
ps_result_enum ps_notify_class::ps_notify_event_method(ps_event_id_t event)
{
    the_broker().publish_system_packet(EVENT_PACKET, &event, (int) sizeof(ps_event_id_t));
    
    //notify observers
    notify_event_observer(event);
    
    return PS_OK;
}

ps_result_enum ps_notify_class::ps_add_event_observer_method(ps_event_id_t event,
                                                                 ps_event_observer_callback_t *callback, void *arg)
{
    ps_event_observer obs {callback, arg};
    
    auto pos = event_observers.find(event);
    
    if (pos == event_observers.end() )
    {
        //first observer
        vector<ps_event_observer> new_vec;
        new_vec.push_back(obs);
        event_observers.insert(make_pair(event, new_vec));
    }
    else
    {
        pos->second.push_back(obs);
    }
    
    return PS_OK;
}

////////////////////////// CONDITIONS


//Set/cancel a condition
ps_result_enum ps_notify_class::ps_set_condition_method(ps_condition_id_t condition)
{
    conditions[SOURCE].set(condition);
    
    //notify observers
    notify_condition_observer(SOURCE, condition, true);
    
    return PS_OK;
}

ps_result_enum ps_notify_class::ps_cancel_condition_method(ps_condition_id_t condition)
{
    conditions[SOURCE].reset(condition);

    //notify observers
    notify_condition_observer(SOURCE, condition, false);
    
    return PS_OK;
}

bool ps_notify_class::ps_test_condition_method(Source_t src, ps_condition_id_t condition)
{
    return conditions[src].test(condition);
}


ps_result_enum ps_notify_class::ps_add_condition_observer_method(Source_t src, ps_condition_id_t condition,
                                                                 ps_condition_observer_callback_t *callback, void *arg)
{
    ps_condition_observer obs {callback, arg};
    
    auto pos = condition_observers[src].find(condition);
    
    if (pos == condition_observers[src].end() )
    {
        //first observer
        vector<ps_condition_observer> new_vec;
        new_vec.push_back(obs);
        condition_observers[src].insert(make_pair(condition, new_vec));
    }
    else
    {
        pos->second.push_back(obs);
    }
    
    return PS_OK;
}

//helpers

void ps_notify_class::notify_event_observer(ps_event_id_t event)
{
    auto pos = event_observers.find(event);
    
    if (pos != event_observers.end() )
    {
        for (auto obs : pos->second)
        {
            (obs.callback)(obs.callbackArg, event);
        }
    }
}

void ps_notify_class::notify_condition_observer(Source_t src, ps_condition_id_t condition, bool state)
{
    auto pos = condition_observers[src].find(condition);
    
    if (pos != condition_observers[src].end() )
    {
        for (auto obs : pos->second)
        {
            (obs.callback)(obs.callbackArg, src, condition, state);
        }
    }
}

void ps_notify_class::process_conditions_packet(ps_packet_source_t src, ps_conditions_message_t msg)
{
    bitset<PS_CONDITIONS_COUNT> msgbits {msg};
    
    bitset<PS_CONDITIONS_COUNT> changes = conditions[src] ^ msgbits;
    
    for (int i=0; i < PS_CONDITIONS_COUNT; i++)
    {
        if (changes[i])
        {
            notify_condition_observer( (Source_t) src, i, msgbits[i]);
        }
    }
    
    conditions[src] = msgbits;
}
