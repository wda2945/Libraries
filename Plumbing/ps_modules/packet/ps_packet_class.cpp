//
//  ps_packet_class.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_packet_class.hpp"

//set callback to receive packets
ps_result_enum ps_packet_class::set_data_callback(void *arg, void (*_dataCallback)(void *arg, ps_packet_class *, void *, size_t))
{
    dataCallback = _dataCallback;
    dataArg = arg;
    return PS_OK;
}

//set callback to receive errors
 ps_result_enum ps_packet_class::set_error_callback(void *arg, void (*_errorCallback)(void *arg, ps_packet_class *, ps_result_enum))
{
    errorCallback = _errorCallback;
    errorArg = arg;
    return PS_OK;
}

//callback for new data packet
void ps_packet_class::action_data_callback(void *pkt, size_t len)
{
    if (dataCallback) (*dataCallback)(dataArg, this, pkt, len);
}

//callback for transmission errors
void ps_packet_class::action_error_callback(ps_result_enum res)
{
    if (errorCallback) (*errorCallback)(errorArg, this, res);
}
