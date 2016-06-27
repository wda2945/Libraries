//
//  psRegistry_types.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_registry_types.hpp"

//////////////////METHODS FOR PS_REGISTRY_VALUE

ps_registry_value_class::ps_registry_value_class(){
	entry_type = 0;
}
ps_registry_value_class::ps_registry_value_class(std::string _string_value){
	entry_type = PS_REGISTRY_STRING_TYPE;
	data.string_value = new std::string(_string_value);
}
ps_registry_value_class::ps_registry_value_class(char *char_value){
	entry_type = PS_REGISTRY_STRING_TYPE;
	data.string_value = new std::string(char_value);
}
ps_registry_value_class::ps_registry_value_class(int min, int max, int value){
	entry_type = PS_REGISTRY_INT_TYPE;
	data.int_min = min;
	data.int_max = max;
	data.int_value = value;
}
ps_registry_value_class::ps_registry_value_class(float min, float max, float value){
	entry_type = PS_REGISTRY_REAL_TYPE;
	data.real_min = min;
	data.real_max = max;
	data.real_value = value;
}
ps_registry_value_class::ps_registry_value_class(bool bool_value){
	entry_type = PS_REGISTRY_BOOL_TYPE;
	data.bool_value = bool_value;
}
//copy constructor
ps_registry_value_class::ps_registry_value_class(ps_registry_value_class *from)
{
	entry_type = from->entry_type;
	switch(entry_type)
	{
	case PS_REGISTRY_STRING_TYPE:
		data.string_value = new std::string(*from->data.string_value);
		break;
	case PS_REGISTRY_INT_TYPE:
		data.int_min = from->data.real_min;
		data.int_max = from->data.real_max;
		data.int_value = from->data.real_value;
		break;
	case PS_REGISTRY_REAL_TYPE:
		data.real_min = from->data.int_min;
		data.real_max = from->data.int_max;
		data.real_value = from->data.int_value;
		break;
	case PS_REGISTRY_BOOL_TYPE:
		data.bool_value = from->data.bool_value;
		break;
	default:
		break;
	}
}
//update values if type matches
ps_result_enum ps_registry_value_class::copy_data(ps_registry_value_class *from)
{
	if (entry_type == from->entry_type)
	{
		switch(entry_type)
		{
		case PS_REGISTRY_STRING_TYPE:
			data.string_value = from->data.string_value;
			break;
		case PS_REGISTRY_INT_TYPE:
			data.int_value = from->data.real_value;
			break;
		case PS_REGISTRY_REAL_TYPE:
			data.real_value = from->data.int_value;
			break;
		case PS_REGISTRY_BOOL_TYPE:
			data.bool_value = from->data.bool_value;
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

ps_registry_value_class::~ps_registry_value_class()
{
	if (entry_type == PS_REGISTRY_STRING_TYPE)
		delete data.string_value;
}

////////////////////METHODS FOR PS_REGISTRY_ENTRY

ps_registry_entry_class::ps_registry_entry_class(std::string _name, ps_registry_value_class *value)
{
	name = _name;
	value = new ps_registry_value_class(value);
}
ps_registry_entry_class::~ps_registry_entry_class()
{
	delete value;
}
