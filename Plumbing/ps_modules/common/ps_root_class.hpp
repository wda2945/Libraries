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
#include <vector>

using namespace std;

//root class used by all platform nodes

class ps_root_class {

public:
	ps_root_class(){}
	ps_root_class(char *_name){name = string(_name);}
	ps_root_class(string _name){name = _name;}

	int tag = 0;
    string name = "";
    
    void set_node_tag(){tag = ++last_tag;}
    void set_node_name(const char * _name){name = _name;}
    void set_node_name(std::string _name){name = _name;}

    //method to receive published messages in subclasses
    virtual void message_handler(ps_packet_source_t packet_source,
                                 ps_packet_type_t   packet_type,
                                 const void *msg, int length){}

    //add observers for received data and events
    virtual void add_data_observer(ps_root_class *ob) {data_observers.push_back(ob);}
    virtual void add_event_observer(ps_root_class *eo) {event_observers.push_back(eo);}
    
    //observer callbacks
    virtual void process_observed_data(ps_root_class *src, const void *msg, int length) {}
    virtual void process_observed_event(ps_root_class *src, int event) {}
    
protected:
    
    //action calls to observers
    virtual void pass_new_data(const void *msg, int length)
        {for (auto *ob : data_observers) ob->process_observed_data(this, msg, length);}
    
    virtual void notify_new_event(int event)
        {for (auto *ob : event_observers) ob->process_observed_event(this, event);}
 
    //action calls to observers with supplied source
    virtual void pass_new_data(ps_root_class *src, const void *msg, int length)
        {for (auto *ob : data_observers) ob->process_observed_data(src, msg, length);}
    
    virtual void notify_new_event(ps_root_class *src, int event)
        {for (auto *ob : event_observers) ob->process_observed_event(src, event);}

    
    //observer lists
    vector<ps_root_class*> data_observers;
    vector<ps_root_class*> event_observers;
 
    static ps_packet_source_t last_tag;
};

#endif /* ps_root_class_hpp */
