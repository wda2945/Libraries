//
//  ps_packet_serial_linux.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include "ps_common.h"
#include "ps_packet_serial_linux.hpp"

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
	const struct sigaction sa {SIG_IGN, 0, 0};
	sigaction(SIGPIPE, &sa, NULL);

    while (1)
    {
        int reply {0};
        unsigned char nextByte = 0;
        
//        PS_DEBUG("pkt: waiting for STX");
        
        do
        {
            reply = serial_driver->read_bytes(&nextByte, 1);
            
            if (reply <= 0)
            {
                usleep(100000);
            }
        }
        while (nextByte != STX_CHAR);
        
        ps_packet_header_t packetHeader;
        
        int charsRead = serial_driver->read_bytes(&packetHeader, sizeof(ps_packet_header_t));
        
        if (charsRead == sizeof(ps_packet_header_t))
        {
            uint16_t length = packetHeader.length1;
            
            if (length <= max_packet_size)
            {
                uint8_t length2 = ~packetHeader.length2;
                if ((packetHeader.length1 ^ length2) == 0)
                {
                    charsRead = serial_driver->read_bytes(&rxmsg, length);
                    
                    if (charsRead == length)
                    {
                        if (packetHeader.csum == calculate_checksum(rxmsg, length))
                        {
//                            PS_DEBUG("pkt: packet received");
                            pass_new_data(this, rxmsg, length);
                        }
                        else
                        {
                            PS_ERROR("pkt: bad checksum: 0x%x read versus 0x%x",
                                     packetHeader.csum, calculate_checksum( rxmsg, length));
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
                PS_ERROR("pkt: message too long: %i", packetHeader.length1);
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
    
//    PS_DEBUG("pkt: length = %i", (int)( length + sizeof(packetHeader)));
    
    packetHeader.length1 = length & 0xff;
    packetHeader.length2 = ~(length & 0xff);

    packetHeader.csum = calculate_checksum(static_cast<const uint8_t*>(packet), length);

    ps_result_enum res = serial_driver->write_bytes(&stx_char, 1);
    if (res != PS_OK) return res;
    res = serial_driver->write_bytes(&packetHeader, sizeof(ps_packet_header_t));
    if (res != PS_OK) return res;
    
    res = serial_driver->write_bytes(packet, length);
    return res;
}
