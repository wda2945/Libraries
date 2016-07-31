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
#include <thread>
#include <mutex>

using namespace std;

class ps_transport_linux : public ps_transport_class
{

public:
    //init a transport with the provided packet driver
	ps_transport_linux(ps_packet_class *driver);
    ~ps_transport_linux();
    
    //send packet - called by broker
    void send_packet(const void *packet, int length) override;
    void send_packet2(const void *packet1, int len1, const void *packet2, int len2) override;

protected:

    thread              *send_thread;
    
    mutex               protocolMtx;	//access control to protocol data
    condition_variable  protocolCond;	//signals reply packet received

    ps_queue_linux *sendQueue;

    void transport_linux_send_thread_method();

    //////////// Packet -> Transport callback methods
    virtual void process_observed_data(ps_root_class *src, const void *_pkt, int len) override;    //incoming data from packet
    virtual void process_observed_event(ps_root_class *src, int stat) override;
};

#endif /* ps_transport_linux_hpp */
