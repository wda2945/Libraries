//
//  ps_transport_linux.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "transport/linux/ps_transport_linux.hpp"
#include "network/ps_network.hpp"
#include <string.h>
#include <sys/time.h>

//callback function for new data packet
void packet_data_callback(void *arg, ps_packet_class *pd, void *_pkt, size_t len);
//callback function for transmission errors
void packet_error_callback(void *arg, ps_packet_class *pd, ps_result_enum len);

void *transport_linux_send_thread(void *arg);

ps_transport_linux::ps_transport_linux(const char *_name, ps_packet_class *_driver)
{
	set_node_name(_name);
	set_node_tag();

    packet_driver = _driver;

    sendQueue = new ps_queue_linux(sizeof(ps_transport_packet_t), PS_DEFAULT_PRELOAD);

    packet_driver->set_data_callback((void*) this, &packet_data_callback);

    packet_driver->set_error_callback((void*) this, &packet_error_callback);

    pthread_t thread;
    pthread_create(&thread, NULL, transport_linux_send_thread, (void*) this);

    the_network().add_transport_to_network(this);
}

//callback function for new data packet
void packet_data_callback(void *arg, ps_packet_class *pd, void *_pkt, size_t len)
{
	ps_transport_linux *pst = (ps_transport_linux*) arg;
    ps_transport_packet_t *pkt = (ps_transport_packet_t *) _pkt;
    
    //critical section
    pthread_mutex_lock(&pst->protocolMtx);
    
    //check for Ack
    if (pst->lastReceived == pkt->sequenceNumber)
    {
        //duplicate message
        pst->statusRx = PS_TRANSPORT_RX_DUP;
        //ignore
    }
    else
    {
        //new message
        //calc expected sequence number
        uint8_t nextSequence = pst->lastReceived + 1;
        if (nextSequence == 1) nextSequence++;
        
        if (nextSequence == pkt->sequenceNumber
            || pst->lastReceived == 0
            || pkt->sequenceNumber == 0
            || (pkt->status & PS_TRANSPORT_IGNORE_SEQ))
        {
            //sequence number good
            pst->lastReceived = pkt->sequenceNumber;
            pst->remoteLastReceived = pkt->lastReceivedSequenceNumber;
            pst->remoteLastStatus = pkt->status;
            
            if (pkt->length > PS_TRANSPORT_HEADER_SIZE)
            {
                size_t pktLength = pkt->length - PS_TRANSPORT_HEADER_SIZE;
                pst->action_data_callback(pkt->message, pktLength);
                
                pst->statusRx = PS_TRANSPORT_RX_MSG;
            }
            else
            {
                pst->statusRx = PS_TRANSPORT_RX_STATUS;
            }
        }
        else
        {
            //flag bad sequence number
            pst->statusRx = PS_TRANSPORT_RX_SEQ_ERROR;
        }
    }
    pthread_mutex_unlock(&pst->protocolMtx);
    //end critical section
    
    //signal send thread
    pthread_cond_signal(&pst->protocolCond);
}

//callback function for transmission errors
void packet_error_callback(void *arg, ps_packet_class *pd, ps_result_enum len)
{
	ps_transport_linux *pst = (ps_transport_linux*) arg;
    
    //critical section
    pthread_mutex_lock(&pst->protocolMtx);
    
    pst->statusRx = PS_TRANSPORT_RX_NAK;
    pst->remoteLastStatus = 0;
    pthread_mutex_unlock(&pst->protocolMtx);
    //end critical section
    
    //signal send thread
    pthread_cond_signal(&pst->protocolCond);
}

