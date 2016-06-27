//
//  ps_serial_linux.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "serial/linux_serial/ps_serial_linux.hpp"
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

ps_serial_linux::ps_serial_linux(const char *_name, const char *devicePath, unsigned int baudrate)
{
	set_node_name(_name);

    struct termios settings;
    
    //initialize UART
    FD = open(devicePath, O_RDWR | O_NOCTTY);
    
    tcgetattr(FD, &settings);
    
    //no processing
    settings.c_iflag = 0;
    settings.c_oflag = 0;
    settings.c_lflag = 0;
    settings.c_cflag = CLOCAL | CREAD | CS8;        //no modem, 8-bits
    
    //baudrate
    cfsetospeed(&settings, baudrate);
    cfsetispeed(&settings, baudrate);
    
    tcsetattr(FD, TCSANOW, &settings);
}

//receive bytes
int ps_serial_linux::read_bytes(void *data, int length)
{
    int readchars;
    
    do {
        readchars = read(FD, &data, length);
    } while (readchars == EAGAIN);
    
    return readchars;
}

//send bytes
ps_result_enum ps_serial_linux::write_bytes(void *data, int _length)
{
    int written;
    int length = _length;
    unsigned char *next = data;
    
    do {
        written = write(FD, next, length);
        if (written >= 0)
        {
            next += written;
            length -= written;
        }
        else if (written != EAGAIN)
        {
            return PS_IO_ERROR;
        }
    } while (length > 0);
    
    return PS_OK;
}

