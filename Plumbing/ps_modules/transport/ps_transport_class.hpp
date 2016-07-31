//
//  ps_transport_class.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_transport_hpp
#define ps_transport_hpp

#include <set>
#include "common/ps_root_class.hpp"
#include "packet/ps_packet_class.hpp"
#include "transport/transport_header.h"

typedef enum {
    PS_TRANSPORT_UNKNOWN,
    PS_TRANSPORT_ONLINE,
    PS_TRANSPORT_OFFLINE,
    PS_TRANSPORT_ADDED,
	PS_TRANSPORT_REMOVED
} ps_transport_event_enum;

#define TRANSPORT_EVENT_NAMES {\
    "PS_TRANSPORT_UNKNOWN",\
    "PS_TRANSPORT_ONLINE",\
    "PS_TRANSPORT_OFFLINE",\
    "PS_TRANSPORT_ADDED",\
    "PS_TRANSPORT_REMOVED"}

extern const char *transport_event_names[];

//parameters
const int PS_TRANSPORT_RETRIES          = 10;       //retransmissions -> offline
const int PS_TRANSPORT_QUEUEWAIT_MSECS  = 50;       //wait on queue for new message = status interval
const int PS_TRANSPORT_REPLYWAIT_MSECS  = 500;      //wait for ack

//transport flags
const uint8_t PS_TRANSPORT_IGNORE_SEQ   = 0x01;     //ignore sequence number at start
const uint8_t PS_TRANSPORT_RETX         = 0x02;     //re-transmission
const uint8_t PS_TRANSPORT_ACK          = 0x04;
const uint8_t PS_TRANSPORT_NAK          = 0x08;
const uint8_t PS_TRANSPORT_SEQ_ERROR    = 0x10;     //out of sequence message
const uint8_t PS_TRANSPORT_DUP          = 0x20;     //duplicate received

//status Rx
typedef enum {
    PS_TRANSPORT_RX_IDLE,
    PS_TRANSPORT_RX_WAIT,           //send thread waiting for ack/nack packet
    PS_TRANSPORT_RX_STATUS,         //status-only received
    PS_TRANSPORT_RX_DUP,            //duplicate ignored
    PS_TRANSPORT_RX_SEQ_ERROR,      //out of sequence
    PS_TRANSPORT_RX_NAK,
    PS_TRANSPORT_RX_MSG             //good message received
}ps_transport_rx_status_enum;

class ps_transport_class : public ps_root_class {

public:
    ps_transport_class(ps_packet_class *driver);
    virtual ~ps_transport_class();

    std::set<ps_topic_id_t> topicList;  //subscribed topics
    
    Source_t transport_source;          //generally, subsystem id
    
    int max_packet_size;                //from broker
    
    ps_packet_class *packet_driver;
    
    ps_transport_rx_status_enum transport_rx_status;
    
    virtual void change_status(ps_transport_event_enum newStatus);
    
    virtual bool is_online() {return transport_is_online;}
    
    //send packet
    virtual void send_packet(const void *packet, int len) = 0;
    virtual void send_packet2(const void *packet1, int len1, const void *packet2, int len2) = 0;	//convenience version
    
protected:
    bool transport_is_online {false};
    
    ps_transport_event_enum transport_status {PS_TRANSPORT_OFFLINE};

    //protocol data - serial numbers
    uint8_t lastReceived;
    uint8_t remoteLastReceived;
    
    uint8_t lastSent;
    
    uint8_t flags;
    uint8_t remoteLastFlags;
    
    ps_transport_rx_status_enum statusRx;

public:
    void process_observed_data(ps_root_class *src, const void *msg, int length) override;
    void process_observed_event(ps_root_class *src, int event) override;
};

#endif /* ps_transport_hpp */
