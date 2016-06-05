//
//  ps_settings.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_settings_hpp
#define ps_settings_hpp

#include "ps_types.h"
#include "common/ps_root_class.hpp"

class ps_settings : public ps_root_class {
  

public:
    
    ps_result_enum set_string(char *name, char *value);
    
    ps_result_enum get_string(char *name, char *buffer, size_t length);
    
 
    ps_result_enum set_real(char *name, double value);
    
    ps_result_enum get_real(char *name, double *value);

   
    ps_result_enum set_integer(char *name, int32_t value);
    
    ps_result_enum get_integer(char *name, int32_t *value);

    
    ps_result_enum set_boolean(char *name, bool value);
    
    ps_result_enum get_boolean(char *name, bool *value);

};

#endif /* ps_settings_hpp */
