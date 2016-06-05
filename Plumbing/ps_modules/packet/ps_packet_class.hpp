//
//  ps_packet_driver.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_packet_class_hpp
#define ps_packet_class_hpp

#include "ps_types.h"
#include "common/ps_root_class.hpp"

class ps_packet_class : public ps_root_class {
    
protected:
    
    //callback for new data packet
    void (*dataCallback)(void *, ps_packet_class *, void *, size_t);
    void *dataArg;
    
    //callback for transmission errors
    void (*errorCallback)(void *, ps_packet_class *, ps_result_enum);
    void *errorArg;
    
public:
    
    uint16_t            max_packet_size;            ///< max size of rxmsg buffer
    uint32_t            packet_rx_success_count;    ///< Received packets
    uint32_t            packet_rx_drop_count;       ///< Number of packet drops

    //send packet
    virtual ps_result_enum send_packet(uint8_t *packet, size_t length) = 0;
    
    //set callback to receive packets
    ps_result_enum set_data_callback(void *arg,
                                  void (*_dataCallback)(void *arg, ps_packet_class *, void *, size_t));

    //set callback to receive errors
    ps_result_enum set_error_callback(void *arg,
                                   void (*_errorCallback)(void *arg, ps_packet_class *, ps_result_enum));
    
    //callback for new data packet
    void action_data_callback(void *pkt, size_t len);
    
    //callback for transmission errors
    void action_error_callback(ps_result_enum res);
    
};

#endif /* ps_packet_class_hpp */
