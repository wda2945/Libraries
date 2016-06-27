//
//  ps_packet_class.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_packet_class.hpp"

ps_packet_class::ps_packet_class(char *name, int _max_packet)
: ps_root_class(name)
{
	max_packet_size = _max_packet;
	if (max_packet_size == 0) max_packet_size = PS_DEFAULT_MAX_PACKET;
}

ps_packet_class::ps_packet_class(std::string name, int _max_packet)
: ps_root_class(name)
{
	max_packet_size = _max_packet;
	if (max_packet_size == 0) max_packet_size = PS_DEFAULT_MAX_PACKET;
}

ps_packet_class::~ps_packet_class()
{

}

//set callback to receive packets
ps_result_enum ps_packet_class::set_data_callback(void *arg, packet_data_callback_t *_dataCallback)
{
    dataCallback = _dataCallback;
    dataArg = arg;
    return PS_OK;
}

//set callback to receive errors
ps_result_enum ps_packet_class::set_status_callback(void *arg, packet_status_callback_t *_errorCallback)
{
	statusCallback = _errorCallback;
    statusArg = arg;
    return PS_OK;
}

//callback for new data packet
void ps_packet_class::action_data_callback(void *pkt, int len)
{
    if (dataCallback) (*dataCallback)(dataArg, this, pkt, len);
}

//callback for transmission errors
void ps_packet_class::action_status_callback(ps_packet_status res)
{
    if (statusCallback) (*statusCallback)(statusArg, this, res);
}
