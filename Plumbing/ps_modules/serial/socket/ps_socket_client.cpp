//
//  ps_socket_client.cpp
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

#include "ps_socket_client.hpp"
#include "ps_socket.hpp"
#include "packet/serial_packet/linux/ps_packet_serial_linux.hpp"
#include "transport/linux/ps_transport_linux.hpp"
#include "network/ps_network.hpp"


void *ClientConnectThreadWrapper(void *arg);

ps_socket_client::ps_socket_client(const char *_server_name, const char *_ip_address, int _port_number)
	ps_root_class(_server_name)
{
	server_name = strdup(_server_name);
	if (_ip_address) ip_address = strdup(_ip_address);
	port_number = _port_number;

	ps_packet_serial_linux *pkt = new ps_packet_serial_linux(this, 0);
	ps_transport_linux *trns = new ps_transport_linux(pkt);
	the_network().add_transport_to_network(trns);

	ps_serial_class::action_error_callback(PS_SERIAL_OFFLINE);

	//create connect thread
	int s = pthread_create(&connectThread, NULL, ClientConnectThreadWrapper, (void*) this);
	if (s != 0) {
		LogError("client: Connect thread create failed: %s\n", strerror(s));
	}
}

ps_socket_client::~ps_socket_client()
{
	pthread_cancel(connectThread);
	close(rxSocket);
	close(txSocket);
}

void *ClientConnectThreadWrapper(void *arg)
{
	ps_socket_client *ss = (ps_socket_client*) arg;
	ss->client_connect_thread_method();
	//no return
	return 0;
}

void ps_socket_client::client_connect_thread_method()
{
	struct hostent *server;

	while (1)
	{
		sleep(2);

		switch(connect_status)
		{
		case PS_CLIENT_CONNECTED:
			break;
		default:
			ipAddressStr[0] = '/0';
			//First try Bonjour name
			if (server_name)
			{
				set_client_status(PS_CLIENT_SEARCHING);

				server = gethostbyname2(server_name, AF_INET);
				if (server != NULL)
				{
					memcpy(ipAddress.bytes, server->h_addr, 4);
					connect_to_server();
				}
			}

			if (connect_status == PS_CLIENT_LOST_CONNECTION) continue;

			//then try hard wired
			if (ip_address)
			{
				//parse ip addres
				sscanf(ip_address, "%i:%i:%i:%i", ipAddress.bytes[0], ipAddress.bytes[1], ipAddress.bytes[2], ipAddress.bytes[3]);
				connect_to_server();
			}
			break;
		}
	}
}


void ps_socket_client::connect_to_server()
{

    struct sockaddr_in serverSockAddress;

    sprintf(ipAddressStr, "%i.%i.%i.%i", ipAddress.bytes[0], ipAddress.bytes[1], ipAddress.bytes[2], ipAddress.bytes[3]);
    set_client_status(PS_CLIENT_CONNECTING);

    memset(&serverSockAddress, 0, sizeof(serverSockAddress));
    serverSockAddress.sin_len = sizeof(serverSockAddress);
    serverSockAddress.sin_family = AF_INET;
    serverSockAddress.sin_port = htons(port_number);
    serverSockAddress.sin_addr.s_addr = ipAddress.address;

    //create socket & connect
    rxSocket = connect_retry(AF_INET, SOCK_STREAM, 0, (const struct sockaddr*) &serverSockAddress, sizeof(serverSockAddress));

    if (rxSocket < 0)
    {
    	set_client_status(PS_CLIENT_CONNECT_ERROR);
    }
    else
    {
        //dup a socket for the send
        txSocket = dup(rxSocket);
    	set_client_status(PS_CLIENT_CONNECTED);
    }
}

void ps_socket_client::action_error_callback(ps_status_enum res)
{
	switch(res)
	{
	case PS_SERIAL_WRITE_ERROR:
	case PS_SERIAL_READ_ERROR:
		close(rxSocket);
		close(txSocket);
		set_client_status(PS_CLIENT_LOST_CONNECTION);
		break;
	default:
		ps_serial_class::action_error_callback(res);
		break;
	}
}

void ps_socket_client::set_client_status(ps_client_status_enum stat)
{
	connect_status = stat;

	switch(stat)
	{
	case PS_CLIENT_SEARCHING:
	case PS_CLIENT_CONNECTING:
	case PS_CLIENT_CONNECT_ERROR:
	default:
		break;
	case PS_CLIENT_CONNECTED:
		ps_serial_class::action_error_callback(PS_SERIAL_ONLINE);
		break;
	case PS_CLIENT_LOST_CONNECTION
		ps_serial_class::action_error_callback(PS_SERIAL_OFFLINE);
		break;
	}
}

void get_client_status_caption(char *buff, int len)
{
	switch(connect_status)
	{
	case PS_CLIENT_SEARCHING:
		snprintf(buff, len, "Searching...");
		break;
	case PS_CLIENT_CONNECTING:
		snprintf(buff, len, "Connecting to %s", ipAddressStr);
		break;
	case PS_CLIENT_CONNECT_ERROR:
		snprintf(buff, len, "Connect Error");
		break;
	case PS_CLIENT_CONNECTED:
		snprintf(buff, len, "Connected to %s", ipAddressStr);
		break;
	case PS_CLIENT_LOST_CONNECTION
		snprintf(buff, len, "Lost Connection");
		break;
	default:
		snprintf(buff, len, "Unknown");
		break;
	}
}
