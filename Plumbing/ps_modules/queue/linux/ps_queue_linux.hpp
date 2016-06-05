//
//  ps_queue_linux.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/21/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_queue_linux_hpp
#define ps_queue_linux_hpp

#include "queue/ps_queue_class.hpp"
#include <pthread.h>

class ps_queue_linux : public ps_queue_class {
    
    //queue sizes
    size_t sizeOffset        = 0;
    size_t nextPointerOffset = 0;
    size_t queueEntrySize    = 0;
    
    //queue mutex
    pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;	//access control
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;     //signals item in previously empty queue
    
    //queue base
    uint8_t *qHead = nullptr;
    uint8_t *qTail = nullptr;
    
    int queueCount = 0;
    
    //freelist
    uint8_t *freelist = nullptr;                            //common free list
    pthread_mutex_t	freeMtx = PTHREAD_MUTEX_INITIALIZER;	//freelist mutex
    
    void append_queue_entry(uint8_t *e);		//appends an allocated message q entry

    uint8_t *get_free_entry();				//new broker q entry <- freelist
    
    uint8_t *new_queue_entry();
    
    void add_to_freelist(uint8_t *e);

public:
    
    ps_queue_linux(size_t entrysize, size_t preload);
    
    //append to queue
    void copy_message_to_q(uint8_t *msg, size_t len);
    void copy_2message_parts_to_q(uint8_t *msg1, size_t len1, uint8_t *msg2, size_t len2);
    
    //get next message
    uint8_t *get_next_message(int msecs, size_t *length);	//waits msecs if empty, returns pointer
    
    void done_with_message(uint8_t *msg);	//when done with message Q entry -> freelist
    
    //queue info
    bool empty();
    size_t count();
};

#endif /* ps_queue_linux_hpp */
