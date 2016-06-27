//
//  ps_transport_linux.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_common.h"
#include "ps_transport_linux.hpp"
#include "network/ps_network.hpp"
#include <string.h>
#include <sys/time.h>

void *transport_linux_send_thread_wrapper(void *arg);

ps_transport_linux::ps_transport_linux(ps_packet_class *_driver)
	: ps_transport_class(_driver)
{
    sendQueue = new ps_queue_linux(max_packet_size, 0);

    pthread_t thread;
    int s = pthread_create(&thread, NULL, transport_linux_send_thread_wrapper, (void*) this);
	if (s != 0)
	{
		PS_ERROR("transport: Thread error %i\n", s);
	}
}

//callback function for new data packet
void ps_transport_linux::packet_data_callback_method(ps_packet_class *pd, void *_pkt, int len)
{
    //critical section
    pthread_mutex_lock(&protocolMtx);
    
    process_received_message(_pkt, len);

    pthread_mutex_unlock(&protocolMtx);
    //end critical section
    
    //signal send thread
    pthread_cond_signal(&protocolCond);
}

//callback function for transmission errors
void  ps_transport_linux::packet_status_callback_method(ps_packet_class *pd, ps_packet_status stat)
{
    //critical section
    pthread_mutex_lock(&protocolMtx);

    process_packet_status_callback(stat);

    pthread_mutex_unlock(&protocolMtx);
    //end critical section

    //signal send thread
    pthread_cond_signal(&protocolCond);
}

void ps_transport_linux::transport_linux_send_thread_method()
{
    struct timespec abstime;
    struct timeval timenow;
    
    ps_transport_status_t statusPkt;
    
    while (1)
    {
        //offline
        //critical section
        pthread_mutex_lock(&protocolMtx);
        
        change_status(PS_TRANSPORT_OFFLINE);
        statusPkt.sequenceNumber = 0;
        statusPkt.lastReceivedSequenceNumber = lastReceived;
        statusPkt.status = statusTx;
        
        //send status poll
        packet_driver->send_packet(&statusPkt, sizeof(statusPkt));
        
        statusRx = PS_TRANSPORT_RX_WAIT;
        
        gettimeofday(&timenow, NULL);
        abstime.tv_sec = timenow.tv_sec;
        abstime.tv_nsec = (timenow.tv_usec * 1000) + (PS_TRANSPORT_REPLYWAIT_MSECS * 1000000);
        
        pthread_cond_timedwait(&protocolCond, &protocolMtx, &abstime);
        
        if (statusRx == PS_TRANSPORT_RX_WAIT) continue;
        change_status(PS_TRANSPORT_ONLINE);
        
        statusTx = PS_TRANSPORT_IGNORE_SEQ;
        statusRx = PS_TRANSPORT_RX_IDLE;
        
        //critical section end
        pthread_mutex_unlock(&protocolMtx);
        
        while (is_online())
        {
            //wait for a message to send
            int length;
            void *pkt = sendQueue->get_next_message(PS_TRANSPORT_QUEUEWAIT_MSECS, &length);
            ps_transport_packet_header_t *packet_header = (ps_transport_packet_header_t *) pkt;
            
            //critical section
            pthread_mutex_lock(&protocolMtx);
            
            if (pkt)
            {
                int retryCount = PS_TRANSPORT_RETRIES;
                
                void *packet = (void*) ( (uint8_t*) pkt + sizeof(ps_transport_packet_header_t));
                if (++lastSent == 0) lastSent++;
                packet_header->sequenceNumber = lastSent;
                packet_header->status = statusTx;
                
                do {
                	packet_header->lastReceivedSequenceNumber = lastReceived;
                    
                    packet_driver->send_packet(packet, packet_header->length);
                    
                    statusRx = PS_TRANSPORT_RX_WAIT;
                    
                    gettimeofday(&timenow, NULL);
                    abstime.tv_sec = timenow.tv_sec;
                    abstime.tv_nsec = (timenow.tv_usec * 1000) + (PS_TRANSPORT_REPLYWAIT_MSECS * 1000000);
                    
                    //wait for Rx packet signal with timeout
                    pthread_cond_timedwait(&protocolCond, &protocolMtx, &abstime);
                    
                    if (statusRx == PS_TRANSPORT_RX_WAIT){
                        if (--retryCount) continue;
                        else
                        {
                            change_status(PS_TRANSPORT_OFFLINE);
                            break;
                        }
                    }
                    statusTx = PS_TRANSPORT_RETX;
                    
                } while (remoteLastReceived != lastSent);
                
                statusRx = PS_TRANSPORT_RX_IDLE;
            }
            else
            {
                if (statusRx == PS_TRANSPORT_RX_IDLE)
                {
                    int retryCount = PS_TRANSPORT_RETRIES;
                    do {
                        //wait for a signal
                        statusRx = PS_TRANSPORT_RX_WAIT;
                        
                        gettimeofday(&timenow, NULL);
                        abstime.tv_sec = timenow.tv_sec;
                        abstime.tv_nsec = (timenow.tv_usec * 1000) + (PS_TRANSPORT_REPLYWAIT_MSECS * 1000000);
                        
                        //wait for Rx packet signal with timeout
                        pthread_cond_timedwait(&protocolCond, &protocolMtx, &abstime);
                        
                        if (statusRx == PS_TRANSPORT_RX_WAIT){
                            if (--retryCount) continue;
                            else
                            {
                                change_status(PS_TRANSPORT_OFFLINE);
                                break;
                            }
                        }
                        
                    } while (statusRx == PS_TRANSPORT_RX_WAIT);
                }
                
                //need to send a Status
                switch (statusRx)
                {
                    case PS_TRANSPORT_RX_WAIT:
                    case PS_TRANSPORT_RX_IDLE:
                        break;
                    case PS_TRANSPORT_RX_DUP:
                    case PS_TRANSPORT_RX_SEQ_ERROR:
                    case PS_TRANSPORT_RX_NAK:
                        statusTx = PS_TRANSPORT_NAK;
                        break;
                    case PS_TRANSPORT_RX_STATUS:
                    case PS_TRANSPORT_RX_MSG:
                        statusTx = PS_TRANSPORT_ACK;
                        break;
                }
                statusPkt.sequenceNumber = 0;
                statusPkt.lastReceivedSequenceNumber = lastReceived;
                statusPkt.status = statusTx;
                
                packet_driver->send_packet((uint8_t*)&statusPkt, sizeof(statusPkt));
                
                statusRx = PS_TRANSPORT_RX_IDLE;
                
            }
            
            //critical section end
            pthread_mutex_unlock(&protocolMtx);
        }
    }
}

void *transport_linux_send_thread_wrapper(void *arg)
{
     ps_transport_linux *pst = (ps_transport_linux*) arg;
     pst->transport_linux_send_thread_method();
     //no return
     return 0;
}

//send packet
void ps_transport_linux::send_packet2(void *packet1, int len1, void *packet2, int len2)
{
	ps_transport_packet_header_t packetHeader;	//dummy for future use
    sendQueue->copy_3message_parts_to_q(&packetHeader, sizeof(ps_transport_packet_header_t), packet1, len1, packet2, len2);
}
void ps_transport_linux::send_packet(void *packet, int length)
{
	send_packet2(packet, length, nullptr, 0);
}
