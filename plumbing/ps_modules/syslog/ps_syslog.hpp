/* 
 * File:   ps_syslog.hpp
 * Author: martin
 *
 * Message-related data initialized in ps_messages.cpp
 *
 * Created on April 19, 2014, 11:09 AM
 */

#ifndef _PS_SYSLOG_H
#define	_PS_SYSLOG_H

#include "ps_common.h"
#include "common/ps_root_class.hpp"

typedef struct {
    uint8_t severity;
    char source[PS_SOURCE_LENGTH + 1];
    char text[PS_MAX_LOG_TEXT + 1];
} ps_syslog_message_t;

class ps_syslog_class : public ps_root_class {

};

#endif	/* _PS_SYSLOG_H */

