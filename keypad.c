/*
 * SW Timer
 *
 */

#include <ez8.h>
#include <stdio.h>
#include "zhal.h"
#include "sw_timer.h"
#include "keypad.h"


static struct {
    struct {
        void (* Callback) (uint8_t button, Keypad_Transition_t status);
        Keypad_Button_Mode_t Mode;
    } Button[KEYPAD_MAX_BUTTONS];
    uint8_t Status;
} Keypad;


/*
 *
 */
bool_t Keypad_Config_Button (uint8_t button, void (* callback) (uint8_t button, Keypad_Transition_t status), Keypad_Button_Mode_t mode) {
    uint8_t status = 0;

    if (button < KEYPAD_MAX_BUTTONS) {
        Keypad.Button[button].Callback = callback;
        Keypad.Button[button].Mode = mode;
        status = 1;
    }

    return (status);
}


void Keypad_Task (void) {




}
