//
//  ps_serial_rtos.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_serial_rtos_hpp
#define ps_serial_rtos_hpp

#include "ps_serial_class.hpp"

class ps_serial_rtos : public ps_serial_class {
    uint8_t     my_uart;
    uint16_t    my_baudrate;

public:
    
    ps_result_enum init(uint8_t uart, uint16_t baudrate, char *destination);
    
};

#endif /* ps_serial_rtos_hpp */
