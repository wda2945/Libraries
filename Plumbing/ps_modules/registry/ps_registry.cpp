//
//  psRegistry.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include <string.h>
#include "ps_registry.hpp"

ps_registry& the_registry()
{
	static ps_registry the_registry;
	return the_registry;
}

ps_registry::ps_registry()
{

}

//copy a new entry into the registry only if it does not exist
ps_result_enum ps_registry::new_registry_entry(std::string name, ps_registry_value_class *new_value)
{
	auto pos = registry.find(name);
	if (pos == registry.end())
	{
		ps_registry_entry_class *new_entry = new ps_registry_entry_class(name, new_value);
		registry.insert(std::make_pair(name, new_entry));
	}
	else
	{
		return PS_NAME_EXISTS;
	}
	return PS_OK;
}
//copy value only into the registry
ps_result_enum ps_registry::set_registry_entry(std::string name, ps_registry_value_class *set_value)
{
	auto pos = registry.find(name);
	if (pos == registry.end())
	{
		return PS_NAME_NOT_FOUND;
	}
	else
	{
		return pos->second->value->copy_data(set_value);
	}
	return PS_OK;
}
//copy entry from the registry
ps_result_enum ps_registry::get_registry_entry(std::string name, ps_registry_value_class *get_value)
{
	auto pos = registry.find(name);
	if (pos == registry.end())
	{
		return PS_NAME_NOT_FOUND;
	}
	else
	{
		get_value->copy_data(pos->second->value);
	}
	return PS_OK;
}

ps_result_enum ps_registry::set_observer(std::string name, ps_registry_observer_callback_t *callback, void *arg)
{
	std::map<std::string, ps_registry_entry_class*>::iterator pos = registry.find(name);
	if (pos == registry.end())
	{
		return PS_NAME_NOT_FOUND;
	}
	else
	{
		ps_observer_class *obs = new ps_observer_class(callback, arg);

		ps_registry_entry_class *entry = pos->second;
		entry->observers.insert(obs);
	}
	return PS_OK;
}
//////////////////////C API

//////////////Adding new entries

ps_result_enum ps_registry_new_char(const char *name, const char *string_value){
	ps_registry_value_class value(string_value);
	return the_registry().new_registry_entry(std::string(name), &value);
}
ps_result_enum ps_registry_new_string(const char *name, std::string string_value){
	ps_registry_value_class value(string_value);
	return the_registry().new_registry_entry(std::string(name), &value);
}
ps_result_enum ps_registry_new_int(const char *name, int min, int max, int _value){
	ps_registry_value_class value(min, max, _value);
	return the_registry().new_registry_entry(std::string(name), &value);
}
ps_result_enum ps_registry_new_real(const char *name, float min, float max, float _value){
	ps_registry_value_class value(min, max, _value);
	return the_registry().new_registry_entry(std::string(name), &value);
}
ps_result_enum ps_registry_new_bool(const char *name, bool _value){
	ps_registry_value_class value( _value);
	return the_registry().new_registry_entry(std::string(name), &value);
}

/////////////Changing values of existing entries

ps_result_enum ps_registry_set_char(const char *name, const char *string_value){
	ps_registry_value_class value(string_value);
	return the_registry().set_registry_entry(std::string(name), &value);
}
ps_result_enum ps_registry_set_string(const char *name, std::string string_value){
	ps_registry_value_class value(string_value);
	return the_registry().set_registry_entry(std::string(name), &value);
}
ps_result_enum ps_registry_set_int(const char *name, int _value){
	ps_registry_value_class value(0, 0, _value);
	return the_registry().set_registry_entry(std::string(name), &value);
}
ps_result_enum ps_registry_set_real(const char *name, float _value){
	ps_registry_value_class value(0.0, 0.0, _value);
	return the_registry().set_registry_entry(std::string(name), &value);
}
ps_result_enum ps_registry_set_bool(const char *name, bool _value){
	ps_registry_value_class value( _value);
	return the_registry().set_registry_entry(std::string(name), &value);
}

////////////Getting current values

ps_result_enum ps_registry_get_char(const char *_name, char *buff, int len){
	ps_registry_value_class *value = new ps_registry_value_class();
	std::string name(_name);
	ps_result_enum reply = the_registry().get_registry_entry(name, value);

	if (reply == PS_OK)
	{
		if (value->entry_type == PS_REGISTRY_STRING_TYPE)
		{
			strncpy(buff, value->data.string_value->c_str(), len);
			delete value;
			return PS_OK;
		}
		else
		{
			delete value;
			return PS_WRONG_DATA_TYPE;
		}
	}
	delete value;
	return reply;
}

ps_result_enum ps_registry_get_string(const char *_name, std::string *_value){
	ps_registry_value_class *value = new ps_registry_value_class();
	std::string name(_name);
	ps_result_enum reply = the_registry().get_registry_entry(name, value);

	if (reply == PS_OK)
	{
		if (value->entry_type == PS_REGISTRY_STRING_TYPE)
		{
			_value = value->data.string_value;
			return PS_OK;
		}
		else
		{
			return PS_WRONG_DATA_TYPE;
		}
	}
	return reply;
}

ps_result_enum ps_registry_get_int(const char *_name,  int *buff){
	ps_registry_value_class *value = new ps_registry_value_class();
	std::string name(_name);
	ps_result_enum reply = the_registry().get_registry_entry(name, value);

	if (reply == PS_OK)
	{
		if (value->entry_type == PS_REGISTRY_INT_TYPE)
		{
			*buff = value->data.int_value;
			return PS_OK;
		}
		else
		{
			return PS_WRONG_DATA_TYPE;
		}
	}
	return reply;
}

ps_result_enum ps_registry_get_real(const char *_name, float *buff){
	ps_registry_value_class *value = new ps_registry_value_class();
	std::string name(_name);
	ps_result_enum reply = the_registry().get_registry_entry(name, value);

	if (reply == PS_OK)
	{
		if (value->entry_type == PS_REGISTRY_REAL_TYPE)
		{
			*buff = value->data.real_value;
			return PS_OK;
		}
		else
		{
			return PS_WRONG_DATA_TYPE;
		}
	}
	return reply;
}

ps_result_enum ps_registry_get_bool(const char *_name, bool *buff){
	ps_registry_value_class *value = new ps_registry_value_class();
	std::string name(_name);
	ps_result_enum reply = the_registry().get_registry_entry(name, value);

	if (reply == PS_OK)
	{
		if (value->entry_type == PS_REGISTRY_BOOL_TYPE)
		{
			*buff = value->data.bool_value;
			return PS_OK;
		}
		else
		{
			return PS_WRONG_DATA_TYPE;
		}
	}
	return reply;
}

ps_result_enum ps_registry_set_observer(const char *_name, ps_registry_observer_callback_t *callback, void *arg)
{
	std::string name(_name);
	return the_registry().set_observer(std::string(name), callback, arg);
}

