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
#include "ps_config.h"

//registry client api

typedef uint32_t ps_registry_serial_t;
typedef uint8_t ps_registry_flags_t;
typedef uint8_t ps_registry_datatype_t;

//Datatype of the registry value

#define PS_REGISTRY_UNKNOWN_TYPE 	0
#define PS_REGISTRY_INT_TYPE		1
#define PS_REGISTRY_REAL_TYPE 	 	2
#define PS_REGISTRY_TEXT_TYPE		3
#define PS_REGISTRY_BOOL_TYPE		4

//Usage flags

#define PS_REGISTRY_INVALID_FLAGS 0x0

#define PS_REGISTRY_SRC_WRITE 	0x1    	//owner only can write
#define PS_REGISTRY_OTHER_WRITE	0x2    	//owner only can write
#define PS_REGISTRY_ANY_WRITE 	0x3    	//any user can write
#define PS_REGISTRY_LOCAL		0x4    	//do not promulgate
#define PS_REGISTRY_PRIORITY	0x8		//promulgate immediately
#define PS_REGISTRY_READ_ONLY 	0x10		//read-only

#define PS_REGISTRY_DEFAULT (PS_REGISTRY_SRC_WRITE)

#ifdef __cplusplus

#include <string>

extern "C" {

    //Registry data struct
    typedef struct {
        ps_registry_serial_t serial {1};
        Source_t source {SRC_UNKNOWN};
        ps_registry_datatype_t datatype {PS_REGISTRY_UNKNOWN_TYPE};
        ps_registry_flags_t flags {PS_REGISTRY_DEFAULT};
        
        union {
            char string_value[REGISTRY_TEXT_LENGTH];
            struct {
                int int_min, int_max, int_value;
            };
            struct {
                float real_min, real_max, real_value;
            };
            bool bool_value;
        };
    } ps_registry_struct_t;
    
    //add new entry
    ps_result_enum ps_registry_add_new(const char *domain, const char *name,
                                       ps_registry_datatype_t type, ps_registry_flags_t flags);
    
    ps_result_enum ps_registry_set_new(const char *domain, const char *name,const ps_registry_struct_t data);
    
    //set existing entry
    ps_result_enum ps_registry_set(const char *domain, const char *name, const ps_registry_struct_t data);
    
    //get entry
    ps_result_enum ps_registry_get(const char *domain, const char *name, ps_registry_struct_t *data);

    //std::string convenience versions if c++
    ps_result_enum ps_registry_set_string(const char *domain, const char *name, std::string value);
    ps_result_enum ps_registry_get_string(const char *domain, const char *name, std::string *value);
    
#endif

    //update value
    ps_result_enum ps_registry_set_text(const char *domain, const char *name, const char *value);
    ps_result_enum ps_registry_set_int(const char *domain, const char *name,  int value);
    ps_result_enum ps_registry_set_real(const char *domain, const char *name, float value);
    ps_result_enum ps_registry_set_bool(const char *domain, const char *name, bool value);
    
    //read value
    ps_result_enum ps_registry_get_text(const char *domain, const char *name, char *buff, int len);
    ps_result_enum ps_registry_get_int(const char *domain, const char *name,  int *value);
    ps_result_enum ps_registry_get_real(const char *domain, const char *name, float *value);
    ps_result_enum ps_registry_get_bool(const char *domain, const char *name, bool *buff);
    
    //iteration and observing
    typedef void (ps_registry_callback_t)(const char *domain, const char *name, const void *arg);
    ps_result_enum ps_registry_set_observer(const char *domain, const char *name, ps_registry_callback_t *callback, void *arg);
    ps_result_enum ps_registry_iterate_domain(const char *domain, ps_registry_callback_t *callback, void *arg);
    
    //load from and save to file
    ps_result_enum load_registry(const char *path);
    ps_result_enum save_registry(const char *path, const char *domain);
    
    ps_registry_datatype_t  ps_registry_get_type(const char *domain, const char *name);
    ps_registry_flags_t     ps_registry_get_flags(const char *domain, const char *name);
    
    ps_result_enum          send_registry_sync();

#ifdef __cplusplus
}
#endif

#endif /* ps_registry_h */
