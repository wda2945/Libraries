//
//  ps_packet_driver.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_packet_class_hpp
#define ps_packet_class_hpp

#include "common/ps_root_class.hpp"

typedef enum {
	PS_PACKET_OFFLINE,
	PS_PACKET_ONLINE,
	PS_PACKET_ERROR,
	PS_PACKET_REMOVED
} ps_packet_event_t;

class ps_packet_class : public ps_root_class {
    
public:
    ps_packet_class(const char *name);
    ps_packet_class(std::string name);

    virtual ~ps_packet_class();
    
    uint16_t            max_packet_size;            ///< max size of rxmsg buffer
    uint32_t            packet_rx_success_count;    ///< Received packets
    uint32_t            packet_rx_drop_count;       ///< Number of packet drops

    //send packet call from transport
    virtual ps_result_enum send_packet(const void *packet, int length) = 0;
    
public:

    //data and events from the serial layer
    virtual void process_observed_data(ps_root_class *src, const void *msg, int length) override = 0;
    virtual void process_observed_event(ps_root_class *src, int event) override = 0;
};

#endif /* ps_packet_class_hpp */
