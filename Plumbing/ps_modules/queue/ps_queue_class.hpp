//
//  ps_queue.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/21/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_queue_hpp
#define ps_queue_hpp

#include "ps_types.h"

class ps_queue_class {

public:

    //append a message to the queue
    virtual void copy_message_to_q(uint8_t *msg, size_t len) = 0;
    virtual void copy_2message_parts_to_q(uint8_t *msg1, size_t len1, uint8_t *msg2, size_t len2) = 0;
    
    //get next message
    //waits if empty, returns pointer (call done_with_message!)
    virtual uint8_t *get_next_message(int msecs, size_t *length) = 0;
    
    virtual void done_with_message(uint8_t *msg) = 0;	//when done with message Q entry -> freelist
    
    //queue info
    virtual bool empty() = 0;
    
    virtual size_t count() = 0;
};

#endif /* ps_queue_hpp */
