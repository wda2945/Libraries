//
//  ps_socket.hpp
//  RobotFramework
//
//  Connected stream socket
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_socket_hpp
#define ps_socket_hpp

#include "serial/ps_serial_class.hpp"

class ps_socket : public ps_serial_class {

public:
	ps_socket(const char *name, int socket);
    
    ~ps_socket();

    //send bytes
    ps_result_enum write_bytes(const void *data, int length);

    //receive bytes
    bool data_available();

    int read_bytes(void *data, int length);
    
protected:
    
    int rxSocket;
    int txSocket;
    
};

#endif /* ps_socket_server_hpp */
