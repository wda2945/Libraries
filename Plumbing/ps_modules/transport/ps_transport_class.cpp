//
//  ps_transport_class.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "transport/ps_transport_class.hpp"

ps_transport_class:: ps_transport_class(ps_packet_class *driver)
	: ps_root_class(driver->name)
{
	max_packet_size = PS_MAX_PACKET;
	packet_driver = driver;

    packet_driver->add_data_observer(this);
    packet_driver->add_event_observer(this);
}
ps_transport_class::~ps_transport_class()
{
	delete packet_driver;
}

//process packet received
void ps_transport_class::process_packet_data(const void *pkt, int len)
{
    if (len >= (int) sizeof(ps_transport_header_t))
    {
        ps_transport_header_t *packet_header = (ps_transport_header_t *) pkt;
        
        //check for duplicate
        if (lastReceived == packet_header->sequenceNumber)
        {
            //duplicate message
            statusRx = PS_TRANSPORT_RX_DUP;
            //ignore
        }
        else
        {
            //new message
            //calc expected sequence number
            uint8_t nextSequence = lastReceived + 1;
            if (nextSequence == 1) nextSequence++;
            
            if (nextSequence == packet_header->sequenceNumber
                || lastReceived == 0
                || packet_header->sequenceNumber == 0
                || (packet_header->flags & PS_TRANSPORT_IGNORE_SEQ))
            {
                //sequence number good
            }
            else
            {
                //flag bad sequence number
                statusRx = PS_TRANSPORT_RX_SEQ_ERROR;
            }
            lastReceived = packet_header->sequenceNumber;
            remoteLastReceived = packet_header->lastReceivedSequenceNumber;
            remoteLastFlags = packet_header->flags;
            
            if (len > (int) sizeof(ps_transport_header_t))
            {
                int length = len - sizeof(ps_transport_header_t);
                pass_new_data((uint8_t*) pkt + sizeof(ps_transport_header_t), length);
                
                statusRx = PS_TRANSPORT_RX_MSG;
            }
            else
            {
                statusRx = PS_TRANSPORT_RX_STATUS;
            }
        }
    }
}

void ps_transport_class::process_packet_event(ps_packet_event_t ev)
{
	switch(ev)
	{
	case PS_PACKET_OFFLINE:
        transport_is_online = false;
        statusRx = PS_TRANSPORT_RX_NAK;
		change_status(PS_TRANSPORT_OFFLINE);
		break;
	case PS_PACKET_ONLINE:
		break;
	case PS_PACKET_REMOVED:
        transport_is_online = false;
        statusRx = PS_TRANSPORT_RX_NAK;
		change_status(PS_TRANSPORT_REMOVED);
		break;
	case PS_PACKET_ERROR:
		statusRx = PS_TRANSPORT_RX_NAK;
		break;
	default:
		break;
	}
}

void ps_transport_class::change_status(ps_transport_event_enum ev)
{
    if (transport_status != ev)
    {
        transport_status = ev;
        notify_new_event(ev);
    }
}

const char *transport_event_names[] = TRANSPORT_EVENT_NAMES;
