//
//  ps_transport_linux.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_transport_linux_hpp
#define ps_transport_linux_hpp

#include "transport/ps_transport_class.hpp"
#include "queue/linux/ps_queue_linux.hpp"
#include <pthread.h>

class ps_transport_linux : public ps_transport_class
{

public:
    //init a transport with the provided packet driver
	ps_transport_linux(ps_packet_class *driver);

    //send packet
    void send_packet(void *packet, int length);
    void send_packet2(void *packet1, int len1, void *packet2, int len2);

    friend void *transport_linux_send_thread_wrapper(void *arg);
protected:

    pthread_mutex_t protocolMtx = PTHREAD_MUTEX_INITIALIZER;	//access control
    pthread_cond_t protocolCond = PTHREAD_COND_INITIALIZER;	//signals item in previously empty queue

    ps_queue_linux *sendQueue;

    void transport_linux_send_thread_method();

    void packet_data_callback_method(ps_packet_class *pd, void *_pkt, int len);
    void packet_status_callback_method(ps_packet_class *pd, ps_packet_status stat);
};

#endif /* ps_transport_linux_hpp */
