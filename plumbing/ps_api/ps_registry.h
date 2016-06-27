//
//  ps_registry.h
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_registry_h
#define ps_registry_h

#include "ps_types.h"
#include <string>

//registry client api

#ifdef __cplusplus
extern "C" {
#endif

ps_result_enum ps_registry_new_char(const char *name, const char *value);
ps_result_enum ps_registry_new_string(const char *name, std::string value);
ps_result_enum ps_registry_new_int(const char *name, int min, int max, int value);
ps_result_enum ps_registry_new_real(const char *name, float min, float max, float value);
ps_result_enum ps_registry_new_bool(const char *name, bool value);

ps_result_enum ps_registry_set_char(const char *name, const char *value);
ps_result_enum ps_registry_set_string(const char *name, std::string value);
ps_result_enum ps_registry_set_int(const char *name,  int value);
ps_result_enum ps_registry_set_real(const char *name, float value);
ps_result_enum ps_registry_set_bool(const char *name, bool value);

ps_result_enum ps_registry_get_char(const char *name, char *buff, int len);
ps_result_enum ps_registry_get_string(const char *name, std::string *value);
ps_result_enum ps_registry_get_int(const char *name,  int *buff);
ps_result_enum ps_registry_get_real(const char *name, float *buff);
ps_result_enum ps_registry_get_bool(const char *name, bool *buff);

typedef void (ps_registry_observer_callback_t)(const char *name, void *arg);

ps_result_enum ps_registry_set_observer(const char *name, ps_registry_observer_callback_t *callback, void *arg);

#ifdef __cplusplus
}
#endif

#endif /* ps_registry_h */
