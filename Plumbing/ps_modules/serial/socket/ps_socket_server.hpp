//
//  ps_socket_server.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_socket_server_hpp
#define ps_socket_server_hpp

#include <thread>

extern bool ping_response_received;

class ps_socket_server {
	int listen_port_number;
	int listen_socket;

	bool ping_response_received;
	char *pingTarget;

	std::thread *listenThread;
	std::thread *pingThread;
	std::thread *broadcastThread;


public:

	ps_socket_server(int _listen_port_number, const char *ping_target);
	~ps_socket_server();

	void ServerListenThreadMethod();
	void ServerPingThreadMethod();
	void ServerBroadcastThreadMethod();
};

#endif /* ps_socket_server_hpp */
