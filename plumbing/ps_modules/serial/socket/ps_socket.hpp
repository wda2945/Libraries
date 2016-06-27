//
//  ps_socket.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_socket_hpp
#define ps_socket_hpp

#include "serial/ps_serial_class.hpp"

class ps_socket : public ps_serial_class {
protected:

	int rxSocket;
	int txSocket;
	
	virtual void action_error_callback(ps_serial_status_enum res);

public:
	ps_socket(char *name, int socket);
	virtual ~ps_socket();

    //send bytes
    ps_result_enum write_bytes(void *data, int length);

    //receive bytes
    bool data_available();

    int read_bytes(void *data, int length);
};

#endif /* ps_socket_server_hpp */
