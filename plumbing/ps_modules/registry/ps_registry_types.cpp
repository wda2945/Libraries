//
//  psRegistry_types.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_registry_types.hpp"

//////////////////METHODS FOR PS_REGISTRY_VALUE

//update values if type matches
ps_result_enum copy_data(ps_registry_struct_t& to, const ps_registry_struct_t& from)
{
	if (to.datatype == from.datatype)
	{
		switch(to.datatype)
		{
		case PS_REGISTRY_TEXT_TYPE:
			strcpy(to.string_value, from.string_value);
			break;
		case PS_REGISTRY_INT_TYPE:
			to.int_value = from.int_value;
			break;
		case PS_REGISTRY_REAL_TYPE:
			to.real_value = from.real_value;
			break;
		case PS_REGISTRY_BOOL_TYPE:
			to.bool_value = from.bool_value;
			break;
		default:
			break;
		}
		return PS_OK;
	}
	else
	{
		return PS_INVALID_PARAMETER;
	}
}

ps_result_enum copy_all_data(ps_registry_struct_t& to, const ps_registry_struct_t& from)
{
    if ((to.datatype == from.datatype) || (to.datatype == PS_REGISTRY_UNKNOWN_TYPE))
    {
        to.datatype = from.datatype;
        to.flags    = from.flags;
        to.source   = from.source;
        to.serial   = from.serial;
        
        switch(to.datatype)
        {
            case PS_REGISTRY_TEXT_TYPE:
                strcpy(to.string_value, from.string_value);
                break;
            case PS_REGISTRY_INT_TYPE:
                to.int_min = from.int_min;
                to.int_max = from.int_max;
                to.int_value = from.int_value;
                break;
            case PS_REGISTRY_REAL_TYPE:
                to.real_min = from.real_min;
                to.real_max = from.real_max;
                to.real_value = from.real_value;
                break;
            case PS_REGISTRY_BOOL_TYPE:
                to.bool_value = from.bool_value;
                break;
            default:
                break;
        }
        return PS_OK;
    }
    else
    {
        return PS_INVALID_PARAMETER;
    }
}
