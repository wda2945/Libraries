//
//  ps_network.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/22/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_network_hpp
#define ps_network_hpp

#include "common/ps_root_class.hpp"
#include "transport/ps_transport_class.hpp"
#include <vector>
#include <map>

class ps_network : public ps_root_class {

public:
    
    //add to transports vector
    void add_transport_to_network(ps_transport_class *pst);
    
    //list the transports
    void iterate_transports(void (*callback)(ps_transport_class *, ps_transport_status_enum));

    //lookup transport
    ps_transport_class *get_transport_by_name(const char *name);
    ps_transport_status_enum get_transport_status(const char *name);
    
    //add to listeners vector
    void add_network_event_listener(void (*_listener)(ps_transport_class*, ps_transport_status_enum));
    
    //change of transport status
    void report_network_event(ps_transport_class *pst, ps_transport_status_enum status);

protected:
    ps_network();

	//listeners for network events
    typedef void (*event_listener_t)(ps_transport_class*, ps_transport_status_enum);

    std::vector<event_listener_t> listeners;

    //transports in the network
    std::vector<ps_transport_class*> transports;

    friend ps_network& the_network();
};

ps_network& the_network();

#endif /* ps_network_hpp */
