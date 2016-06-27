//
//  ps_packet_serial_linux.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_common.h"
#include "ps_packet_serial_linux.hpp"
#include <string.h>

void *packet_serial_rx_thread_wrapper(void *arg);

ps_packet_serial_linux::ps_packet_serial_linux(ps_serial_class *_driver)
		: ps_packet_serial_class(_driver)
{
    int s = pthread_create(&thread, NULL, packet_serial_rx_thread_wrapper, (void*) this);
	if (s != 0)
	{
		PS_ERROR("packet: thread error %i\n", s);
	}
}

void *packet_serial_rx_thread_wrapper(void *arg)
{
	ps_packet_serial_linux *psl = (ps_packet_serial_linux*) arg;
	psl->packet_serial_rx_thread_method();
	//no return
	return 0;
}

void ps_packet_serial_linux::packet_serial_rx_thread_method()
{
    while (1)
    {
        unsigned char nextByte;
        int charsRead;
        parse_result_enum parseResult = PARSE_OK;
        
        reset_parse_status();
        
        do {
            charsRead = serial_driver->read_bytes(&nextByte, 1);
            if (charsRead == 1)
            {
                parseResult = parse_next_character(nextByte);
            }
        } while ((parseResult == PARSE_OK) && (charsRead >= 0));
        
        if (parseResult == PARSED_MESSAGE)
        {
            action_data_callback(rxmsg, packetLength);
        }
        else{
//            action_error_callback(PS_PARSE_ERROR);
        }
    }
}

//send packet
ps_result_enum ps_packet_serial_linux::send_packet(void *packet, int length)
{
    ps_packet_header_t 		packetHeader;
    ps_packet_checksum_t 	checksum;
    
    packetHeader.start   = STX_CHAR;
    packetHeader.lengthH = (length << 8) & 0xff;
    packetHeader.lengthL = length & 0xff;
    
    ps_result_enum res = serial_driver->write_bytes(packetHeader.header, sizeof(packetHeader.header));

    if (res != PS_OK) return res;
    
    serial_driver->write_bytes(packet, length);
    
    checksum = calculate_checksum((uint8_t*) packet, length);
    
    serial_driver->write_bytes((uint8_t*) &checksum, sizeof(checksum));
    
    return PS_OK;
}
