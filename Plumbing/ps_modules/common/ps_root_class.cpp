//
//  ps_root_class.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "common/ps_root_class.hpp"
#include <string.h>

void ps_root_class::set_node_name(const char *_name)
{
	if (name) free(name);

	name = static_cast<char*> (malloc(strlen(_name) + 1));
	strcpy(name, _name);
}

void ps_root_class::set_node_tag()
{
	static int next_tag = 0;

	tag = ++next_tag;
}
