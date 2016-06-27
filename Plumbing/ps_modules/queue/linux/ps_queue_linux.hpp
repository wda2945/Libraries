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

typedef uint16_t ps_q_size_t;

class ps_queue_linux : public ps_queue_class {
    
    //queue entry total size
    int queueEntrySize    = 0;
    //overhead offsets (at end)
    int sizeOffset        = 0;
    int nextPointerOffset = 0;
    
    //queue mutex
    pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;	//access control
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;     //signals item in previously empty queue
    
    //queue base
    void *qHead;
    void *qTail;
    
    int queueCount = 0;
    
    //freelist
    void *freelist = nullptr;                            //common free list
    pthread_mutex_t	freeMtx = PTHREAD_MUTEX_INITIALIZER;	//freelist mutex
    
    void append_queue_entry(void *e);		//appends an allocated message q entry

    void *get_free_entry();				//new broker q entry <- freelist
    
    void *new_queue_entry();
    
    void add_to_freelist(void *e);

public:
    
    ps_queue_linux(int entrysize, int preload);
    
    //append to queue
    void copy_message_to_q(void *msg, int len);
    void copy_2message_parts_to_q(void *msg1, int len1, void *msg2, int len2);
    void copy_3message_parts_to_q(void *msg1, int len1, void *msg2, int len2, void *msg3, int len3);
    
    //get next message
    void *get_next_message(int msecs, int *length);	//waits msecs if empty, returns pointer
    
    void done_with_message(void *msg);	//when done with message Q entry -> freelist
    
    //queue info
    bool empty();
    int count();
};

#endif /* ps_queue_linux_hpp */
