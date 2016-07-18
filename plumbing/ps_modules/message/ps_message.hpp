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
#include "ps_common.h"
#include "topics.h"

class ps_message : public ps_root_class {

public:
    ps_message();
	~ps_message();
    
	int				serialized_length;

	virtual int	get_serialized_message(void *buff, int buff_length);

	virtual ps_result_enum publish();
    
private:

    
};

#endif	/* _PS_MESSAGE_H */

