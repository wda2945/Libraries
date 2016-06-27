/*
 * PubSubLinux.c
 *
 *	PubSub Services on Linux
 *
 *  Created on: Jan 19, 2014
 *      Author: martin
 */

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "ps_message.hpp"
#include "messages.h"

	ps_message::ps_message(char *message_name)
	{

	}
	ps_message::ps_message(char *message_name, void *message_struct)
	{

	}
	ps_message::ps_message(void *serialized_message)
	{

	}
	ps_message::~ps_message()
	{

	}

	int	ps_message::get_serialized_message(void *buff, int buff_length)
	{
		return 0;
	}

	ps_result_enum ps_message::publish()
	{
		return PS_OK;
	}
