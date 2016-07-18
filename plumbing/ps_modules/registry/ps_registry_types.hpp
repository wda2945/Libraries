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
#include <string.h>
#include "common/ps_root_class.hpp"
#include "ps_api/ps_registry.h"
#include "ps_registry_message.h"

/////////////////// Observer class

class ps_observer_class {
public:
	ps_observer_class( ps_registry_callback_t *_callback, const void *_arg){
		observer_callback = _callback;
		arg = const_cast<void*>(_arg);
	}

	ps_registry_callback_t *observer_callback;
	void *arg;
};

/////////////////// Registry data value helpers

    ps_result_enum copy_data(ps_registry_struct_t& to, const ps_registry_struct_t& from);        //copy value only
    ps_result_enum copy_all_data(ps_registry_struct_t& to, const ps_registry_struct_t& from);    //copy max/min too

/////////////////////Registry entry struct

typedef struct  {
    ps_update_packet_t   entry;
    std::set<ps_observer_class*> observers;
} ps_registry_entry_t;

#endif /* ps_registry_types_hpp */
