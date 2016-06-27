//
//  ps_registry_types.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_registry_types_hpp
#define ps_registry_types_hpp

#include <set>
#include <map>
#include <string>
#include "common/ps_root_class.hpp"

typedef uint8_t ps_registry_datatype_t;

const ps_registry_datatype_t PS_REGISTRY_STRING_TYPE 	= 1;
const ps_registry_datatype_t PS_REGISTRY_INT_TYPE 		= 2;
const ps_registry_datatype_t PS_REGISTRY_REAL_TYPE 	= 3;
const ps_registry_datatype_t PS_REGISTRY_BOOL_TYPE 	= 4;

/////////////////// Observer class

class ps_observer_class {
public:
	ps_observer_class(ps_registry_observer_callback_t *_callback, void *_arg){
		observer_callback = _callback;
		arg = _arg;
	}

	ps_registry_observer_callback_t *observer_callback;
	void *arg;
};

/////////////////// Registry data value class

class ps_registry_value_class {
public:
	ps_registry_value_class();
	ps_registry_value_class(std::string string_value);
	ps_registry_value_class(char *char_value);
	ps_registry_value_class(int min, int max, int value);
	ps_registry_value_class(float min, float max, float value);
	ps_registry_value_class(bool bool_value);
	ps_registry_value_class(ps_registry_value_class *from);	//copy constructor
	~ps_registry_value_class();

	ps_result_enum copy_data(ps_registry_value_class *from);

	ps_registry_datatype_t entry_type;

	union {
		std::string *string_value;
		struct {
			int int_min, int_max, int_value;
		};
		struct {
			float real_min, real_max, real_value;
		};
		bool bool_value;
	}data;
};

/////////////////////Registry entry class

class ps_registry_entry_class {
public:
	ps_registry_entry_class(std::string name, ps_registry_value_class *value);
	~ps_registry_entry_class();

	std::string name;
	ps_registry_value_class *value;
	std::set<ps_observer_class*> observers;
};

#endif /* ps_registry_types_hpp */
