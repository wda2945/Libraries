//
//  ps_pubsub_linux.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_pubsub_linux_hpp
#define ps_pubsub_linux_hpp

#include "pubsub/ps_pubsub_class.hpp"
#include "network/ps_network.hpp"
#include "queue/linux/ps_queue_linux.hpp"

//PubSub Singleton
class ps_pubsub_linux : public ps_pubsub_class {
protected:

    ps_pubsub_linux();

public:

    friend ps_pubsub_class& the_broker();
    friend void *broker_thread_wrapper(void *arg);
};

#endif /* ps_pubsub_linux_hpp */
