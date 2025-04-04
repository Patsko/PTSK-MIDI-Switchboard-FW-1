/*
 * Debug monitor
 *
 */

#include <ez8.h>
#include <stdio.h>

#include "zhal.h"
#include "sw_timer.h"
#include "debug.h"


static struct {
    uint8_t Status;
    SW_Timer_t Timer;
} DEBUG;



static void Debug_Init () {
#if DEBUG_THROUGH_SERIAL
    ZHAL_UART_Driver_Config_t uart_config = {
        DEBUG_SERIAL_BAUD_RATE,
        NULL,
        NULL
    };
    ZHAL_UART_Driver_Init(&uart_config);
#endif
}

static char Hex_to_Ascii (uint8_t value) {
    char result = 0;

    if (value < 10) {
        result = '0' + value;
    } else if (value < 16) {
        result = 'A' + (value - 10);
    }
    return (result);
}


/*
 * Debug_Message
 * Returns 1 if message was successfully sent
 */
bool_t Debug_Message (const char * msg, const uint8_t size) {

#if DEBUG_THROUGH_SERIAL
    if (ZHAL_UART_Driver_Put_Data (msg, size)) {
        ZHAL_UART_Driver_Send_Data();
        return (TRUE);
    } else {
        return (FALSE);
    }
#else
    return (TRUE);
#endif
}

/*
 * Debug_Send_Hex
 * Returns 1 if message was successfully sent
 */
bool_t Debug_Send_Hex (const uint8_t * value, const uint8_t size) {
    char msg[8];
    uint8_t msg_size = 0;
    uint8_t i;

#if DEBUG_THROUGH_SERIAL
    for (i = 0; i < size; i++) {
        msg[msg_size++] = Hex_to_Ascii((*value) >> 4);
        msg[msg_size++] = Hex_to_Ascii((*value) & 0x0F);
        value++;
    }

    return (Debug_Message(msg, msg_size));
#else
    return (TRUE);
#endif
}

/*
 * Debug_Monitor_Task
 * Must be called periodically from main application
 */
void Debug_Monitor_Task () {

    switch (DEBUG.Status) {
    case 0:
        Debug_Init();
#if DEBUG_MESSAGE_ENABLE
        SW_Timer_Init(&DEBUG.Timer, DEBUG_MESSAGE_TIMEOUT);
#endif
        DEBUG.Status++;
        break;
    case 1:
#if DEBUG_MESSAGE_ENABLE
        if ((SW_Timer_Is_Timed_Out(&DEBUG.Timer)) && (Debug_Message("Running\r\n", sizeof("Running\r\n") - 1))) {
            SW_Timer_Init(&DEBUG.Timer, DEBUG_MESSAGE_TIMEOUT);
        }
#endif
        break;
    }
}
