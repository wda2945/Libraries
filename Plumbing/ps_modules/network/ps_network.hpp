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

typedef void (network_observer_t)(ps_transport_class*, ps_transport_event_enum);

class ps_network : public ps_root_class {

public:
    
    //add to transports vector
    void add_transport_to_network(ps_transport_class *pst);
    
    //list the transports
    void iterate_transports(ps_root_class *cb);

    //lookup transport
    ps_transport_class *get_transport_by_name(const char *name);
    
    //callback method for transport -> network events
    void process_observed_event(ps_transport_class *pst, ps_transport_event_enum ev);

protected:
    ps_network();

    //transports in the network
    std::vector<ps_transport_class*> transports;

    friend ps_network& the_network();
};

ps_network& the_network();

#endif /* ps_network_hpp */
