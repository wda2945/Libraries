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
#include "serial/ps_serial_class.hpp"
#include <pthread.h>

class ps_packet_serial_linux : public ps_packet_serial_class {
    
    pthread_mutex_t	sendMtx = PTHREAD_MUTEX_INITIALIZER;
    
public:
    ps_packet_serial_linux(const char *_name, ps_serial_class *driver, size_t maxPacket);
    
    //send packet
    ps_result_enum send_packet(uint8_t *packet, size_t length);
    
};

#endif /* ps_packet_serial_linux_hpp */
