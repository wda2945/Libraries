//
//  psPic32UartDriver.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_serial_rtos.hpp"
#include "ps_config.h"
#include "Serial.h"

ps_result_enum ps_serial_rtos::init(uint8_t uart, uint16_t baudrate, char *destination)
{
    
    set_string_parameter("destination", destination);
    
    //initialize the uart
    
    if (!Serial_begin(uart, baudrate,
                      UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1,
                      UART_BROKER_BUFFER_SIZE, UART_BROKER_BUFFER_SIZE)) {
        
        return PS_IO_INIT_FAIL;
    }
    
    //create the offline timer
    uartTimeout[i] = xTimerCreate("Offline", // Just a text name, not used by the kernel.
                                  OFFLINE_TIMER_PERIOD, // The timer period in ticks.
                                  pdFALSE, // The timer will auto-reload itself when it expires.
                                  (void *) 0,
                                  uartOfflineTimerCallback
                                  );
    
    //create two tasks for each UART
    char taskName[configMAX_TASK_NAME_LEN];
    
    snprintf(taskName, configMAX_TASK_NAME_LEN, "U%i Tx %s", uart + 1, destination);
    /* Create the UART Tx task */
    if (xTaskCreate(psUARTTxTask, /* The function that implements the task. */
                    (signed char *) taskName, /* The text name assigned to the task - for debug only as it is not used by the kernel. */
                    UART_TASK_STACK_SIZE, /* The size of the stack to allocate to the task. */
                    (void *) i, /* The parameter passed to the task. */
                    UART_TASK_PRIORITY, /* The priority assigned to the task. */
                    NULL) /* The task handle is not required, so NULL is passed. */
        != pdPASS) {

        return -1;
    }
    
    snprintf(taskName, configMAX_TASK_NAME_LEN, "U%i Rx %s", uart + 1, destination);
    /* Create the UART Rx task */
    if (xTaskCreate(psUARTRxTask, /* The function that implements the task. */
                    (signed char *) taskName, /* The text name assigned to the task - for debug only as it is not used by the kernel. */
                    UART_TASK_STACK_SIZE, /* The size of the stack to allocate to the task. */
                    (void *) i, /* The parameter passed to the task. */
                    UART_TASK_PRIORITY, /* The priority assigned to the task. */
                    NULL) /* The task handle is not required, so NULL is passed. */
        != pdPASS) {

        return -1;
    }

    return PS_OK;
}
