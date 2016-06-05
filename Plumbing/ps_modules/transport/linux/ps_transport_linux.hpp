//
//  ps_transport_linux.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_transportLinux_hpp
#define ps_transportLinux_hpp

#include "transport/ps_transport_class.hpp"
#include "queue/linux/ps_queue_linux.hpp"
#include <pthread.h>

class ps_transport_linux : public ps_transport_class
{
    
public:
    //init a transport with the provided packet driver
	ps_transport_linux(const char * _name, ps_packet_class *driver);
    
    ps_queue_linux *sendQueue;

    //send packet
    void send_packet(void *packet, size_t length);

    //protocol data
    pthread_mutex_t protocolMtx = PTHREAD_MUTEX_INITIALIZER;	//access control
    pthread_cond_t protocolCond = PTHREAD_COND_INITIALIZER;	//signals item in previously empty queue

    uint8_t lastReceived;
    uint8_t remoteLastReceived;
    uint8_t lastSent;
    uint8_t remoteLastStatus;
    
    uint8_t statusTx;
    ps_transport_rx_status_enum statusRx;
};

#endif /* ps_transportLinux_hpp */
