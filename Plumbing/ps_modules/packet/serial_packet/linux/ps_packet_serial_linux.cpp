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
#include <unistd.h>

ps_packet_serial_linux::ps_packet_serial_linux(ps_serial_class *_driver)
		: ps_packet_serial_class(_driver)
{
    packet_thread = new std::thread([this](){packet_serial_rx_thread_method();});
}

ps_packet_serial_linux::~ps_packet_serial_linux()
{
    delete packet_thread;
}

void ps_packet_serial_linux::packet_serial_rx_thread_method()
{
    while (1)
    {
        unsigned char nextByte = 0;
        
        while (nextByte != STX_CHAR)
        {
            serial_driver->read_bytes(&nextByte, 1);
        }
        
        ps_packet_header_t packetHeader;
        
        int charsRead = serial_driver->read_bytes(&packetHeader, sizeof(ps_packet_header_t));
        
        if (charsRead == sizeof(ps_packet_header_t))
        {
        	uint16_t length = packetHeader.length1;
            
            if ((length <= max_packet_size) && ((packetHeader.length1 ^ packetHeader.length2) == 0))
            {
                charsRead = serial_driver->read_bytes(&rxmsg, length);
                
                if (charsRead == length)
                {
                    if (packetHeader.csum == calculate_checksum((uint8_t*) rxmsg, length))
                    {
                        pass_new_data(rxmsg, length);
   
                    }
                    else
                    {
                        PS_ERROR("pkt: bad checksum: 0x%x read versus 0x%x",
                                 packetHeader.csum, calculate_checksum((uint8_t*) rxmsg, length));
                    }
                }
                else
                {
                    PS_ERROR("pkt: short read: %i versus %i", charsRead, length);
                }
            }
            else
            {
                PS_ERROR("pkt: bad message length: %i, check %i", packetHeader.length1, packetHeader.length2);
            }
        }
        else
        {
            PS_ERROR("pkt: bad header read: %i bytes", charsRead);
        }
    }
}

//send packet
static uint8_t stx_char = STX_CHAR;

ps_result_enum ps_packet_serial_linux::send_packet(const void *packet, int length)
{
    ps_packet_header_t 		packetHeader;
    
    packetHeader.length1 = length & 0xff;
    packetHeader.length2 = ~(length & 0xff);

    packetHeader.csum = calculate_checksum((uint8_t*) packet, length);

    ps_result_enum res = serial_driver->write_bytes(&stx_char, 1);
    if (res != PS_OK) return res;
    res = serial_driver->write_bytes(&packetHeader, sizeof(ps_packet_header_t));
    if (res != PS_OK) return res;
    
    res = serial_driver->write_bytes(packet, length);
    return res;
}
