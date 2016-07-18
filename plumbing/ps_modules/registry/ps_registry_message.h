//
//  ps_registry_message.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_registry_message_hpp
#define ps_registry_message_hpp

#include "ps_api/ps_registry.h"

typedef struct {
    char domain[REGISTRY_DOMAIN_LENGTH];
    char name[REGISTRY_NAME_LENGTH];
    ps_registry_struct_t value;
} ps_update_packet_t;

#endif /* ps_registry_message_hpp */
