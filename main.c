/*
 * 
 *
 */


#include <ez8.h>
#include <string.h>
#include "zhal.h"

#include "bsp.h"
#include "sw_timer.h"
#include "output_expander.h"
#include "memory.h"
#include "keypad.h"
#include "crosspoint_switch.h"
#include "debug.h"

#warning "Test"
#include "test.h"


void main () {
    ZHAL_Init();

    Crosspoint_Switch_Init();   // must be called before Memory_Init/Output_Expander_Init

    Output_Expander_Init();
    Output_Expander_Data(0xFF);

    Memory_Init();
    
    while (1){

        Output_Expander_Task();
        Memory_Task();
        Keypad_Task();
        Crosspoint_Switch_Task();
        Debug_Monitor_Task();

        TIMER_TEST();
        KEYPAD_TEST();
        APPLICATION_TEST();
    }
}







