//
//  ps_serial_class_linux.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_serial_linux_hpp
#define ps_serial_linux_hpp

#include "../ps_serial_class.hpp"

class ps_serial_linux : public ps_serial_class {
    
    int FD;

public:
    
    ps_serial_linux(const char *_name, const char *devicePath, unsigned int baudrate);
    
    //send bytes
    ps_result_enum write_bytes(uint8_t *data, size_t length);
    
    //receive bytes
    bool data_available();
    
    ssize_t read_bytes(uint8_t *data, size_t length);
    
};

#endif /* ps_serial_linux_hpp */
