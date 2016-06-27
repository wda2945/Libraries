//
//  ps_registry.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_registry_hpp
#define ps_registry_hpp

#include <set>
#include <map>
#include <string>
#include "common/ps_root_class.hpp"
#include "ps_registry_types.hpp"

class ps_registry : public ps_root_class {
  
public:
	ps_result_enum new_registry_entry(std::string name, ps_registry_value_class *new_value);
	ps_result_enum set_registry_entry(std::string name, ps_registry_value_class *set_value);
	ps_result_enum get_registry_entry(std::string name, ps_registry_value_class *get_value);
	ps_result_enum set_observer(std::string name, ps_registry_observer_callback_t *callback, void *arg);

protected:
	ps_registry();

	std::map<std::string, ps_registry_entry_class*> registry;

	friend ps_registry& the_registry();
};

ps_registry& the_registry();

#endif /* ps_registry_hpp */
