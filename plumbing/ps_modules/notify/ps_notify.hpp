//
//  ps_notify.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/22/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_notify_hpp
#define ps_notify_hpp

#include <vector>
#include <map>
#include <string>
#include <bitset>

#include "ps_common.h"
#include "ps_api/ps_notify.h"
#include "common/ps_root_class.hpp"

using namespace std;

typedef struct {
    ps_event_observer_callback_t *callback;
    void *callbackArg;
} ps_event_observer;

typedef struct {
    ps_condition_observer_callback_t *callback;
    void *callbackArg;
} ps_condition_observer;

typedef unsigned long long ps_conditions_message_t ;

class ps_notify_class : public ps_root_class {
public:
    ////////////////////////// EVENTS
    
    //Raise an event
    ps_result_enum ps_notify_event_method(ps_event_id_t event);
    
    ps_result_enum ps_add_event_observer_method(ps_event_id_t event,
                                                ps_event_observer_callback_t *callback, void *arg);
    
    ////////////////////////// CONDITIONS
    
    //Set/cancel a condition
    ps_result_enum ps_set_condition_method(ps_condition_id_t condition);
    ps_result_enum ps_cancel_condition_method(ps_condition_id_t condition);
    
    bool ps_test_condition_method(Source_t src, ps_condition_id_t condition);
    
    ps_result_enum ps_add_condition_observer_method(Source_t src, ps_condition_id_t condition, ps_condition_observer_callback_t *callback, void *arg);
    
protected:
    ps_notify_class();
    ~ps_notify_class();

    
    bitset<PS_CONDITIONS_COUNT> conditions[SRC_COUNT];
    
    map<ps_event_id_t, vector<ps_event_observer>> event_observers;
    map<ps_condition_id_t, vector<ps_condition_observer>> condition_observers[SRC_COUNT];
    
    void notify_event_observer(ps_event_id_t event);
    void notify_condition_observer(Source_t src, ps_condition_id_t condition, bool state);
    
    void process_conditions_packet(ps_packet_source_t src, ps_conditions_message_t msg);

    friend ps_notify_class& the_notifier();
};

ps_notify_class& the_notifier();


#endif /* ps_notify_hpp */
