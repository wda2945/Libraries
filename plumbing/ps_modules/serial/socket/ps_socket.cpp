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
ps_result_enum ps_socket::write_bytes(const void *_data, int length)
{
	if (txSocket < 0) {
//		notify_new_event(this, PS_SERIAL_OFFLINE);
		return PS_IO_ERROR;
	}

    const uint8_t *data = static_cast<const uint8_t *>(_data);
    
	ssize_t reply;
	do {
		reply = send(txSocket, data, length, 0);
        if (reply >0)
        {
            data    += reply;
            length  -= reply;
        }
        
	} while (reply == 0 && length > 0);

	if (reply < 0)
	{
		notify_new_event(this, PS_SERIAL_WRITE_ERROR);
        return PS_IO_ERROR;
	}

	return PS_OK;
}

//receive bytes
bool ps_socket::data_available()
{
	if (rxSocket < 0) return false;

	uint8_t c;
	if (recv(rxSocket, &c, 1, MSG_PEEK) <= 0)
		return false;
	else
		return true;
}

int ps_socket::read_bytes(void *_data, int length)
{
	while (rxSocket < 0)
    {
		sleep(1);
	}

    uint8_t *data = static_cast<uint8_t *>(_data);
    int reply;
    do
    {
        reply = (int) recv(rxSocket, data, length, 0);
        if (reply > 0)
        {
            data += reply;
            length -= reply;
        }
    } while (reply >= 0 && length > 0);
    
    if (reply < 0)
    {
        notify_new_event(this, PS_SERIAL_READ_ERROR);
    }
    else if (reply == 0)
    {
        //end of file
        notify_new_event(this, PS_SERIAL_OFFLINE);
    }
    
	return reply;
}

void ps_socket::set_serial_status(ps_serial_status_enum s)
{
	notify_new_event(this, s);
}
