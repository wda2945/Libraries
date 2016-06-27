/* 
 * File:   ps_message.hpp
 * Author: martin
 *
 * Message class
 *
 */

#ifndef _PS_MESSAGE_H
#define	_PS_MESSAGE_H

#include "common/ps_root_class.hpp"

class ps_message : public ps_root_class {

public:
	virtual ps_message(const char *message_name);
	virtual ps_message(const char *message_name, void *message_struct);
	virtual ps_message(void *serialized_message);
	virtual ~ps_message();

	ps_topic_Id_t 	topic_Id;
	ps_message_Id_t message_Id;
	int 			QoS;
	int				serialized_length;

	virtual int	get_serialized_message(void *buff, int buff_length);

	virtual ps_result_enum publish();
};

#endif	/* _PS_MESSAGE_H */

