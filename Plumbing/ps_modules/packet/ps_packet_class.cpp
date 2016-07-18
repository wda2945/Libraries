//
//  ps_packet_class.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_packet_class.hpp"

ps_packet_class::ps_packet_class(const char *name, int _max_packet)
: ps_root_class(name)
{
	max_packet_size = _max_packet;
	if (max_packet_size == 0) max_packet_size = PS_MAX_PACKET;
}

ps_packet_class::ps_packet_class(std::string name, int _max_packet)
: ps_root_class(name)
{
	max_packet_size = _max_packet;
	if (max_packet_size == 0) max_packet_size = PS_MAX_PACKET;
}

ps_packet_class::~ps_packet_class()
{

}
