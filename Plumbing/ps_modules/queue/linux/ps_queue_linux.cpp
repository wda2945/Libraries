//
//  ps_queue_linux.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/21/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_common.h"
#include "ps_queue_linux.hpp"
#include <string.h>
#include <sys/time.h>

ps_queue_linux::ps_queue_linux(int entrysize, int preload){

    nextPointerOffset = entrysize;
    queueEntrySize = nextPointerOffset + sizeof(uint8_t *);

    int i;
    for (i=0; i<preload; i++)
    {
        void *entry = new_queue_entry();
        if (entry == nullptr) return;
        add_to_freelist(entry);
    }
}

void ps_queue_linux::append_queue_entry(void *e)		//appends an allocated message q entry
{
	void **next = (void **)((uint8_t*) e + nextPointerOffset);
    
    *next = nullptr;
    
    //critical section
    pthread_mutex_lock(&mtx);
    
    int wake = 0;
    if (qHead == nullptr)
    {
        //Q empty
        qHead = qTail = e;
        wake = 1;
    }
    else
    {
    	void **tailnext = (void **)((uint8_t*)qTail + nextPointerOffset);
        *tailnext = e;
        qTail = e;
    }
    queueCount++;
    
    pthread_mutex_unlock(&mtx);
    //end critical section
    
    if (wake) pthread_cond_signal(&cond);
}

//appends to queue
void ps_queue_linux::copy_3message_parts_to_q(void *msg1, int len1, void *msg2, int len2, void *msg3, int len3)
{
	if (len1 + len2 + len3 < nextPointerOffset)
	{
		void *e = get_free_entry();

		if (e != nullptr)
		{
			ps_q_size_t *size = (ps_q_size_t*)((uint8_t*) e + sizeOffset);
			*size = (ps_q_size_t) (len1 + len2 + len3);

			if (len1) memcpy(e, msg1, len1);
			if (len2) memcpy(((uint8_t*) e + len1), msg2, len2);
			if (len3) memcpy(((uint8_t*) e + len1 + len2), msg3, len3);

			append_queue_entry( e);
		}
	}
	else
	{
		PS_ERROR("queue: message too long");
	}
}
void ps_queue_linux::copy_2message_parts_to_q(void *msg1, int len1, void *msg2, int len2)
{
	copy_3message_parts_to_q(msg1, len1, msg2, len2, nullptr, 0);
}
void ps_queue_linux::copy_message_to_q(void *msg, int len)
{
    copy_3message_parts_to_q(msg, len, nullptr, 0, nullptr, 0);
}

//waits if empty, returns pointer (call done_with_message!)
void *ps_queue_linux::get_next_message(int msecs, int *length)
{
	void *e;
    
    struct timespec abstime;
    struct timeval timenow;
    gettimeofday(&timenow, NULL);
    abstime.tv_sec = timenow.tv_sec;
    abstime.tv_nsec = (timenow.tv_usec * 1000) + (msecs * 1000000);
    
    //critical section
    pthread_mutex_lock(&mtx);
    
    int result = 0;
    //empty wait case
    while (queueCount == 0 && result == 0) {
        if (msecs)
        {
            result = pthread_cond_timedwait(&cond, &mtx, &abstime);
        }
        else
        {
            result = pthread_cond_wait(&cond, &mtx);
        }
    }
    
    e = qHead;
    
    if (e)
    {
    	void **next = (void **)((uint8_t*) e + nextPointerOffset);

        qHead = *next;
        *next = nullptr;
        if (qHead == nullptr)
        {
            //end of queue
            qTail = nullptr;
        }
        
        queueCount--;
        
        if (length != nullptr)
        {
            uint16_t *size = (uint16_t*)((uint8_t*) e + sizeOffset);
            *length = *size;
        }
    }
    pthread_mutex_unlock(&mtx);
    //end critical section
    
    return e;
}

bool ps_queue_linux::empty()
{
    return ((queueCount == 0));
}
int ps_queue_linux::count()
{
    return queueCount;
}
void ps_queue_linux::done_with_message(void *msg)
{
    add_to_freelist(msg);
}

void *ps_queue_linux::get_free_entry()				//new broker q entry <- freelist
{
	void *entry;
    
    //critical section
    pthread_mutex_lock(&freeMtx);
    
    if (freelist == nullptr)
    {
        entry = new_queue_entry();
    }
    else
    {
        entry = freelist;
        void **next = (void **)((uint8_t*) entry + nextPointerOffset);
        freelist = *next;
    }
    
    pthread_mutex_unlock(&freeMtx);
    //end critical section
    
    return entry;
}

void *ps_queue_linux::new_queue_entry()
{
    return (uint8_t *) calloc(1, queueEntrySize);
}

void ps_queue_linux::add_to_freelist(void *e)
{
    //critical section
    pthread_mutex_lock(&freeMtx);

    void **next = (void **)((uint8_t*)e + nextPointerOffset);
    *next = freelist;
    freelist = e;
    
    pthread_mutex_unlock(&freeMtx);
    //end critical section
}
