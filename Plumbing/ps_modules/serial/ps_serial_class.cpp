//
//  ps_serial_class.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_serial_class.hpp"

ps_serial_class::ps_serial_class(char *name)
	: ps_root_class(name)
{

}

void ps_serial_class::set_error_callback(serial_error_callback_t *cb, void *arg)
{
	error_callback = cb;
	callback_arg = arg;
}

void ps_serial_class::action_error_callback(ps_serial_status_enum res)
{
	serial_status = res;
	if (error_callback) (error_callback)(callback_arg, this, res);
}

ps_serial_status_enum ps_serial_class::get_serial_status()
{
	return serial_status;
}
