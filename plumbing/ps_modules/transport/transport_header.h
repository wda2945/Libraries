//
//  transport_header.h
//  RobotMonitor
//
//  Created by Martin Lane-Smith on 7/6/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef transport_header_h
#define transport_header_h

typedef struct {
    uint8_t sequenceNumber;
    uint8_t lastReceivedSequenceNumber;
    uint8_t flags;
} ps_transport_header_t;

#endif /* transport_header_h */
