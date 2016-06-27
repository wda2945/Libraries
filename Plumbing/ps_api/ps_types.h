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

//response and error codes
typedef enum {
    PS_OK,
    PS_NOT_IMPLEMENTED,
    PS_NAME_NOT_FOUND,
    PS_NAME_EXISTS,
    PS_LENGTH_ERROR,
    PS_IO_ERROR,
	PS_INVALID_PARAMETER,
	PS_WRONG_DATA_TYPE,
//    PS_NO_MEMORY,
//    PS_TASK_INIT_FAIL,
//    PS_CHECKSUM_ERROR,
//	  PS_PARSE_ERROR,
//    PS_QUEUE_EMPTY,
//    PS_TIMEOUT,
//    PS_SOURCE_NOT_FOUND,
//    PS_UNKNOWN_LINK_NUMBER
} ps_result_enum;

typedef uint8_t ps_topic_id_t;

typedef uint16_t event_id_t;
typedef uint16_t condition_id_t;

#endif /* ps_types_h */
