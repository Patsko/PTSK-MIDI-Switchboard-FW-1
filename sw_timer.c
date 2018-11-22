/*
 * SW Timer
 *
 */

#include <ez8.h>
#include <stdio.h>
#include "zhal.h"
#include "sw_timer.h"


/*
 * Timer_Init
 * Initializes a timer, with a timeout specified in ticks
 */
void SW_Timer_Init (SW_Timer_t * timer_p, uint16_t ticks) {

    timer_p->InitialTick = ZHAL_Systick_Get_Tick();
    timer_p->Ticks = ticks;
}

/*
 * SW_Timer_Is_Timed_Out
 * Returns TRUE if the defined timeout has passed
 */
bool_t SW_Timer_Is_Timed_Out (SW_Timer_t * timer_p) {
    uint16_t current_tick;
    uint16_t diff;
    bool_t timed_out = FALSE;

    if (timer_p != NULL) {
        current_tick = ZHAL_Systick_Get_Tick();
        if (current_tick >= timer_p->InitialTick) {
            diff = (current_tick - timer_p->InitialTick);
        } else {
            diff = (MAX_UINT16 - timer_p->InitialTick);
            diff += (current_tick + 1);
        }
        if (diff > timer_p->Ticks) {
            timed_out = TRUE;
        }
    }

    return (timed_out);
}


/*
 * Timer_Blocking_Delay
 * Blocking delay
 */
void SW_Timer_Blocking_Delay_1ms (uint16_t ticks_to_wait) {
    uint16_t final_tick;

    final_tick = ticks_to_wait + ZHAL_Systick_Get_Tick();

    // wait until final_tick is the same as ZHAL_Systick.Tick
    while (final_tick != ZHAL_Systick_Get_Tick()) {
#warning "Add a watchdog reset here"
    }

}


