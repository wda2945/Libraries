//
//  ps_socket_server.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stropts.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include "ps_socket_server.hpp"
#include "ps_socket.hpp"
#include "packet/serial_packet/linux/ps_packet_serial_linux.hpp"
#include "transport/linux/ps_transport_linux.hpp"
#include "network/ps_network.hpp"

#include "ping.h"

void *ServerListenThreadWrapper(void *arg);
void *ServerPingThread(void *arg);

ps_socket_server::ps_socket_server(int _listen_port_number, const char *ping_target)
{
	listen_port_number = _listen_port_number;

	//create agent Listen thread
	int s = pthread_create(&listenThread, NULL, ServerListenThreadWrapper, (void*) this);
	if (s != 0) {
		LogError("server: Listen thread create failed: %s\n", strerror(s));
	}

	if (ping_target)
	{
		//create agent Ping thread
		s = pthread_create(&pingThread, NULL, ServerPingThread, (void*) ping_target);
		if (s != 0) {
			LogError("server: Ping thread create failed: %s\n", strerror(s));
		}
	}
}
ps_socket_server::~ps_socket_server()
{
	pthread_cancel(listenThread);
	if (pingThread) pthread_cancel(pingThread);

	close(listen_socket);
}
void *ServerListenThreadWrapper(void *arg)
{
	ps_socket_server *ss = (ps_socket_server*) arg;
	ss->ServerListenThreadMethod();
	//no return
	return 0;
}

//thread to ping the gateway every 10 seconds
bool ping_response_received;
void *ServerPingThread(void *arg)
{
	char *target = (char*) arg;
	while (1)
	{
		if (pingServer(target) > 0)
		{
			ping_response_received = true;
		}
		else
		{
			ping_response_received = false;
		}

		sleep(10);
	}
	return 0;
}

void ps_socket_server::ServerListenThreadMethod()
{
	struct sockaddr client_address;
	socklen_t client_address_len;

	PS_DEBUG("server: listen thread ready");

	while ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		PS_ERROR("server: listen socket() error: %s", strerror(errno));
		sleep(1);
	}
	int optval = 1;
	setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &optval, 4);

	PS_DEBUG("server: listen socket created");

	//bind socket address
	struct sockaddr_in my_address;
	memset(&my_address, 0, sizeof(my_address));
	my_address.sin_family = AF_INET;
	my_address.sin_port = htons(listen_port_number);
	my_address.sin_addr.s_addr = INADDR_ANY;

	while (bind(listen_socket, (struct sockaddr*) &my_address, sizeof(my_address)) == -1)
	{
		PS_ERROR("server: bind() error: %s", strerror(errno));

		if (errno == EADDRINUSE) sleep(10);

		sleep(1);
	}

	PS_DEBUG("server: listen socket ready");

	while(1)
	{
		//wait for connect
		while(listen(listen_socket, 10) != 0)
		{
			PS_ERROR("server: listen() error %s", strerror(errno));
			sleep(1);
			//ignore errors, just retry
		}

		client_address_len = sizeof(client_address);
		int acceptSocket = accept(listen_socket, &client_address, &client_address_len);

		if (acceptSocket >= 0)
		{
			char address[20];

			//print the address
			struct sockaddr_in *client_address_in = (struct sockaddr_in *) &client_address;

			uint8_t addrBytes[4];
			memcpy(addrBytes, &client_address_in->sin_addr.s_addr, 4);

			snprintf(address, 20, "%i.%i.%i.%i", addrBytes[0], addrBytes[1], addrBytes[2], addrBytes[3]);

			PS_DEBUG("server: connect from %s", address);

			ps_socket *sock = new ps_socket(address, acceptSocket);
			ps_packet_serial_linux *pkt = new ps_packet_serial_linux(sock);
			ps_transport_linux *trns = new ps_transport_linux(pkt);
			the_network().add_transport_to_network(trns);
		}
	}

}

