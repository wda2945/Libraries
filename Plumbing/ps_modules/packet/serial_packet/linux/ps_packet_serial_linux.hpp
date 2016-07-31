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
#include <thread>
using namespace std;

class ps_packet_serial_linux : public ps_packet_serial_class {
    
public:

    ps_packet_serial_linux(ps_serial_class *_driver);
    ~ps_packet_serial_linux();
    
    //send packet
    ps_result_enum send_packet(const void *packet, int length) override;
    
protected:
    thread *packet_thread;
    
    void packet_serial_rx_thread_method();

};

#endif /* ps_packet_serial_linux_hpp */