void *transport_linux_send_thread(void *arg)
{
    struct timespec abstime;
    struct timeval timenow;
    
    ps_transport_linux *pst = (ps_transport_linux*) arg;
    ps_transport_packet_t *pkt;
    
    ps_transport_status_t statusPkt;
    
    while (1)
    {
        //offline
        //critical section
        pthread_mutex_lock(&pst->protocolMtx);
        
        pst->change_status(PS_TRANSPORT_OFFLINE);
        statusPkt.sequenceNumber = 0;
        statusPkt.lastReceivedSequenceNumber = pst->lastReceived;
        statusPkt.status = pst->statusTx;
        
        //send status poll
        pst->packet_driver->send_packet((uint8_t*)&statusPkt, sizeof(statusPkt));
        
        pst->statusRx = PS_TRANSPORT_RX_WAIT;
        
        gettimeofday(&timenow, NULL);
        abstime.tv_sec = timenow.tv_sec;
        abstime.tv_nsec = (timenow.tv_usec * 1000) + (PS_TRANSPORT_REPLYWAIT_MSECS * 1000000);
        
        pthread_cond_timedwait(&pst->protocolCond, &pst->protocolMtx, &abstime);
        
        if (pst->statusRx == PS_TRANSPORT_RX_WAIT) continue;
        pst->change_status(PS_TRANSPORT_ONLINE);
        
        pst->statusTx = PS_TRANSPORT_IGNORE_SEQ;
        pst->statusRx = PS_TRANSPORT_RX_IDLE;
        
        //critical section end
        pthread_mutex_unlock(&pst->protocolMtx);
        
        while (pst->is_online())
        {
            //wait for a message to send
            size_t length;
            pkt = (ps_transport_packet_t *) pst->sendQueue->get_next_message(PS_TRANSPORT_QUEUEWAIT_MSECS, &length);
            
            //critical section
            pthread_mutex_lock(&pst->protocolMtx);
            
            if (pkt)
            {
                int retryCount = PS_TRANSPORT_RETRIES;
                
                uint8_t *packet = ( ((uint8_t*)pkt) + PS_TRANSPORT_HEADER_OFFSET);
                if (++pst->lastSent == 0) pst->lastSent++;
                pkt->sequenceNumber = pst->lastSent;
                pkt->status = pst->statusTx;
                
                do {
                    pkt->lastReceivedSequenceNumber = pst->lastReceived;
                    
                    pst->packet_driver->send_packet(packet, pkt->length);
                    
                    pst->statusRx = PS_TRANSPORT_RX_WAIT;
                    
                    gettimeofday(&timenow, NULL);
                    abstime.tv_sec = timenow.tv_sec;
                    abstime.tv_nsec = (timenow.tv_usec * 1000) + (PS_TRANSPORT_REPLYWAIT_MSECS * 1000000);
                    
                    //wait for Rx packet signal with timeout
                    pthread_cond_timedwait(&pst->protocolCond, &pst->protocolMtx, &abstime);
                    
                    if (pst->statusRx == PS_TRANSPORT_RX_WAIT){
                        if (--retryCount) continue;
                        else
                        {
                            pst->change_status(PS_TRANSPORT_OFFLINE);
                            break;
                        }
                    }
                    pst->statusTx = PS_TRANSPORT_RETX;
                    
                } while (pst->remoteLastReceived != pst->lastSent);
                
                pst->statusRx = PS_TRANSPORT_RX_IDLE;
            }
            else
            {
                if (pst->statusRx == PS_TRANSPORT_RX_IDLE)
                {
                    int retryCount = PS_TRANSPORT_RETRIES;
                    do {
                        //wait for a signal
                        pst->statusRx = PS_TRANSPORT_RX_WAIT;
                        
                        gettimeofday(&timenow, NULL);
                        abstime.tv_sec = timenow.tv_sec;
                        abstime.tv_nsec = (timenow.tv_usec * 1000) + (PS_TRANSPORT_REPLYWAIT_MSECS * 1000000);
                        
                        //wait for Rx packet signal with timeout
                        pthread_cond_timedwait(&pst->protocolCond, &pst->protocolMtx, &abstime);
                        
                        if (pst->statusRx == PS_TRANSPORT_RX_WAIT){
                            if (--retryCount) continue;
                            else
                            {
                                pst->change_status(PS_TRANSPORT_OFFLINE);
                                break;
                            }
                        }
                        
                    } while (pst->statusRx == PS_TRANSPORT_RX_WAIT);
                }
                
                //need to send a Status
                switch (pst->statusRx)
                {
                    case PS_TRANSPORT_RX_WAIT:
                    case PS_TRANSPORT_RX_IDLE:
                        break;
                    case PS_TRANSPORT_RX_DUP:
                    case PS_TRANSPORT_RX_SEQ_ERROR:
                    case PS_TRANSPORT_RX_NAK:
                        pst->statusTx = PS_TRANSPORT_NAK;
                        break;
                    case PS_TRANSPORT_RX_STATUS:
                    case PS_TRANSPORT_RX_MSG:
                        pst->statusTx = PS_TRANSPORT_ACK;
                        break;
                }
                statusPkt.sequenceNumber = 0;
                statusPkt.lastReceivedSequenceNumber = pst->lastReceived;
                statusPkt.status = pst->statusTx;
                
                pst->packet_driver->send_packet((uint8_t*)&statusPkt, sizeof(statusPkt));
                
                pst->statusRx = PS_TRANSPORT_RX_IDLE;
                
            }
            
            //critical section end
            pthread_mutex_unlock(&pst->protocolMtx);
        }
    }
    return 0;
}

//send packet
void ps_transport_linux::send_packet(void *packet, size_t _length)
{
    uint8_t packetHeader[PS_TRANSPORT_HEADER_SIZE];
    size_t length = (_length > PS_DEFAULT_MAX_PACKET ? PS_DEFAULT_MAX_PACKET : _length);
    
    sendQueue->copy_2message_parts_to_q(packetHeader, PS_TRANSPORT_HEADER_SIZE,
                                    (uint8_t*)packet, length);
}
