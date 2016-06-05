//
//  ps_types.h
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_types_h
#define ps_types_h

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
//#include <sys/_types/_ssize_t.h>

typedef enum {
    PS_OK,
    PS_NAME_NOT_FOUND,
    PS_IO_INIT_FAIL,
    PS_IO_ERROR,
    PS_NO_MEMORY,
    PS_TASK_INIT_FAIL,
    PS_PARSE_ERROR,
    PS_NOT_IMPLEMENTED
} ps_result_enum;

#define PS_MAX_TRANSPORTS 20
#define PS_NAME_SIZE 32
#define PS_DEFAULT_MAX_PACKET 100
#define PS_DEFAULT_PRELOAD  100
#define PS_MAX_TOPIC_LIST 20


#endif /* ps_types_h */
