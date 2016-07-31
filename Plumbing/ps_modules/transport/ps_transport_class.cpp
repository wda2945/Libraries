//
//  ps_transport_class.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "transport/ps_transport_class.hpp"
#include "pubsub/ps_pubsub_class.hpp"

ps_transport_class:: ps_transport_class(ps_packet_class *driver)
	: ps_root_class(driver->name)
{
    max_packet_size = the_broker().max_ps_packet;
	packet_driver = driver;

    packet_driver->add_data_observer(this);
    packet_driver->add_event_observer(this);
}
ps_transport_class::~ps_transport_class()
{
	delete packet_driver;
}

//process packet received
void ps_transport_class::process_observed_data(ps_root_class *src, const void *pkt, int len)
{
    if (len >= (int) sizeof(ps_transport_header_t))
    {
        ps_transport_header_t *packet_header = (ps_transport_header_t *) pkt;
        uint8_t flags = packet_header->flags;
        
        //check for duplicate
        if ((lastReceived == packet_header->sequenceNumber) && ((flags & PS_TRANSPORT_IGNORE_SEQ) == 0))
        {
            //duplicate message
            remoteLastReceived = packet_header->lastReceivedSequenceNumber;
            remoteLastFlags = packet_header->flags;
            
            PS_DEBUG("trns: duplicate packet %i", lastReceived);
            statusRx = PS_TRANSPORT_RX_DUP;
            //ignore
        }
        else
        {
            //new message
            //calc expected sequence number
            uint8_t nextSequence = lastReceived + 1;
            if (nextSequence == 0) nextSequence++;
            
            if (nextSequence == packet_header->sequenceNumber
                || lastReceived == 0
                || packet_header->sequenceNumber == 0
                || (flags & PS_TRANSPORT_IGNORE_SEQ))
            {
                //sequence number good
            }
            else
            {
                //flag bad sequence number
                PS_DEBUG("trns: bad sequence %i, expected %i", packet_header->sequenceNumber, nextSequence);
                statusRx = PS_TRANSPORT_RX_SEQ_ERROR;
            }
            lastReceived = packet_header->sequenceNumber;
            remoteLastReceived = packet_header->lastReceivedSequenceNumber;
            remoteLastFlags = packet_header->flags;
            
            if (len > (int) sizeof(ps_transport_header_t))
            {
//                PS_DEBUG("trns: new message");

                int length = len - sizeof(ps_transport_header_t);
                pass_new_data(this, (uint8_t*) pkt + sizeof(ps_transport_header_t), length);
                
                statusRx = PS_TRANSPORT_RX_MSG;
            }
            else
            {
//                PS_DEBUG("trns: rx status packet");

                statusRx = PS_TRANSPORT_RX_STATUS;
            }
        }
    }
}

void ps_transport_class::process_observed_event(ps_root_class *src, int _event)
{
    ps_packet_event_t ev = static_cast<ps_packet_event_t>(_event);
    
	switch(ev)
	{
	case PS_PACKET_OFFLINE:
		statusRx = PS_TRANSPORT_RX_NAK;
        change_status(PS_TRANSPORT_OFFLINE);
		break;
	case PS_PACKET_ONLINE:
        change_status(PS_TRANSPORT_ONLINE);
		break;
	case PS_PACKET_REMOVED:
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
        PS_DEBUG("trns: %s", transport_event_names[ev]);
        transport_status = ev;

        switch(transport_status)
        {
        case PS_TRANSPORT_ONLINE:
        	transport_is_online = true;
        	break;

        case PS_TRANSPORT_OFFLINE:
        case PS_TRANSPORT_REMOVED:
        	transport_is_online = false;
        	break;

        default:
        	break;
        }

        notify_new_event(this, ev);
    }
}

const char *transport_event_names[] = TRANSPORT_EVENT_NAMES;
