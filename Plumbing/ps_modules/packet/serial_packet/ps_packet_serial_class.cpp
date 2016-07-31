//
//  ps_packet_serial_class.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_packet_serial_class.hpp"


ps_packet_serial_class::ps_packet_serial_class(ps_serial_class *_driver) : ps_packet_class(_driver->name)
{
	serial_driver = _driver;
	serial_driver->add_event_observer(this);

//    rxmsg = static_cast<uint8_t*>(malloc(max_packet_size));
//
//    if (rxmsg == NULL)
//    {
//        PS_ERROR("packet: no memory");
//    }
}
ps_packet_serial_class::~ps_packet_serial_class()
{
	delete serial_driver;
	free(rxmsg);
}

//serial driver event
void ps_packet_serial_class::process_observed_event(ps_root_class *src, int _event)
{
    ps_serial_status_enum stat = static_cast<ps_serial_status_enum>(_event);
    
	switch(stat)
	{
	case PS_SERIAL_OFFLINE:
		notify_new_event(this, PS_PACKET_OFFLINE);
		break;
	case PS_SERIAL_WRITE_ERROR:
	case PS_SERIAL_READ_ERROR:
		notify_new_event(this, PS_PACKET_ERROR);
		break;
	case PS_SERIAL_ONLINE:
		notify_new_event(this, PS_PACKET_ONLINE);
		break;
	}
}

uint16_t ps_packet_serial_class::calculate_checksum(const uint8_t *packet, int length)
{
    int i;
    int checksum = 0;
    
    for (i=0; i<length; i++)
    {
        int byte = *packet;
        checksum += byte;
        packet++;
    }
    return (checksum & 0xffff);
}

