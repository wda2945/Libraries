//
//  ps_serial_class.hpp
//  RobotFramework
//
//	Generic serial service
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_serial_class_hpp
#define ps_serial_class_hpp

#include <stdbool.h>
#include "common/ps_root_class.hpp"

typedef enum {
	PS_SERIAL_OFFLINE,
	PS_SERIAL_WRITE_ERROR,
	PS_SERIAL_READ_ERROR,
	PS_SERIAL_ONLINE
} ps_serial_status_enum;


class ps_serial_class : public ps_root_class {

public:
	ps_serial_class(const char *name);
	virtual ~ps_serial_class(){}

    //send bytes
    virtual ps_result_enum write_bytes(const void *data, int length) = 0;
    
    //receive bytes
    virtual bool data_available() = 0;
    virtual int read_bytes(void *data, int length) = 0;

    virtual ps_serial_status_enum get_serial_status();

protected:
	ps_serial_status_enum serial_status;

};

#endif /* ps_serial_class_hpp */
