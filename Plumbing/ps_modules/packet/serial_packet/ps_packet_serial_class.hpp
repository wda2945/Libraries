//
//  ps_packet_serial_class.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_packet_serial_class_hpp
#define ps_packet_serial_class_hpp

#include "packet/ps_packet_class.hpp"
#include "serial/ps_serial_class.hpp"

typedef union {
	struct {
		uint8_t     start;
		uint8_t     lengthH;
		uint8_t     lengthL;
	};
	uint8_t header[3];
}ps_packet_header_t;

typedef uint16_t ps_packet_checksum_t;

#define STX_CHAR 0x7f

typedef enum {
    PARSE_STATE_UNINIT=0,
    PARSE_STATE_IDLE,
    PARSE_STATE_GOT_STX,
    PARSE_STATE_GOT_LENGTH1,
    PARSE_STATE_GOT_LENGTH2,
    PARSE_STATE_GOT_PAYLOAD,
    PARSE_STATE_GOT_CRC1
} ps_parse_state_enum; ///< The state machine for the comm parser

typedef enum {
    PARSE_OK,
    PARSED_MESSAGE,
    PARSE_CHECKSUM_ERROR,
    PARSE_RESULT_COUNT
} parse_result_enum;

#define PS_PARSE_RESULT_NAMES {"Running", "Message", "Checksum"}

class ps_packet_serial_class : public ps_packet_class {

public:
	ps_packet_serial_class(ps_serial_class *_driver);
	virtual ~ps_packet_serial_class();
    
    parse_result_enum    parse_result;
    ps_parse_state_enum  parse_state;               ///< Parsing state machine
    uint16_t            checksum;                   ///< Running checksum
    uint8_t             checksumH;                  ///< 1st byte of trailing checksum
    uint16_t            parse_error;                ///< Number of parse errors
    uint16_t            packet_idx;                 ///< Index in current packet
    uint16_t            current_rx_seq;             ///< Sequence number of last packet received
    uint16_t            packetLength;
    uint8_t             packetLengthH;              ///< 1st byte of length
    
    ps_serial_class      *serial_driver;

    //parse an incoming packet
    parse_result_enum parse_next_character(uint8_t c);
    
    void reset_parse_status();
  
    uint16_t calculate_checksum(uint8_t *packet, int length);
    
    //send packet
    virtual ps_result_enum send_packet(void *packet, int length) = 0;
    
protected:

    void *rxmsg;

	void process_serial_error_callback(ps_serial_status_enum stat);

	friend void serial_error_callback(void *arg, ps_serial_class *psc, ps_serial_status_enum stat);

};

#endif /* ps_packet_serial_class_hpp */
