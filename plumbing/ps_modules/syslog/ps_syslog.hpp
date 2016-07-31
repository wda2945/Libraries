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
#include "ps_syslog_message.h"

class ps_syslog_class : public ps_root_class {
public:
    ps_syslog_class()  : ps_root_class(std::string("syslog")){}
    
    virtual void message_handler(ps_packet_source_t packet_source,
                                 ps_packet_type_t   packet_type,
                                 const void *msg, int length) override {}
private:
    //observer callbacks
    virtual void process_observed_data(ps_root_class *src, const void *msg, int length) override {}
    virtual void process_observed_event(ps_root_class *src, int event) override {}
};


#endif	/* _PS_SYSLOG_H */

