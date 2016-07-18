/*
 * ps_syslog_linux.cpp
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
#include "ps_syslog_linux.hpp"
#include "pubsub/ps_pubsub_class.hpp"

ps_syslog_linux::ps_syslog_linux()
{
	char logfilepath[200];
	const time_t now = time(NULL);
	struct tm *timestruct = localtime(&now);

	snprintf(logfilepath, 200, "%s/SYS_%4i_%02i_%02i_%02i_%02i_%02i.log", LOGFILE_FOLDER,
			timestruct->tm_year + 1900, timestruct->tm_mon + 1, timestruct->tm_mday,
			timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec);

	logfile = fopen(logfilepath, "w");

	if (logfile == NULL)
	{
		logfile = stderr;
		fprintf(stderr, "syslog: fopen(%s) fail (%s)\n", logfilepath, strerror(errno));
	}
	else {
		fprintf(stderr, "syslog: Logfile opened on %s\n", logfilepath);
	}

	//queue for messages
	log_print_queue = new ps_queue_linux(sizeof(ps_syslog_message_t), SYSLOG_QUEUE_LENGTH);

	//start log print thread
    log_thread = new thread([this](){logging_thread_method();});

}

ps_syslog_linux::~ps_syslog_linux()
{
    delete log_thread;
}

void ps_syslog_linux::logging_thread_method()
{
	the_broker().register_object(SYSLOG_PACKET, this);

	while(1)
	{
		int len;
		ps_syslog_message_t *log_msg = (ps_syslog_message_t *) log_print_queue->get_next_message(0, &len);
		
		the_broker().publish_system_packet(SYSLOG_PACKET, log_msg, len);
		
		print_log_message(log_msg);
		log_print_queue->done_with_message(log_msg);
	}
}

void ps_syslog_linux::message_handler(void *msg, int length)
{
	if (length <= (int) sizeof(ps_syslog_message_t))	//sanity check
	{
		log_print_queue->copy_message_to_q(msg, length);
	}
}

void ps_syslog_linux::print_log_message(ps_syslog_message_t *log_msg)
{
	const int MAX_MESSAGE = (PS_SOURCE_LENGTH + PS_MAX_LOG_TEXT + 20);
	std::string severity;

	char printBuff[MAX_MESSAGE];

	const time_t now = time(NULL);
	struct tm *timestruct = localtime(&now);

	switch (log_msg->severity) {
	case SYSLOG_ROUTINE:
	default:
		severity = "R";
		break;
	case SYSLOG_INFO:
		severity = "I";
		break;
	case SYSLOG_WARNING:
		severity = "W";
		break;
	case SYSLOG_ERROR:
		severity = "E";
		break;
	case SYSLOG_FAILURE:
		severity = "F";
		break;
	}
	//make sure we don't overflow
	log_msg->text[PS_MAX_LOG_TEXT] = '\0';
	log_msg->source[PS_SOURCE_LENGTH] = '\0';

	//remove newline
	int len = (int) strlen(log_msg->text);
	if (log_msg->text[len-1] == '\n') log_msg->text[len-1] = '\0';

	snprintf(printBuff, MAX_MESSAGE, "%02i:%02i:%02i %s@%s: %s\n",
			timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec,
			severity.c_str(), log_msg->source, log_msg->text);

	fprintf(logfile, "%s", printBuff);
	fflush(logfile);

	printf("%s", printBuff);
}

