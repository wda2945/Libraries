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

#include <thread>
using namespace std;

#include "syslog/ps_syslog.hpp"
#include "queue/linux/ps_queue_linux.hpp"

class ps_syslog_linux : public ps_syslog_class {
	thread *log_thread;
	ps_queue_linux *log_print_queue;
	FILE *logfile;

public:
	ps_syslog_linux();
	~ps_syslog_linux();

	void message_handler(ps_packet_source_t packet_source, ps_packet_type_t   packet_type, const void *msg, int length) override;

protected:

	void logging_thread_method();
	void print_log_message(ps_syslog_message_t *log_msg);

	DEFINE_MUTEX(printlogMtx);
};

#endif	/* _PS_SYSLOG_LINUX_H */

