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
} ps_packet_status;

class ps_packet_class;

typedef void (packet_data_callback_t)(void *, ps_packet_class *, void *, int);
typedef void (packet_status_callback_t)(void *, ps_packet_class *, ps_packet_status);

class ps_packet_class : public ps_root_class {
    
public:
    ps_packet_class(char *name, int max_packet);
    ps_packet_class(std::string name, int max_packet);
    virtual ~ps_packet_class();
    
    uint16_t            max_packet_size;            ///< max size of rxmsg buffer
    uint32_t            packet_rx_success_count;    ///< Received packets
    uint32_t            packet_rx_drop_count;       ///< Number of packet drops

    //send packet
    virtual ps_result_enum send_packet(void *packet, int length) = 0;
    
    //set callback to receive packets
    ps_result_enum set_data_callback(void *arg, packet_data_callback_t *dcb);

    //set callback to receive errors
    ps_result_enum set_status_callback(void *arg, packet_status_callback_t *ecb);
    
    //callback for new data packet
    void action_data_callback(void *pkt, int len);
    
    //callback for transmission errors
    void action_status_callback(ps_packet_status res);

protected:

    //callback for new data packet
	packet_data_callback_t *dataCallback;
    void *dataArg;

    //callback for transmission errors
    packet_status_callback_t *statusCallback;
    void *statusArg;

};

#endif /* ps_packet_class_hpp */
