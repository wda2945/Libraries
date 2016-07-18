//
//  ps_notify_linux.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/22/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_notify_linux_hpp
#define ps_notify_linux_hpp

#include <thread>

#include "notify/ps_notify.hpp"

class ps_notify_linux : public ps_notify_class {
public:
    ps_notify_linux();
    ~ps_notify_linux();
    
    std::thread *conditionsThread;
    
    void message_handler(ps_packet_source_t packet_source,
                                 ps_packet_type_t   packet_type,
                                    const void *msg, int length);
    
    void notify_conditions_thread();
    
};

#endif /* ps_notify_linux_hpp */
