//
//  ps_transport_class.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "transport/ps_transport_class.hpp"

//callback function for new data packet
void packet_data_callback(void *arg, ps_packet_class *pd, void *_pkt, int len);
//callback function for transmission errors
void packet_status_callback(void *arg, ps_packet_class *pd, ps_packet_status stat);

ps_transport_class:: ps_transport_class(ps_packet_class *driver)
	: ps_root_class(driver->name)
{
	max_packet_size = driver->max_packet_size - sizeof(ps_transport_packet_header_t);
	packet_driver = driver;

    packet_driver->set_data_callback((void*) this, &packet_data_callback);
    packet_driver->set_status_callback((void*) this, &packet_status_callback);
}
ps_transport_class::~ps_transport_class()
{
	delete packet_driver;
}

//callback function for new data packet
void packet_data_callback(void *arg, ps_packet_class *pd, void *_pkt, int len)
{
	ps_transport_class *ptc = (ps_transport_class*) arg;
	ptc->packet_data_callback_method(pd, _pkt, len);
}
//callback function for transmission errors
void packet_status_callback(void *arg, ps_packet_class *pd, ps_packet_status stat)
{
	ps_transport_class *ptc = (ps_transport_class*) arg;
	ptc->packet_status_callback_method(pd, stat);
}

void ps_transport_class::process_received_message(void *pkt, int len)
{
	ps_transport_packet_header_t *packet_header = (ps_transport_packet_header_t *) pkt;

	//check for Ack
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
				|| (packet_header->status & PS_TRANSPORT_IGNORE_SEQ))
		{
			//sequence number good
			lastReceived = packet_header->sequenceNumber;
			remoteLastReceived = packet_header->lastReceivedSequenceNumber;
			remoteLastStatus = packet_header->status;

			if (packet_header->length > sizeof(ps_transport_packet_header_t))
			{
				int length = packet_header->length - sizeof(ps_transport_packet_header_t);
				action_data_callback((uint8_t*) pkt + sizeof(ps_transport_packet_header_t), length);

				statusRx = PS_TRANSPORT_RX_MSG;
			}
			else
			{
				statusRx = PS_TRANSPORT_RX_STATUS;
			}
		}
		else
		{
			//flag bad sequence number
			statusRx = PS_TRANSPORT_RX_SEQ_ERROR;
		}
	}
}

void ps_transport_class::process_packet_status_callback(ps_packet_status stat)
{
	switch(stat)
	{
	case PS_PACKET_OFFLINE:
		change_status(PS_TRANSPORT_OFFLINE);
		break;
	case PS_PACKET_ONLINE:
		change_status(PS_TRANSPORT_ONLINE);
		break;
	case PS_PACKET_REMOVED:
		change_status(PS_TRANSPORT_REMOVED);
		delete this;
	case PS_PACKET_ERROR:
		statusRx = PS_TRANSPORT_RX_NAK;
		remoteLastStatus = 0;
		break;
	default:
		break;
	}
}

//set callback to receive packets
ps_result_enum ps_transport_class::set_data_callback(transport_data_callback_t *_callback)
{
    data_callback = _callback;
    return PS_OK;
}

//set callback to receive packets
ps_result_enum ps_transport_class::set_status_callback(transport_status_callback_t *_callback)
{
    status_callback = _callback;
    return PS_OK;
}

//callback for new data packet
void ps_transport_class::action_data_callback(void *pkt, int len)
{
    if (data_callback) (*data_callback)(this, pkt, len);
}

//callback for status changes
void ps_transport_class::action_status_callback(ps_transport_status_enum stat)
{
    if (status_callback) (*status_callback)(this, stat);
}

void ps_transport_class::change_status(ps_transport_status_enum status)
{
    if (transport_status != status)
    {
        transport_status = status;
        action_status_callback(status);
    }
}
bool ps_transport_class::is_online()
{
    return (transport_status == PS_TRANSPORT_ONLINE);
}
