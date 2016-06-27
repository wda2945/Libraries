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
    PS_TRANSPORT_ADDED,
	PS_TRANSPORT_REMOVED
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
} ps_transport_packet_header_t;

typedef struct {
    uint8_t sequenceNumber;
    uint8_t lastReceivedSequenceNumber;
    uint8_t status;
} ps_transport_status_t;

class ps_transport_class;

typedef void (transport_data_callback_t)(ps_transport_class*, void *, int);
typedef void (transport_status_callback_t)(ps_transport_class*, ps_transport_status_enum);

class ps_transport_class : public ps_root_class {

public:
    ps_transport_class(ps_packet_class *driver);
    virtual ~ps_transport_class();

    int max_packet_size;
    ps_packet_class *packet_driver;
    
    ps_transport_status_enum transport_status;
    
    virtual void change_status(ps_transport_status_enum newStatus);
    virtual bool is_online();
    
    //send packet
    virtual void send_packet(void *packet, int len) = 0;
    virtual void send_packet2(void *packet1, int len1, void *packet2, int len2) = 0;	//convenience version
    
    //set callback to receive packets
    ps_result_enum set_data_callback(transport_data_callback_t *dc);
    
    //set callback to receive packets
    ps_result_enum set_status_callback(transport_status_callback_t *sc);
    
    //callback for new data packet
    virtual void action_data_callback(void *pkt, int len);
    
    //callback for transmission errors
    virtual void action_status_callback(ps_transport_status_enum stat);

protected:

    transport_data_callback_t *data_callback;
    transport_status_callback_t *status_callback;

    virtual void process_received_message(void *pkt, int len);

    virtual void process_packet_status_callback(ps_packet_status stat);

    virtual void packet_data_callback_method(ps_packet_class *pd, void *_pkt, int len) = 0;

    virtual void packet_status_callback_method(ps_packet_class *pd, ps_packet_status stat) = 0;

    //protocol data
    uint8_t lastReceived;
    uint8_t remoteLastReceived;
    uint8_t lastSent;
    uint8_t remoteLastStatus;

    uint8_t statusTx;
    ps_transport_rx_status_enum statusRx;

    //callback function for new data packet
    friend void packet_data_callback(void *arg, ps_packet_class *pd, void *_pkt, int len);
    //callback function for transmission errors
    friend void packet_status_callback(void *arg, ps_packet_class *pd, ps_packet_status stat);
};

#endif /* ps_transport_hpp */
