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

ps_syslog_ios::ps_syslog_ios(new_syslog_message_t *nsm)
{
    new_syslog_message = nsm;
    the_broker().register_object(SYSLOG_PACKET, this);
}

ps_syslog_ios::~ps_syslog_ios()
{

}

void ps_syslog_ios::message_handler(void *msg, int length)
{
    ps_syslog_message_t *log_msg = (ps_syslog_message_t *) msg;
    (new_syslog_message)(log_msg);
    
    
//    const int MAX_MESSAGE = (PS_SOURCE_LENGTH + PS_MAX_LOG_TEXT + 20);
//    std::string severity;
//    
//    char printBuff[MAX_MESSAGE];
//    
//    const time_t now = time(NULL);
//    struct tm *timestruct = localtime(&now);
//    
//    switch (log_msg->severity) {
//        case SYSLOG_ROUTINE:
//        default:
//            severity = "R";
//            break;
//        case SYSLOG_INFO:
//            severity = "I";
//            break;
//        case SYSLOG_WARNING:
//            severity = "W";
//            break;
//        case SYSLOG_ERROR:
//            severity = "E";
//            break;
//        case SYSLOG_FAILURE:
//            severity = "F";
//            break;
//    }
//    //make sure we don't overflow
//    log_msg->text[PS_MAX_LOG_TEXT] = '\0';
//    log_msg->source[PS_SOURCE_LENGTH] = '\0';
//    
//    //remove newline
//    int len = (int) strlen(log_msg->text);
//    if (log_msg->text[len-1] == '\n') log_msg->text[len-1] = '\0';
//    
//    snprintf(printBuff, MAX_MESSAGE, "%02i:%02i:%02i %s@%s: %s\n",
//             timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec,
//             severity.c_str(), log_msg->source, log_msg->text);
}

