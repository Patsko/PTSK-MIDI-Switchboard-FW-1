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
#define SWBOARD_SIGNAL_ROUTE_MAX        128

/*
 * Typedefs
 */


/*
 * Function prototypes
 */

void Switchboard_Task (void);



#endif // SWITCHBOARD_APP_H
