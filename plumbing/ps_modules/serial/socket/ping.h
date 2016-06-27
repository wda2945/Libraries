//
//  ping.h
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ping_h
#define ping_h

typedef struct ip struct_ip;


#ifdef __cplusplus
extern "C" {
#endif

int pingServer(char *target);

#ifdef __cplusplus
}
#endif

#endif /* ping_h */
