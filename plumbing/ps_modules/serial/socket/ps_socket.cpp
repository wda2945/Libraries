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

#include "ps_socket.hpp"
#include "ps_socket_server.hpp"

ps_socket::ps_socket(char *name, int socket)
	: ps_serial_class(name)
{
	rxSocket = socket;
	txSocket = dup(socket);
}
ps_socket::~ps_socket()
{
	close(rxSocket);
	close(txSocket);
}

//send bytes
ps_result_enum ps_socket::write_bytes(void *data, int length)
{
	if (txSocket == 0) {
		action_error_callback(PS_SERIAL_OFFLINE);
		return PS_IO_ERROR;
	}

	ssize_t reply;
	do {
		reply = send(txSocket, data, length, MSG_NOSIGNAL);
	} while (reply == 0);

	if (reply < 0)
	{
		action_error_callback(PS_SERIAL_WRITE_ERROR);
	}

	return PS_IO_ERROR;
}

//receive bytes
bool ps_socket::data_available()
{
	if (rxSocket == 0) return false;

	uint8_t c;
	if (recv(rxSocket, &c, 1, MSG_PEEK) <= 0)
		return false;
	else
		return true;
}

int ps_socket::read_bytes(void *data, int length)
{
	if (rxSocket == 0) {
		action_error_callback(PS_SERIAL_OFFLINE);
		return -1;
	}

	int reply = recv(rxSocket, data, length, 0);
	if (reply <= 0)
	{
		action_error_callback(PS_SERIAL_READ_ERROR);
	}
	return reply;
}

void ps_socket::action_error_callback(ps_serial_status_enum res)
{
	if (error_callback) (error_callback)(callback_arg, this, res);
	delete this;
}
