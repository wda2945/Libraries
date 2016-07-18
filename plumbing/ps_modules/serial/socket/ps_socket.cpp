//
//  ps_socket.cpp
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

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "ps_socket.hpp"

ps_socket::ps_socket(const char *name, int socket)
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
ps_result_enum ps_socket::write_bytes(const void *data, int length)
{
	if (txSocket == 0) {
		notify_new_event(PS_SERIAL_OFFLINE);
		return PS_IO_ERROR;
	}

	ssize_t reply;
	do {
		reply = send(txSocket, data, length, 0);
	} while (reply == 0);

	if (reply < 0)
	{
		notify_new_event(PS_SERIAL_WRITE_ERROR);
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
	while (rxSocket == 0)
    {
		sleep(1);
	}

	int reply = (int)recv(rxSocket, data, length, 0);
	if (reply <= 0)
	{
		notify_new_event(PS_SERIAL_READ_ERROR);
	}
	return reply;
}

