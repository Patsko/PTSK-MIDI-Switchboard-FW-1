/*
 * SW Timer
 *
 */

#ifndef SW_TIMER_H // include guard
#define SW_TIMER_H

#include <ez8.h>
#include "zhal.h"

/*
 * Application defines
 */

#define MAX_SW_TIMERS   10

/*
 * Typedefs
 */

typedef struct {
    uint16_t Ticks;
    uint16_t InitialTick;
} SW_Timer_t;

/*
 * Function prototypes
 */

void SW_Timer_Init (SW_Timer_t * timer_p, uint16_t ticks);
bool_t SW_Timer_Is_Timed_Out (SW_Timer_t * timer_p);
void SW_Timer_Blocking_Delay (uint16_t ticks_to_wait);


#endif // SW_TIMER_H
