//
//  ps_common_constants.h
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_common_constants_h
#define ps_common_constants_h

//Packet prefix defines
//Packet Types
const ps_packet_type_t PUBLISH_PACKET 	 		= 0;
const ps_packet_type_t SUBSCRIBE_PACKET 		= 1;
const ps_packet_type_t SEND_SUBS_PACKET 		= 2;
const ps_packet_type_t TRANSPORT_ADDED_PACKET 	= 3;
const ps_packet_type_t TRANSPORT_REMOVED_PACKET	= 4;
const ps_packet_type_t TRANSPORT_ONLINE_PACKET 	= 5;
const ps_packet_type_t TRANSPORT_OFFLINE_PACKET = 6;

const ps_packet_type_t SYSLOG_PACKET			= 7;
const ps_packet_type_t REGISTRY_PACKET			= 8;

const ps_packet_type_t PACKET_TYPES_COUNT 		= 10;

#endif /* ps_common_constants_h */
