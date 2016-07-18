//
//  ps_network.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/22/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_network.hpp"
#include <string.h>

ps_network& the_network()
{
	static ps_network network;
	return network;
}

ps_network::ps_network(){}

void ps_network::process_observed_event(ps_transport_class *pst, ps_transport_event_enum ev)
{
    notify_new_event(pst, ev);
}

void ps_network::add_transport_to_network(ps_transport_class *pst)
{
	transports.push_back(pst);

    pst->add_event_observer(this);
    notify_new_event(pst, PS_TRANSPORT_ADDED);
}

void ps_network::iterate_transports(ps_root_class *cb)
{
    for (const auto pst : transports)
    {
        cb->process_observed_event(pst, PS_TRANSPORT_ADDED);
    }
}

ps_transport_class *ps_network::get_transport_by_name(const char *_name)
{
    for (const auto& pst : transports)
    {
        if (pst->name.compare(_name) == 0) return pst;
    }
    return nullptr;
}


