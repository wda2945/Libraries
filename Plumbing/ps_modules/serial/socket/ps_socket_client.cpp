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
//#include <stropts.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <signal.h>


#include <netdb.h>

#include "serial/socket/ps_socket_client.hpp"

#include "packet/serial_packet/linux/ps_packet_serial_linux.hpp"
#include "transport/linux/ps_transport_linux.hpp"
#include "network/ps_network.hpp"

ps_socket_client::ps_socket_client(const char *_server_name, const char *_ip_address, int _port_number) :
	ps_socket(_server_name, 0)
{
	server_name = strdup(_server_name);
	if (_ip_address) ip_address = strdup(_ip_address);
	port_number = _port_number;

	//create connect thread
    connect_thread = new std::thread([this](){client_connect_thread_method();});
}

ps_socket_client::~ps_socket_client()
{
    delete connect_thread;
}

void ps_socket_client::client_connect_thread_method()
{
	struct hostent *server;

    ps_packet_serial_linux *pkt = new ps_packet_serial_linux(this);
    ps_transport_linux *trns = new ps_transport_linux(pkt);
    the_network().add_transport_to_network(trns);
    
//    ps_serial_class::action_error_callback(PS_SERIAL_OFFLINE, false);

    PS_DEBUG("sock: connect thread started");
    
    while (1)
    {
        sleep(2);
        
        switch(connect_status)
        {
            case PS_CLIENT_CONNECTED:
                break;
            default:
                
                //first try hard wired IP address
                if (ip_address)
                {
                    PS_DEBUG("sock: connecting to %s", ip_address);
                    //parse ip addres
                    int b0, b1, b2, b3;
                    sscanf(ip_address, "%i.%i.%i.%i", &b0, &b1, &b2, &b3);
                    ipAddress.bytes[0] = b0;
                    ipAddress.bytes[1] = b1;
                    ipAddress.bytes[2] = b2;
                    ipAddress.bytes[3] = b3;
                    connect_to_server();
                }
                
                if (connect_status == PS_CLIENT_LOST_CONNECTION) continue;
                
                //next try Bonjour
                if (server_name)
                {
                    PS_DEBUG("sock: connecting to %s", server_name);
                    ipAddressStr[0] = 0;
                    
                    set_client_status(PS_CLIENT_SEARCHING);
                    
                    server = gethostbyname2(server_name, AF_INET);
                    if (server != NULL)
                    {
                        memcpy(ipAddress.bytes, server->h_addr, 4);
                        connect_to_server();
                    }
                }
                break;
        }
    }
}

#define MAXSLEEP 128
int ps_socket_client::connect_retry(int domain, int type, int protocol, const struct sockaddr *addr, socklen_t alen)
{
    int numsec, fd;
    
    for (numsec = 1; numsec < MAXSLEEP; numsec <<= 1)
    {
        if ((fd = socket(domain, type, protocol)) < 0)
        {
            return -1;
        }
        if (connect(fd, addr, alen) == 0)
        {
            //accepted
            return fd;
        }
        close(fd);
        
        //delay
        if (numsec <= MAXSLEEP/2)
            sleep(numsec);
    }
    return(-1);
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
    rxSocket = connect_retry(AF_INET, SOCK_STREAM, 0,
                (const struct sockaddr*) &serverSockAddress, sizeof(serverSockAddress));
    
    if (rxSocket < 0)
    {
        PS_ERROR("sock: connect error");
    	set_client_status(PS_CLIENT_CONNECT_ERROR);
    }
    else
    {
        PS_ERROR("sock: connected");
        //dup a socket for the send
        txSocket = dup(rxSocket);
    	set_client_status(PS_CLIENT_CONNECTED);
    }
}

void ps_socket_client::notify_new_event(ps_serial_status_enum res)
{
    switch(res)
    {
        case PS_SERIAL_WRITE_ERROR:
        case PS_SERIAL_READ_ERROR:
            set_client_status(PS_CLIENT_LOST_CONNECTION);
        case PS_SERIAL_OFFLINE:
            close(rxSocket);
            close(txSocket);
            txSocket = rxSocket = 0;
            ps_serial_class::notify_new_event(PS_SERIAL_OFFLINE);
            break;
        default:
            ps_serial_class::notify_new_event(res);
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
        ps_serial_class::notify_new_event(PS_SERIAL_OFFLINE);
	default:
		break;
	case PS_CLIENT_CONNECTED:
		ps_serial_class::notify_new_event(PS_SERIAL_ONLINE);
		break;
	case PS_CLIENT_LOST_CONNECTION:
		notify_new_event(PS_SERIAL_OFFLINE);
		break;
	}
}

void ps_socket_client::get_client_status_caption(char *buff, int len)
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
    case PS_CLIENT_LOST_CONNECTION:
		snprintf(buff, len, "Lost Connection");
		break;
	default:
		snprintf(buff, len, "Unknown");
		break;
	}
}
