//
//  ps_packet_serial_linux.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_packet_serial_linux.hpp"
#include <string.h>

void *packet_serial_rx_thread(void *arg)
{
    ps_packet_serial_linux *packetDriver = (ps_packet_serial_linux*) arg;
    ps_serial_class *driver = packetDriver->driver;

    packetDriver->rxmsg = (unsigned char *) malloc(packetDriver->max_packet_size);
    
    if (packetDriver->rxmsg == NULL)
    {
        //
        return 0;
    }
    
    while (1)
    {
        unsigned char nextByte;
        parse_result_enum parseResult = PARSE_OK;
        
        packetDriver->reset_parse_status();
        
        do {
            ssize_t charsRead = driver->read_bytes(&nextByte, 1);
            if (charsRead == 1)
            {
                parseResult = packetDriver->parse_next_character(nextByte);
            }
        } while (parseResult == PARSE_OK);
        
        if (parseResult == PARSED_MESSAGE)
        {
            packetDriver->action_data_callback(packetDriver->rxmsg, packetDriver->packetLength);
        }
        else{
            packetDriver->action_error_callback(PS_PARSE_ERROR);
        }
    }
    
    return 0;
}

ps_packet_serial_linux::ps_packet_serial_linux(const char *_name, ps_serial_class *_driver, size_t _maxPacket)
{
	set_node_name(_name);

    max_packet_size = _maxPacket;
    driver = _driver;
    
    pthread_t thread;
    pthread_create(&thread, NULL, packet_serial_rx_thread, (void*) this);

}

//send packet
ps_result_enum ps_packet_serial_linux::send_packet(uint8_t *packet, size_t length)
{
    union {
        struct {
            uint8_t     start;
            uint8_t     lengthH;
            uint8_t     lengthL;
        };
        uint8_t header[3];
    }packetHeader;

    uint16_t checksum;
    
    packetHeader.start   = STX_CHAR;
    packetHeader.lengthH = (length << 8) & 0xff;
    packetHeader.lengthL = length & 0xff;
    
    driver->write_bytes(packetHeader.header, sizeof(packetHeader.header));
    
    driver->write_bytes(packet, length);
    
    checksum = calculate_checksum(packet, length);
    
    driver->write_bytes((uint8_t*) &checksum, sizeof(checksum));
    
    return PS_OK;
}
