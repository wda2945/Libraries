/* 
 * File:   ps_syslog_ios.hpp
 * Author: martin
 *
 * Message-related data initialized in ps_messages.cpp
 *
 * Created on April 19, 2014, 11:09 AM
 */

#ifndef _PS_SYSLOG_IOS_H
#define	_PS_SYSLOG_IOS_H

#include "syslog/ps_syslog.hpp"

typedef void (new_syslog_message_t)(ps_syslog_message_t *);

class ps_syslog_ios : public ps_syslog_class {

public:
    ps_syslog_ios(new_syslog_message_t *nsm);
	~ps_syslog_ios();

	void message_handler(void *msg, int length);
    new_syslog_message_t *new_syslog_message;
};

#endif	/* _PS_SYSLOG_IOS_H */

