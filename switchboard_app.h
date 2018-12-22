/*
 * Switchboard application
 *
 */

#ifndef SWITCHBOARD_APP_H // include guard
#define SWITCHBOARD_APP_H

#include <ez8.h>
#include "zhal.h"

/*
 * Application defines
 */

#define SWBOARD_LEDS                    6

#define SWBOARD_PROGRAM_NAME_SIZE       10

#define SWBOARD_MAX_EFFECTS             6
#define SWBOARD_SIGNAL_ROUTE_MAX        32

#define SWBOARD_MAX_PROGRAMS            8
#define SWBOARD_PROGRAM_SIZE            ((SWBOARD_SIGNAL_ROUTE_MAX * 2) + SWBOARD_PROGRAM_NAME_SIZE + SWBOARD_LEDS)  // memory/SPI driver must be able to handle SWBOARD_PROGRAM_SIZE bytes
#define SWBOARD_PROGRAM_FRAM_ADDR(x)    (x * SWBOARD_PROGRAM_SIZE)    // each program have (SWBOARD_SIGNAL_ROUTE_MAX * 2) bytes

#define SWBOARD_CURR_PROGRAM_FRAM_ADDR

/*
 * Typedefs
 */


/*
 * Function prototypes
 */

void Switchboard_Task (void);
void Switchboard_Test (void);



#endif // SWITCHBOARD_APP_H
