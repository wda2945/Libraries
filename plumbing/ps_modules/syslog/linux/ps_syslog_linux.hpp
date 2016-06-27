/* 
 * File:   ps_syslog_linux.hpp
 * Author: martin
 *
 * Message-related data initialized in ps_messages.cpp
 *
 * Created on April 19, 2014, 11:09 AM
 */

#ifndef _PS_SYSLOG_LINUX_H
#define	_PS_SYSLOG_LINUX_H

#include <pthread.h>
#include "syslog/ps_syslog.hpp"
#include "queue/linux/ps_queue_linux.hpp"

class ps_syslog_linux : public ps_syslog_class {
	pthread_t thread;
	ps_queue_linux *log_print_queue;
	FILE *logfile;

public:
	ps_syslog_linux();
	~ps_syslog_linux();

	void logging_thread_method();
	void print_log_message(ps_syslog_message_t *log_msg);

	void message_handler(void *msg, int length);
};

#endif	/* _PS_SYSLOG_LINUX_H */

