//
//  ps_transport_class.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "transport/ps_transport_class.hpp"


//set callback to receive packets
ps_result_enum ps_transport_class::set_data_callback(void (*_callback)(ps_transport_class*, void *, size_t))
{
    data_callback = _callback;
    return PS_OK;
}

//set callback to receive packets
ps_result_enum ps_transport_class::set_status_callback(void (*_callback)(ps_transport_class*, ps_transport_status_enum))
{
    status_callback = _callback;
    return PS_OK;
}

//callback for new data packet
void ps_transport_class::action_data_callback(void *pkt, size_t len)
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
