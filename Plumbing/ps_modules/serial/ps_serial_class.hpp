//
//  ps_serial_class.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_serial_class_hpp
#define ps_serial_class_hpp

#include "ps_types.h"
#include "common/ps_root_class.hpp"

class ps_serial_class : public ps_root_class {
    
public:
    
    //send bytes
    virtual ps_result_enum write_bytes(uint8_t *data, size_t length) = 0;
    
    //receive bytes
    bool data_available();
    
    virtual ssize_t read_bytes(uint8_t *data, size_t length) = 0;

};

#endif /* ps_serial_class_hpp */
