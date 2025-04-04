/*
 * Button
 *
 */

#include <ez8.h>
#include <stdio.h>
#include "zhal.h"
#include "sw_timer.h"
#include "bsp.h"
#include "button.h"



typedef struct {
    struct {
        ZHAL_GPIO_Port_t Port;
        uint8_t Pin;
    } Pins [BUTTON_MAX_QTY];
} Button_Pins_t;

typedef enum {
    BTN_IDLE = 0,
    BTN_PRESSED_DEBOUNCE,
    BTN_PRESSED,
    BTN_IDLE_DEBOUNCE
} Button_Status_t;

typedef enum {
    BTN_INIT = 0,
    BTN_INPUT_CHECK,
} Button_Control_Status_t;


static const Button_Pins_t Button_Pins = {BUTTON_GPIO_STRUCT};

static struct {
    Button_Control_Status_t Status;
    SW_Timer_t Timer;
} Button_App;

static struct {
    Button_Status_t Status;
    void (* Callback) (uint8_t button, Button_Transition_t status);
    Button_Config_t Config;
    SW_Timer_t DebounceTimer;
    SW_Timer_t PressedTimer;
} Button [BUTTON_MAX_QTY];


/*
 * Button_Init
 */
static void Button_Init (void) {
    uint8_t i;
    ZHAL_GPIO_Config_t gpio_config = {
        ZHAL_GPIO_INPUT,
        ZHAL_GPIO_NORMAL,
        ENABLE,
        ENABLE,
        ENABLE,
        DISABLE
    };


    for (i = 0; i < BUTTON_MAX_QTY; i++) {
        ZHAL_GPIO_Config_Pin(Button_Pins.Pins[i].Port, Button_Pins.Pins[i].Pin, &gpio_config);
    }

    for (i = 0; i < BUTTON_MAX_QTY; i++) {
        Button[i].Callback = NULL;
    }

    // uses BTN3 as GND connection to buttons
    gpio_config.Direction = ZHAL_GPIO_OUTPUT;
    ZHAL_GPIO_Config_Pin(BTN3_PORT, BTN3_PIN, &gpio_config);        
    ZHAL_GPIO_Reset_Output (BTN3_PORT, BTN3_PIN);
}


/*
 * Button_Config_Button
 * button: 0 to BUTTON_MAX_QTY
 */
bool_t Button_Config (uint8_t button, void (* callback) (uint8_t button, Button_Transition_t status), Button_Config_t config) {
    uint8_t status = 0;

    if (button < BUTTON_MAX_QTY) {
        Button[button].Callback = callback;
        Button[button].Config = config;
        status = 1;
    }

    return (status);
}

/*
 * Button_Check_Button_Status
 */
static void Button_Check_Button_Status (const uint8_t button, const uint8_t input) {
    bool_t is_pressed;

    if (Button[button].Config.Type == BUTTON_BTN_NORMALLY_OPEN) {
        if (input == 1) {
            is_pressed = FALSE;
        } else {
            is_pressed = TRUE;
        }
    } else {
        if (input == 1) {
            is_pressed = TRUE;
        } else {
            is_pressed = FALSE;
        }
    }

    switch (Button[button].Status) {
    case BTN_IDLE:
        if (is_pressed) {
            SW_Timer_Init(&Button[button].DebounceTimer, BUTTON_DEBOUNCE_TICKS);
            Button[button].Status = BTN_PRESSED_DEBOUNCE;
        }
        break;
    case BTN_PRESSED_DEBOUNCE:
        if (is_pressed) {
            if (SW_Timer_Is_Timed_Out(&Button[button].DebounceTimer)) {
                Button[button].Status = BTN_PRESSED;
                SW_Timer_Init(&Button[button].PressedTimer, BUTTON_KEPT_PRESSED_TICKS);
                if ((Button[button].Config.Mode == BUTTON_BTN_PRESSED_ONLY) || (Button[button].Config.Mode == BUTTON_BTN_PRESSED_OR_RELEASED)) {
                    if (Button[button].Callback != NULL) {
                        Button[button].Callback(button, BUTTON_BTN_PRESSED);
                    }
                }
            }
        } else {
            Button[button].Status = BTN_IDLE;
        }
        break;
    case BTN_PRESSED:
        if (is_pressed == FALSE) {
            SW_Timer_Init(&Button[button].DebounceTimer, BUTTON_DEBOUNCE_TICKS);
            Button[button].Status = BTN_IDLE_DEBOUNCE;
        } else if (SW_Timer_Is_Timed_Out(&Button[button].PressedTimer)) {
            Button[button].Status = BTN_PRESSED;
            SW_Timer_Init(&Button[button].PressedTimer, BUTTON_KEPT_PRESSED_TICKS);
            if ((Button[button].Config.Mode == BUTTON_BTN_PRESSED_ONLY) || (Button[button].Config.Mode == BUTTON_BTN_PRESSED_OR_RELEASED)) {
                if (Button[button].Callback != NULL) {
                    Button[button].Callback(button, BUTTON_BTN_KEPT_PRESSED);
                }
            }
        }
        break;
    case BTN_IDLE_DEBOUNCE:
        if (is_pressed == FALSE) {
            if (SW_Timer_Is_Timed_Out(&Button[button].DebounceTimer)) {
                Button[button].Status = BTN_IDLE;
                if ((Button[button].Config.Mode == BUTTON_BTN_RELEASED_ONLY) || (Button[button].Config.Mode == BUTTON_BTN_PRESSED_OR_RELEASED)) {
                    if (Button[button].Callback != NULL) {
                        Button[button].Callback(button, BUTTON_BTN_RELEASED);
                    }
                }
            }
        } else {
            Button[button].Status = BTN_PRESSED;
        }
        break;
    default:
        Button[button].Status = BTN_IDLE;
        break;
    }
}



/*
 * Button_Task
 */
void Button_Task (void) {
    uint8_t button;
    uint8_t current_btn;
    uint8_t input;

    switch (Button_App.Status) {
    case BTN_INIT:
    default:
        Button_Init();

        SW_Timer_Init(&Button_App.Timer, 2);
        Button_App.Status = BTN_INPUT_CHECK;
        break;
    case BTN_INPUT_CHECK:
        if (SW_Timer_Is_Timed_Out(&Button_App.Timer)) {
            for (button = 0; button < BUTTON_MAX_QTY; button++) {
                input = ZHAL_GPIO_Read_Input(Button_Pins.Pins[button].Port, Button_Pins.Pins[button].Pin);
                Button_Check_Button_Status(button, input);
            }
            SW_Timer_Init(&Button_App.Timer, 2);
        }
        break;
    }
}
