//
//  ps_notify.h
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_notify_h
#define ps_notify_h

#include "ps_types.h"

//notifications client api

#ifdef __cplusplus
extern "C" {
#endif

//Raise a event
ps_result_enum ps_notify_create(const char *event_name);
ps_result_enum ps_notify_event(const char *event_name);

typedef void (ps_notify_observer_callback_t)(void *arg);
ps_result_enum ps_notify_set_observer(const char *event_name, ps_notify_observer_callback_t *callback, void *arg);

//Set a condition
ps_result_enum ps_condition_create(const char *condition_name);
ps_result_enum ps_set_condition(const char *condition_name);
ps_result_enum ps_cancel_condition(const char *condition_name);
ps_result_enum ps_test_condition(const char *condition_name);

typedef void (ps_condition_observer_callback_t)(void *arg, bool condition_set);

ps_result_enum ps_condition_set_observer(const char *condition_name, ps_condition_observer_callback_t *callback, void *arg);

#ifdef __cplusplus
}
#endif

#endif /* ps_notify_h */
