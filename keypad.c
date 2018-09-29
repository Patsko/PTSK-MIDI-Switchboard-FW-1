/*
 * SW Timer
 *
 */

#include <ez8.h>
#include <stdio.h>
#include "zhal.h"
#include "sw_timer.h"
#include "bsp.h"
#include "keypad.h"

typedef struct {
    struct {
        ZHAL_GPIO_Port_t Port;
        uint8_t Pin;
    } Output [KEYPAD_MAX_ROWS];
    struct {
        ZHAL_GPIO_Port_t Port;
        uint8_t Pin;
    } Input [KEYPAD_MAX_COLUMNS];
} Keypad_Pins_t;

typedef enum {
    BTN_IDLE = 0,
    BTN_PRESSED_DEBOUNCE,
    BTN_PRESSED,
    BTN_IDLE_DEBOUNCE
} Keypad_Button_Status_t;


static const Keypad_Pins_t Keypad_Pins = {KEYPAD_PINS_ROWS_STRUCT, KEYPAD_PINS_COLUMNS_STRUCT};

static struct {
    struct {
        Keypad_Button_Status_t Status;
        void (* Callback) (uint8_t button, Keypad_Transition_t status);
        Keypad_Button_Config_t Config;
        SW_Timer_t Timer;
    } Button[KEYPAD_MAX_BUTTONS];
    uint8_t Status;
    uint8_t CurrentRow;
} Keypad;


/*
 * Keypad_Init
 */
void Keypad_Init (void) {
    uint8_t i;
    ZHAL_GPIO_Config_t gpio_config = {
        ZHAL_GPIO_OUTPUT,
        ZHAL_GPIO_NORMAL,
        ENABLE,
        ENABLE,
        ENABLE,
        DISABLE
    };

    for (i = 0; i < KEYPAD_MAX_ROWS; i++) {
        ZHAL_GPIO_Config_Pin(Keypad_Pins.Output[i].Port, Keypad_Pins.Output[i].Pin, &gpio_config);
        ZHAL_GPIO_Set_Output (Keypad_Pins.Output[i].Port, Keypad_Pins.Output[i].Pin);
    }

    gpio_config.Direction = ZHAL_GPIO_INPUT;
    for (i = 0; i < KEYPAD_MAX_COLUMNS; i++) {
        ZHAL_GPIO_Config_Pin(Keypad_Pins.Input[i].Port, Keypad_Pins.Input[i].Pin, &gpio_config);
    }
}


/*
 * Keypad_Config_Button
 */
bool_t Keypad_Config_Button (uint8_t button, void (* callback) (uint8_t button, Keypad_Transition_t status), Keypad_Button_Config_t config) {
    uint8_t status = 0;

    if (button < KEYPAD_MAX_BUTTONS) {
        Keypad.Button[button].Callback = callback;
        Keypad.Button[button].Config = config;
        status = 1;
    }

    return (status);
}

/*
 * Keypad_Check_Button_Status
 */
static void Keypad_Check_Button_Status (uint8_t button, uint8_t input) {
    uint8_t is_pressed;

    if (Keypad.Button[button].Config.Type == KEYPAD_BTN_NORMALLY_OPEN) {
        if (input == 1) {
            is_pressed = 0;
        } else {
            is_pressed = 1;
        }
    } else {
        if (input == 0) {
            is_pressed = 0;
        } else {
            is_pressed = 1;
        }
    }

    switch (Keypad.Button[button].Status) {
    case BTN_IDLE:
        if (is_pressed) {
            SW_Timer_Init(&Keypad.Button[button].Timer, KEYPAD_DEBOUNCE_TICKS);
            Keypad.Button[button].Status = BTN_PRESSED_DEBOUNCE;
        }
        break;
    case BTN_PRESSED_DEBOUNCE:
        if (is_pressed) {
            if (SW_Timer_Is_Timed_Out(&Keypad.Button[button].Timer)) {
                Keypad.Button[button].Status = BTN_PRESSED;

            }
        } else {
            Keypad.Button[button].Status = BTN_IDLE;
        }
        break;
    case BTN_PRESSED:
        if (is_pressed == 0) {
            SW_Timer_Init(&Keypad.Button[button].Timer, KEYPAD_DEBOUNCE_TICKS);
            Keypad.Button[button].Status = BTN_IDLE_DEBOUNCE;
        }
        break;
    case BTN_IDLE_DEBOUNCE:
        if (is_pressed == 0) {
            if (SW_Timer_Is_Timed_Out(&Keypad.Button[button].Timer)) {
                Keypad.Button[button].Status = BTN_IDLE;
            }
        } else {
            Keypad.Button[button].Status = BTN_PRESSED;
        }
        break;
    default:
        Keypad.Button[button].Status = BTN_IDLE;
        break;
    }
}



/*
 * Keypad_Task
 */
void Keypad_Task (void) {
    uint8_t i;
    uint8_t current_btn;
    uint8_t input;

    switch (Keypad.Status) {
    case 0:

        break;
    case 1:
        ZHAL_GPIO_Reset_Output (Keypad_Pins.Output[Keypad.CurrentRow].Port, Keypad_Pins.Output[Keypad.CurrentRow].Pin);
        Keypad.Status++;
        break;
    case 2:
        for (i = 0; i < KEYPAD_MAX_COLUMNS; i++) {
            current_btn = (Keypad.CurrentRow * KEYPAD_MAX_COLUMNS) + i;
            input = ZHAL_GPIO_Read_Input(Keypad_Pins.Input[i].Port, Keypad_Pins.Input[i].Pin);
            Keypad_Check_Button_Status(current_btn, input);
        }
        break;
    case 3:
        break;
    case 4:
        break;

    }


}
