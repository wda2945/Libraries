//
//  ps_pubsub_rtos.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_pubsub_rtos_hpp
#define ps_pubsub_rtos_hpp

#include "ps_types.h"
#include "pubsub/ps_pubsub_class.hpp"
#include "transport/ps_transport_class.hpp"

//PubSub Singleton
class ps_pubsub_rtos : public ps_root_class {
    
public:
   
//setup
    
    //add a new transport object
    ps_result_enum add_transport(ps_transport_class *transport);
    
    //close down a transport
    ps_result_enum remove_transport(ps_transport_class *transport);

//api
    
    //suscribe to named topic. Receive a callback when a message is received
    ps_result_enum subscribe(const char * topicName, void (*msgHandler)(void *, size_t));
    
    //publish a message to a named topic
    ps_result_enum publish(const char * topicName, void *message, size_t length);
    
};

#endif /* ps_pubsub_rtos_hpp */
