//
//  ps_packet_serial_linux.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_packet_serial_linux_hpp
#define ps_packet_serial_linux_hpp

#include "packet/serial_packet/ps_packet_serial_class.hpp"

#include <pthread.h>

class ps_packet_serial_linux : public ps_packet_serial_class {
	pthread_t thread;
    pthread_mutex_t	sendMtx = PTHREAD_MUTEX_INITIALIZER;

    void packet_serial_rx_thread_method();
    
public:

    ps_packet_serial_linux(ps_serial_class *_driver);
    ~ps_packet_serial_linux(){}
    
    //send packet
    ps_result_enum send_packet(void *packet, int length);
    
    friend void *packet_serial_rx_thread_wrapper(void *arg);
    friend void serial_error_callback(void *arg, ps_serial_class *psc, ps_serial_status_enum stat);
};

#endif /* ps_packet_serial_linux_hpp */
