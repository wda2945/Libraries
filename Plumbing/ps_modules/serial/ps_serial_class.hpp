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

#include "common/ps_root_class.hpp"

typedef enum {
	PS_SERIAL_OFFLINE,
	PS_SERIAL_WRITE_ERROR,
	PS_SERIAL_READ_ERROR,
	PS_SERIAL_ONLINE
} ps_serial_status_enum;

class ps_serial_class;

typedef void (serial_error_callback_t)(void *, ps_serial_class *, ps_serial_status_enum);

class ps_serial_class : public ps_root_class {

public:
	ps_serial_class(char *name);
	virtual ~ps_serial_class(){}

    //send bytes
    virtual ps_result_enum write_bytes(void *data, int length) = 0;
    
    //receive bytes
    virtual bool data_available() = 0;
    virtual int read_bytes(void *data, int length) = 0;

    void set_error_callback(serial_error_callback_t *cb, void *arg);
    virtual ps_serial_status_enum get_serial_status();

protected:
	ps_serial_status_enum serial_status;

	serial_error_callback_t *error_callback;
	void *callback_arg;

	virtual void action_error_callback(ps_serial_status_enum res);

};

#endif /* ps_serial_class_hpp */
