//
//  ps_root_class.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_root_class_hpp
#define ps_root_class_hpp

#include "ps_types.h"
#include <string>

//root class used by all platform nodes

class ps_root_class {

public:
    int tag = 0;
    char *name = nullptr;
    
    void set_node_tag();
    void set_node_name(const char *_name);
    
    virtual ps_result_enum set_string_parameter(const char *name, const char *value){return PS_NAME_NOT_FOUND;}
    virtual ps_result_enum get_string_parameter(const char *name, char *buffer, size_t length){return PS_NAME_NOT_FOUND;}
    virtual ps_result_enum set_real_parameter(const char *name, const double value){return PS_NAME_NOT_FOUND;}
    virtual ps_result_enum get_real_parameter(const char *name, double *value){return PS_NAME_NOT_FOUND;}
    virtual ps_result_enum set_integer_parameter(const char *name, const int32_t value){return PS_NAME_NOT_FOUND;}
    virtual ps_result_enum get_integer_parameter(const char *name, int32_t *value){return PS_NAME_NOT_FOUND;}
    virtual ps_result_enum set_boolean_parameter(const char *name, const bool value){return PS_NAME_NOT_FOUND;}
    virtual ps_result_enum get_boolean_parameter(const char *name, bool *value){return PS_NAME_NOT_FOUND;}

};

#endif /* ps_root_class_hpp */
