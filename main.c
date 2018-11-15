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

#warning "Test"
#include "test.h"


void main () {
    uint8_t i;
    uint8_t data;

    ZHAL_Init();

    Output_Expander_Init();
    Output_Expander_Data(0xFF);

    Memory_Init();
    
    while (1){

        Output_Expander_Task();
        Memory_Task();
        Keypad_Task();
        Crosspoint_Switch_Task();

        TIMER_TEST();
        KEYPAD_TEST();
        APPLICATION_TEST();
    }
}







