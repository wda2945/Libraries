//
//  ps_queue_linux.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/21/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_queue_linux.hpp"
#include <string.h>
#include <sys/time.h>

	ps_queue_linux::ps_queue_linux(size_t entrysize, size_t preload){

    nextPointerOffset = entrysize;
    queueEntrySize = nextPointerOffset + sizeof(uint8_t *);
    
    int i;
    for (i=0; i<preload; i++)
    {
        uint8_t *entry = new_queue_entry();
        if (entry == nullptr) return;
        add_to_freelist(entry);
    }
}

void ps_queue_linux::append_queue_entry(uint8_t *e)		//appends an allocated message q entry
{
    uint8_t **next = (uint8_t **)(e + nextPointerOffset);
    
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
        uint8_t **tailnext = (uint8_t **)(qTail + nextPointerOffset);
        *tailnext = e;
        qTail = e;
    }
    queueCount++;
    
    pthread_mutex_unlock(&mtx);
    //end critical section
    
    if (wake) pthread_cond_signal(&cond);
}

//appends to queue
void ps_queue_linux::copy_2message_parts_to_q(uint8_t *msg1, size_t len1, uint8_t *msg2, size_t len2)
{
    uint8_t *e = get_free_entry();
    
    if (e != nullptr)
    {
        size_t length1 = (len1 > nextPointerOffset ? nextPointerOffset : len1);
        memcpy(e, msg1, length1);
        
        size_t length2 = 0;
        
        if (msg2 != nullptr)
        {
            length2 = (len2 > nextPointerOffset-length1 ? nextPointerOffset-length1 : len2);
            if (length2 > 0)
            {
                memcpy((e + length1), msg2, length2);
            }
        }
        uint16_t *size = (uint16_t*)(e + sizeOffset);
        *size = (uint16_t) (length1+length2);
        
        append_queue_entry( e);
    }
}
void ps_queue_linux::copy_message_to_q(uint8_t *msg, size_t len)
{
    copy_2message_parts_to_q(msg, len, nullptr, 0);
}

//waits if empty, returns pointer (call done_with_message!)
uint8_t *ps_queue_linux::get_next_message(int msecs, size_t *length)
{
    uint8_t *e;
    
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
        uint8_t **next = (uint8_t **)(e + nextPointerOffset);

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
            uint16_t *size = (uint16_t*)(e + sizeOffset);
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
size_t ps_queue_linux::count()
{
    return queueCount;
}
void ps_queue_linux::done_with_message(uint8_t *msg)
{
    add_to_freelist(msg);
}

uint8_t *ps_queue_linux::get_free_entry()				//new broker q entry <- freelist
{
    uint8_t *entry;
    
    //critical section
    pthread_mutex_lock(&freeMtx);
    
    if (freelist == nullptr)
    {
        entry = new_queue_entry();
    }
    else
    {
        entry = freelist;
        uint8_t **next = (uint8_t **)(entry + nextPointerOffset);
        freelist = *next;
    }
    
    pthread_mutex_unlock(&freeMtx);
    //end critical section
    
    return entry;
}

uint8_t *ps_queue_linux::new_queue_entry()
{
    return (uint8_t *) calloc(1, queueEntrySize);
}

void ps_queue_linux::add_to_freelist(uint8_t *e)
{
    //critical section
    pthread_mutex_lock(&freeMtx);

    uint8_t **next = (uint8_t **)(e + nextPointerOffset);
    *next = freelist;
    freelist = e;
    
    pthread_mutex_unlock(&freeMtx);
    //end critical section
}
