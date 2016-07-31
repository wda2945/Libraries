//
//  ps_transport_linux.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <chrono>
#include <signal.h>

#include "ps_common.h"
#include "ps_transport_linux.hpp"
#include "network/ps_network.hpp"

using namespace std::chrono;
using namespace std;

ps_transport_linux::ps_transport_linux(ps_packet_class *_driver)
: ps_transport_class(_driver)
{
    sendQueue = new ps_queue_linux(max_packet_size, 0);
    
    send_thread = new thread([this](){transport_linux_send_thread_method();});
}

ps_transport_linux::~ps_transport_linux()
{
    delete send_thread;
    delete sendQueue;
}

//callback function for new data packet
void ps_transport_linux::process_observed_data(ps_root_class *src, const void *_pkt, int len)
{
    //critical section
    unique_lock<mutex> lck {protocolMtx};
        
    ps_transport_class::process_observed_data(src, _pkt, len);
    
    //signal send thread
    protocolCond.notify_one();
}

//callback function for transmission errors
void  ps_transport_linux::process_observed_event(ps_root_class *src, int ev)
{
    //critical section
    unique_lock<mutex> lck {protocolMtx};
        
    ps_transport_class::process_observed_event(src, ev);
    
    //signal send thread
    protocolCond.notify_one();
}

void ps_transport_linux::transport_linux_send_thread_method()
{
    ps_transport_header_t statusPkt;
    
	const struct sigaction sa {SIG_IGN, 0, 0};
	sigaction(SIGPIPE, &sa, NULL);

    while (1)
    {
        //initially offline
        if (!transport_is_online)
        {
            sleep(1);
            
            //empty send queue
            void *pkt;
            do {
                pkt = sendQueue->get_next_message(0, nullptr);
                sendQueue->done_with_message(pkt);
            } while (pkt);
            
            //send a status packet as a poll
            
            statusPkt.sequenceNumber = lastSent;
            statusPkt.lastReceivedSequenceNumber = lastReceived;
            statusPkt.flags = PS_TRANSPORT_IGNORE_SEQ;
            
            //send status poll
//            PS_DEBUG("trns: tx status packet");
            
            packet_driver->send_packet(&statusPkt, sizeof(statusPkt));
            {
                //critical section
                unique_lock<mutex> lck {protocolMtx};
                
                statusRx = PS_TRANSPORT_RX_WAIT;
                
                //wait for a received packet
                auto now = system_clock::now();
                
                protocolCond.wait_until(lck, now + milliseconds(PS_TRANSPORT_REPLYWAIT_MSECS));
                
                if (statusRx == PS_TRANSPORT_RX_WAIT){
//                    PS_DEBUG("trns: status packet timeout");
                    continue; //timeout
                }
                //critical section end
            }
            
            PS_DEBUG("trns: online");
            //reply received - online
            change_status(PS_TRANSPORT_ONLINE);
        }
        
        while (transport_is_online)
        {
            //wait for a message to send
            int length;
            void *pkt = sendQueue->get_next_message(PS_TRANSPORT_QUEUEWAIT_MSECS, &length);
            
            if (pkt)
            {
//                PS_DEBUG("trns: packet to send");

                ps_transport_header_t *packet_header = (ps_transport_header_t *) pkt;
                
                switch (statusRx)
                {
                    default:
                        packet_header->flags = 0;
                        break;
                    case PS_TRANSPORT_RX_NAK:
                        packet_header->flags = PS_TRANSPORT_NAK;
                        break;
                    case PS_TRANSPORT_RX_DUP:
                        packet_header->flags = PS_TRANSPORT_DUP;
                        break;
                    case PS_TRANSPORT_RX_MSG:
                        packet_header->flags = PS_TRANSPORT_ACK;
                        break;
                    case PS_TRANSPORT_RX_SEQ_ERROR:
                        packet_header->flags = PS_TRANSPORT_SEQ_ERROR;
                        break;
                }
                
                int retryCount = PS_TRANSPORT_RETRIES;
                
                if (++lastSent == 0) lastSent++;
                
                packet_header->lastReceivedSequenceNumber = lastReceived;
                packet_header->sequenceNumber = lastSent;
                
                while(1)
                {
                     packet_driver->send_packet(pkt, length);
                    
                    {
                        //critical section
                        unique_lock<mutex> lck {protocolMtx};
                        
                        statusRx = PS_TRANSPORT_RX_WAIT;
                        
                        auto now = system_clock::now();
                        protocolCond.wait_until(lck, now + milliseconds(PS_TRANSPORT_REPLYWAIT_MSECS));
                        
                        if (statusRx != PS_TRANSPORT_RX_WAIT) break;
                        
                        if (--retryCount == 0)
                        {
                            PS_DEBUG("trns: tx failed");
                            change_status(PS_TRANSPORT_OFFLINE);
                            break;
                        }
                    }
                    
                    packet_header->flags |= PS_TRANSPORT_RETX;
                    
                }
                
                sendQueue->done_with_message(pkt);
            }
            else
            {
                //queue read timeout - send status packet
                
                switch (statusRx)
                {
                    default:
                        statusPkt.flags = 0;
                        break;
                    case PS_TRANSPORT_RX_NAK:
                        statusPkt.flags = PS_TRANSPORT_NAK;
                        break;
                    case PS_TRANSPORT_RX_DUP:
                        statusPkt.flags = PS_TRANSPORT_DUP;
                        break;
                    case PS_TRANSPORT_RX_MSG:
                        statusPkt.flags = PS_TRANSPORT_ACK;
                        break;
                    case PS_TRANSPORT_RX_SEQ_ERROR:
                        statusPkt.flags = PS_TRANSPORT_SEQ_ERROR;
                        break;
                }
                statusPkt.flags |= PS_TRANSPORT_IGNORE_SEQ;
                statusPkt.sequenceNumber = lastSent;
                statusPkt.lastReceivedSequenceNumber = lastReceived;

                
                int retryCount = PS_TRANSPORT_RETRIES;
                
                do {
                    //status packet retries
//                    PS_DEBUG("trns: tx status packet");

                    //send status poll
                    packet_driver->send_packet(&statusPkt, sizeof(statusPkt));
                    {
                        //critical section
                        unique_lock<mutex> lck {protocolMtx};
                        
                        //wait for a signal
                        statusRx = PS_TRANSPORT_RX_WAIT;
                        
                        auto now = system_clock::now();
                        protocolCond.wait_until(lck, now + milliseconds(PS_TRANSPORT_REPLYWAIT_MSECS));
                        
                        if (statusRx != PS_TRANSPORT_RX_WAIT) break;
                        
                        if (--retryCount == 0){
                            PS_DEBUG("trns: tx failed");
                            change_status(PS_TRANSPORT_OFFLINE);
                            break;
                        }
                    }
                    
                } while (statusRx == PS_TRANSPORT_RX_WAIT);
            }
        }
    }
}

//send packet
void ps_transport_linux::send_packet2(const void *packet1, int len1, const void *packet2, int len2)
{
    if (!transport_is_online) {
//        PS_DEBUG("trns: offline");
        return;
    }
    
    ps_transport_header_t packetHeader;	//dummy for later use
    sendQueue->copy_3message_parts_to_q(&packetHeader, sizeof(ps_transport_header_t), packet1, len1, packet2, len2);
}
void ps_transport_linux::send_packet(const void *packet, int length)
{
    send_packet2(packet, length, nullptr, 0);
}
