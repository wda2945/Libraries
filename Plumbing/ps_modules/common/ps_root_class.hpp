//
//  ps_root_class.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_root_class_hpp
#define ps_root_class_hpp

#include "ps_common.h"
#include "ps.h"
#include <string>

//using namespace std;

//root class used by all platform nodes

class ps_root_class {

public:
	ps_root_class(){}
	ps_root_class(char *_name){name = _name;}
	ps_root_class(std::string _name){name = _name;}

	ps_packet_source_t tag = 0;
    std::string name = "";
    
    void set_node_tag(){tag = ++last_tag;}
    void set_node_name(const char * _name){name = _name;}
    void set_node_name(std::string _name){name = _name;}

    //used to receive published messages in subclasses
    virtual void message_handler(void *msg, int length){}

protected:
    static ps_packet_source_t last_tag;
};

#endif /* ps_root_class_hpp */
