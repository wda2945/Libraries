//
//  ps_registry.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_registry_hpp
#define ps_registry_hpp

#include "ps_types.h"
#include "common/ps_root_class.hpp"

class ps_registry : public ps_root_class {
  

public:
  
    //make a hash value from the provided string
    int get_hash_value(char *name);
    
    //asssign a serial number (from 0) in the selected named domain
    uint16_t assign_serial_number(char *domain);
    
    //regisdter or update a value
    ps_result_enum register_string(char *name, char *value);
    
    //looknup a value by name
    ps_result_enum lookup_string(char *name, char *buffer, size_t length);
    
};

#endif /* ps_registry_hpp */
