//
//  ps_packet_serial_class.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_packet_serial_class.hpp"

#define DEBUGPRINT(...)

//message checksum
#define start_checksum() checksum = 0
#define update_checksum(c) checksum += c
#define verify_checksum(c) ((checksum & 0xff) == c)

//adds a character to the message

parse_result_enum ps_packet_serial_class::parse_next_character(uint8_t c) {
    
	parse_result_enum parse_result = PARSE_OK;
    
    switch (parse_state) {
        case PARSE_STATE_UNINIT:
            packet_rx_drop_count = 0;
            packet_rx_success_count = 0;
            parse_error = 0;
            current_rx_seq = -1;
            parse_state = PARSE_STATE_IDLE;
        case PARSE_STATE_IDLE:
            if (c == STX_CHAR) {
                parse_state = PARSE_STATE_GOT_STX;
                start_checksum();
            }
            break;
        case PARSE_STATE_GOT_STX:
            packetLengthH = c;
            update_checksum(c);
            parse_state = PARSE_STATE_GOT_LENGTH1;
            break;
        case PARSE_STATE_GOT_LENGTH1:
            update_checksum(c);
            packetLength = (packetLengthH << 8) | c;
            parse_state = PARSE_STATE_GOT_LENGTH2;
            break;
        case PARSE_STATE_GOT_LENGTH2:
            update_checksum( c);
            rxmsg[packet_idx++] = c;
            update_checksum( c);
            if (packet_idx >= packetLength) {
                parse_state = PARSE_STATE_GOT_PAYLOAD;
            }
            break;
        case PARSE_STATE_GOT_PAYLOAD:
            // Checking checksum
            checksumH = c;
            parse_state = PARSE_STATE_GOT_CRC1;
            break;
        case PARSE_STATE_GOT_CRC1:
            if (checksum != ((checksumH << 8) | c)) {
                DEBUGPRINT("Parse: Parse error. Expected checksum %u, got %u.\n", (checksum & 0xff), c);
                parse_result = PARSE_CHECKSUM_ERROR;
                parse_error++;
                parse_state = PARSE_STATE_IDLE;
                if (c == STX_CHAR) {
                    parse_state = PARSE_STATE_GOT_STX;
                    packetLength = 0;
                    start_checksum();
                }
            } else {
                parse_result = PARSED_MESSAGE;
                packet_rx_success_count++;
                parse_state = PARSE_STATE_IDLE;
            }
        default:
            break;
    }
    return parse_result;
}

void ps_packet_serial_class::reset_parse_status() {
    parse_state 			= PARSE_STATE_UNINIT;
    parse_result			= PARSE_OK;
    packet_rx_success_count	= 0;	///< Received packets
    packet_rx_drop_count	= 0;	///< Number of packet drops
    checksum				= 0;	///< Running checksum
    parse_error				= 0;	///< Number of parse errors
    packet_idx				= 0;	///< Index in current packet
    current_rx_seq			= 0;	///< Sequence number of last packet received
    packetLength            = 0;
}

uint16_t ps_packet_serial_class::calculate_checksum(uint8_t *packet, size_t length)
{
    int i;
    int checksum = 0;
    
    for (i=0; i<length; i++)
    {
        checksum += packet[i];
    }
    return (checksum & 0xffff);
}
