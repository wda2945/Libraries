/*
 * ps_syslog.cpp
 *
 *	Syslog Services
 *
 *  Created on: Jan 19, 2014
 *      Author: martin
 */

#include <string.h>

#include "ps_common.h"
#include "ps_syslog.hpp"
#include "pubsub/ps_pubsub_class.hpp"

void _LogMessage(SysLogSeverity_enum _severity, const char *_message, const char *_file)
{
	const char *filecomponent = (strrchr(_file, '/') + 1);
	if (!filecomponent) filecomponent = _file;
	if (!filecomponent) filecomponent = "****";

	ps_syslog_message_t msg;

	msg.severity = _severity;
	strncpy(msg.source, filecomponent, PS_SOURCE_LENGTH);
	strncpy(msg.text, _message, PS_MAX_LOG_TEXT);

	//publish a copy
	the_broker().publish_system_packet(SYSLOG_PACKET, &msg, sizeof(ps_syslog_message_t));
}
