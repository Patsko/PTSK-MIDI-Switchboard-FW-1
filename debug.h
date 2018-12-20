/*
 * Debug Monitor
 *
 */

#ifndef DEBUG_MONITOR_H // include guard
#define DEBUG_MONITOR_H

#include <ez8.h>
#include "zhal.h"

/*
 * Application defines
 */

#define DEBUG_THROUGH_SERIAL        1
#define DEBUG_MESSAGE_ENABLE        1
#define DEBUG_MESSAGE_TIMEOUT       5000
#define DEBUG_SERIAL_BAUD_RATE      38400

/*
 * Typedefs
 */


/*
 * Function prototypes
 */

bool_t Debug_Message (const char * msg, const uint8_t size);
void Debug_Monitor_Task (void);



#endif // DEBUG_MONITOR_H
