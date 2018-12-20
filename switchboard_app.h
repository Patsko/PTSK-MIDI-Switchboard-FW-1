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

#define SWBOARD_MAX_EFFECTS             6
#define SWBOARD_SIGNAL_ROUTE_MAX        32      // memory/SPI driver must be able to handle (SWBOARD_SIGNAL_ROUTE_MAX * 2) bytes

#define SWBOARD_PROGRAM_FRAM_ADDR(x)    (x * (SWBOARD_SIGNAL_ROUTE_MAX * 2))    // each program have (SWBOARD_SIGNAL_ROUTE_MAX * 2) bytes


/*
 * Typedefs
 */


/*
 * Function prototypes
 */

void Switchboard_Task (void);



#endif // SWITCHBOARD_APP_H
