//
//  ps_transport_class.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_transport_hpp
#define ps_transport_hpp

#include "common/ps_root_class.hpp"
#include "packet/ps_packet_class.hpp"

typedef enum {
    PS_TRANSPORT_UNKNOWN,
    PS_TRANSPORT_ONLINE,
    PS_TRANSPORT_OFFLINE,
    PS_TRANSPORT_ADDED
} ps_transport_status_enum;

const int PS_TRANSPORT_RETRIES          = 10;
const int PS_TRANSPORT_QUEUEWAIT_MSECS  = 50;
const int PS_TRANSPORT_REPLYWAIT_MSECS  = 500;

const uint8_t PS_TRANSPORT_IGNORE_SEQ   = 0x01;
const uint8_t PS_TRANSPORT_RETX         = 0x02;
const uint8_t PS_TRANSPORT_ACK          = 0x04;
const uint8_t PS_TRANSPORT_NAK          = 0x08;

//status Rx
typedef enum {
    PS_TRANSPORT_RX_IDLE,
    PS_TRANSPORT_RX_WAIT,
    PS_TRANSPORT_RX_STATUS,
    PS_TRANSPORT_RX_DUP,
    PS_TRANSPORT_RX_SEQ_ERROR,
    PS_TRANSPORT_RX_NAK,
    PS_TRANSPORT_RX_MSG
}ps_transport_rx_status_enum;

typedef struct {
    uint16_t length;
    uint8_t sequenceNumber;
    uint8_t lastReceivedSequenceNumber;
    uint8_t status;
    uint8_t message[PS_DEFAULT_MAX_PACKET];
} ps_transport_packet_t;

const int PS_TRANSPORT_HEADER_OFFSET = (sizeof(uint16_t));
const int PS_TRANSPORT_HEADER_SIZE      = 3 * sizeof(uint8_t);
const int PS_TRANSPORT_MESSAGE_OFFSET = (PS_TRANSPORT_HEADER_OFFSET + PS_TRANSPORT_HEADER_SIZE);

typedef struct {
    uint8_t sequenceNumber;
    uint8_t lastReceivedSequenceNumber;
    uint8_t status;
} ps_transport_status_t;

class ps_transport_class : public ps_root_class {

protected:
    
    void (*data_callback)(ps_transport_class*, void *, size_t);
    
    void (*status_callback)(ps_transport_class*, ps_transport_status_enum);
    
public:
    
    ps_transport_status_enum transport_status;
    
    void change_status(ps_transport_status_enum newStatus);
    bool is_online();
    
    ps_packet_class *packet_driver;

    //send packet
    virtual void send_packet(void *packet, size_t length) = 0;
    
    //set callback to receive packets
    ps_result_enum set_data_callback(void (*_callback)(ps_transport_class*, void *, size_t));
    
    //set callback to receive packets
    ps_result_enum set_status_callback(void (*_callback)(ps_transport_class*, ps_transport_status_enum));
    
    //callback for new data packet
    void action_data_callback(void *pkt, size_t len);
    
    //callback for transmission errors
    void action_status_callback(ps_transport_status_enum stat);
};

#endif /* ps_transport_hpp */
