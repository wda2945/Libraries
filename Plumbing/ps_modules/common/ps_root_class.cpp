//
//  ps_root_class.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_root_class.hpp"

    void ps_root_class::add_data_observer(ps_root_class *ob)
    {
    	LOCK_MUTEX(observerMtx);
    	data_observers.push_back(ob);
    	UNLOCK_MUTEX(observerMtx);
    }
    void ps_root_class::add_event_observer(ps_root_class *eo)
    {
    	LOCK_MUTEX(observerMtx);
    	event_observers.push_back(eo);
    	UNLOCK_MUTEX(observerMtx);
    }

    void ps_root_class::pass_new_data(ps_root_class *src, const void *msg, int length)
    {
    	LOCK_MUTEX(observerMtx);
    	for (auto *ob : data_observers) ob->process_observed_data(src, msg, length);
    	UNLOCK_MUTEX(observerMtx);
    }

    void ps_root_class::notify_new_event(ps_root_class *src, int event)
    {
    	LOCK_MUTEX(observerMtx);
    	for (auto *ob : event_observers) ob->process_observed_event(src, event);
    	UNLOCK_MUTEX(observerMtx);
    }
