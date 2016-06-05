//
//  ps_pubsub_class.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "pubsub/ps_pubsub_class.hpp"
#include <string.h>

uint16_t ps_topic_hash(const char *topicName)
{
	char *str = (char *) topicName;
	unsigned int len  = strlen(str);
	unsigned int hash = 0;
	unsigned int x    = 0;
	unsigned int i    = 0;

	for(i = 0; i < len; str++, i++)
	{
		hash = (hash << 4) + (*str);
		if((x = hash & 0xF0000000L) != 0)
		{
			hash ^= (x >> 24);
		}
		hash &= ~x;
	}

	return (hash & 0xffff);
}
