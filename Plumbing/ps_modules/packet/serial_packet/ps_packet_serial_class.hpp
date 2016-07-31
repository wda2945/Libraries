//
//  ps_packet_serial_class.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_packet_serial_class_hpp
#define ps_packet_serial_class_hpp

#include "packet/ps_packet_class.hpp"
#include "serial/ps_serial_class.hpp"

#include "packet/packet_header.h"

#define STX_CHAR 0x7f

#define PS_PARSE_RESULT_NAMES {"Running", "Message", "Checksum"}

class ps_packet_serial_class : public ps_packet_class {

public:
	ps_packet_serial_class(ps_serial_class *_driver);
	virtual ~ps_packet_serial_class();
    
    ps_serial_class      *serial_driver;

    uint16_t calculate_checksum(const uint8_t *packet, int length);
    
    //send packet
    virtual ps_result_enum send_packet(const void *packet, int length) override = 0;
    
    //serial errors
    void process_observed_event(ps_root_class *src, int event) override;

    //not used
    void process_observed_data(ps_root_class *src, const void *msg, int length) override {}

protected:

    uint8_t rxmsg[200];
};

#endif /* ps_packet_serial_class_hpp */
