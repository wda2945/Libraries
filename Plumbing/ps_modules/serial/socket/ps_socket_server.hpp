//
//  ps_socket_server.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_socket_server_hpp
#define ps_socket_server_hpp

#include "pthread.h"

extern bool ping_response_received;

class ps_socket_server {
	int listen_port_number;
	int listen_socket;

	pthread_t listenThread;
	pthread_t pingThread;

public:

	ps_socket_server(int _listen_port_number, const char *ping_target);
	~ps_socket_server();

	void ServerListenThreadMethod();

};

#endif /* ps_socket_server_hpp */
