//
//  ps_socket_client.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_socket_client_hpp
#define ps_socket_client_hpp

#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>

#include "ps_socket.hpp"

typedef enum {
	PS_CLIENT_UNDEFINED,
	PS_CLIENT_SEARCHING,
	PS_CLIENT_CONNECTING,
	PS_CLIENT_CONNECT_ERROR,
	PS_CLIENT_CONNECTED,
	PS_CLIENT_LOST_CONNECTION
} ps_client_status_enum;

typedef union {
    uint8_t bytes[4];
    in_addr_t address;
} IPaddress_t;

class ps_socket_client  : public ps_socket {
public:

	ps_socket_client(const char *server_name, const char *ip_address, int port_number);
    
    ~ps_socket_client();

    bool isConnected(){return (connect_status == PS_CLIENT_CONNECTED);}
    
	void get_client_status_caption(char *buff, int len);
    
protected:
    
	char *server_name;
	char *ip_address;
	int port_number;
    std::thread *connect_thread;
	ps_client_status_enum connect_status;

	IPaddress_t ipAddress;
	char		ipAddressStr[30];

	void client_connect_thread_method();

	void connect_to_server();

    int connect_retry(int domain, int type, int protocol, const struct sockaddr *addr, socklen_t alen);
    
	void set_client_status(ps_client_status_enum stat);
    
    //intercepts socket errors
    void notify_new_event(ps_serial_status_enum res);

};

#endif /* ps_socket_client_hpp */
