/*
 * ps_syslog_IOS.cpp
 *
 *	Syslog Services
 *
 *  Created on: Jan 19, 2014
 *      Author: martin
 */

#include <errno.h>
#include <string.h>
#include <string>

#include "ps_common.h"
#include "ps_config.h"
#include "ps_syslog_ios.hpp"
#include "pubsub/ps_pubsub_class.hpp"

static ps_syslog_ios *syslog_ios;

void _LogMessage(SysLogSeverity_enum _severity, const char *_message, const char *_file)
{
	const char *filecomponent = (strrchr(_file, '/') + 1);
	if (!filecomponent) filecomponent = _file;
	if (!filecomponent) filecomponent = "****";

	ps_syslog_message_t msg;

	msg.severity = _severity;
	strncpy(msg.source, filecomponent, PS_SOURCE_LENGTH);
	strncpy(msg.text, _message, PS_MAX_LOG_TEXT);

    (syslog_ios->new_syslog_message)(&msg);
}

ps_syslog_ios::ps_syslog_ios(new_syslog_message_t *nsm)
{
    syslog_ios = this;
    new_syslog_message = nsm;
    the_broker().register_object(SYSLOG_PACKET, this);
}

ps_syslog_ios::~ps_syslog_ios()
{

}

void ps_syslog_ios::message_handler(ps_packet_source_t packet_source, ps_packet_type_t  packet_type,
                     const void *msg, int length)
{
    ps_syslog_message_t *log_msg = (ps_syslog_message_t *) msg;
    
    if (packet_type == SYSLOG_PACKET)
    {
        (new_syslog_message)(log_msg);
    }
}

