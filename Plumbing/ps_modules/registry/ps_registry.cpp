//
//  psRegistry.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_registry.hpp"

//make a hash value from the provided string
int ps_registry::get_hash_value(char *name)
{
	return 0;
}

//asssign a serial number (from 0) in the selected named domain
uint16_t assign_serial_number(char *domain)
{
	return 0;
}

//regisdter or update a value
ps_result_enum ps_registry::register_string(char *name, char *value)
{
	return PS_NOT_IMPLEMENTED;
}

//looknup a value by name
ps_result_enum ps_registry::lookup_string(char *name, char *buffer, size_t length)
{
	return PS_NOT_IMPLEMENTED;
}
