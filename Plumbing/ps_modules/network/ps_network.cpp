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

void network_transport_status_callback(ps_transport_class *pst, ps_transport_status_enum status)
{
    the_network().report_network_event(pst, status);
}

void ps_network::add_transport_to_network(ps_transport_class *pst)
{
	transports.push_back(pst);

    pst->set_status_callback(&network_transport_status_callback);
    report_network_event(pst, PS_TRANSPORT_ADDED);
}

void ps_network::iterate_transports(void (*callback)(ps_transport_class *, ps_transport_status_enum))
{
    for (const auto pst : transports)
    {
        (*callback)(pst, PS_TRANSPORT_ADDED);
    }
}

ps_transport_class *ps_network::get_transport_by_name(char *_name)
{
    for (const auto& pst : transports)
    {
        if (strcmp(pst->name, _name) == 0) return pst;
    }
    return nullptr;
}

ps_transport_status_enum ps_network::get_transport_status(char *_name)
{
    ps_transport_class *pst = get_transport_by_name(_name);
    if (pst != nullptr)
    {
        return pst->transport_status;
    }
    else
    {
        return PS_TRANSPORT_UNKNOWN;
    }
}

void ps_network::add_network_event_listener(void (*_listener)(ps_transport_class*, ps_transport_status_enum))
{
	listeners.push_back(_listener);
}

void ps_network::report_network_event(ps_transport_class *pst, ps_transport_status_enum status)
{
    for (const auto event_listener : listeners)
    {
        (event_listener)(pst,status);
    }
}

ps_network network;
