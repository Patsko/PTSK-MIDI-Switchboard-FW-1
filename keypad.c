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

typedef enum {
    KPD_INIT = 0,
    KPD_ROW_OUTPUT_LOW,
    KPD_INPUT_CHECK,
    KPD_ROW_OUTPUT_HIGH
} Keypad_Status_t;


static const Keypad_Pins_t Keypad_Pins = {KEYPAD_GPIO_ROWS_STRUCT, KEYPAD_GPIO_COLUMNS_STRUCT};

static struct {
    Keypad_Status_t Status;
    uint8_t CurrentRow;
    SW_Timer_t Timer;
} Keypad;

static struct {
    Keypad_Button_Status_t Status;
    void (* Callback) (uint8_t row, uint8_t column, Keypad_Transition_t status);
    Keypad_Button_Config_t Config;
    SW_Timer_t Timer;
} Button [KEYPAD_MAX_ROWS][KEYPAD_MAX_COLUMNS];


/*
 * Keypad_Init
 */
static void Keypad_Init (void) {
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

    Keypad.CurrentRow = 0;
}


/*
 * Keypad_Config_Button
 * row: 0 to KEYPAD_MAX_ROWS
 * column: 0 to KEYPAD_MAX_COLUMNS
 */
bool_t Keypad_Config_Button (uint8_t row, uint8_t column, void (* callback) (uint8_t row, uint8_t column, Keypad_Transition_t status), Keypad_Button_Config_t config) {
    uint8_t status = 0;

    if ((row < KEYPAD_MAX_ROWS) && (column < KEYPAD_MAX_COLUMNS)) {
        Button[row][column].Callback = callback;
        Button[row][column].Config = config;
        status = 1;
    }

    return (status);
}

/*
 * Keypad_Check_Button_Status
 */
static void Keypad_Check_Button_Status (const uint8_t row, const uint8_t column, const uint8_t input) {
    bool_t is_pressed;

    if (Button[row][column].Config.Type == KEYPAD_BTN_NORMALLY_OPEN) {
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

    switch (Button[row][column].Status) {
    case BTN_IDLE:
        if (is_pressed) {
            SW_Timer_Init(&Button[row][column].Timer, KEYPAD_DEBOUNCE_TICKS);
            Button[row][column].Status = BTN_PRESSED_DEBOUNCE;
        }
        break;
    case BTN_PRESSED_DEBOUNCE:
        if (is_pressed) {
            if (SW_Timer_Is_Timed_Out(&Button[row][column].Timer)) {
                Button[row][column].Status = BTN_PRESSED;
                if ((Button[row][column].Config.Mode == KEYPAD_BTN_PRESSED_ONLY) || (Button[row][column].Config.Mode == KEYPAD_BTN_PRESSED_OR_RELEASED)) {
                    Button[row][column].Callback(row, column, KEYPAD_BTN_PRESSED);
                }
            }
        } else {
            Button[row][column].Status = BTN_IDLE;
        }
        break;
    case BTN_PRESSED:
        if (is_pressed == FALSE) {
            SW_Timer_Init(&Button[row][column].Timer, KEYPAD_DEBOUNCE_TICKS);
            Button[row][column].Status = BTN_IDLE_DEBOUNCE;
        }
        break;
    case BTN_IDLE_DEBOUNCE:
        if (is_pressed == FALSE) {
            if (SW_Timer_Is_Timed_Out(&Button[row][column].Timer)) {
                Button[row][column].Status = BTN_IDLE;
                if ((Button[row][column].Config.Mode == KEYPAD_BTN_RELEASED_ONLY) || (Button[row][column].Config.Mode == KEYPAD_BTN_PRESSED_OR_RELEASED)) {
                    Button[row][column].Callback(row, column, KEYPAD_BTN_RELEASED);
                }
            }
        } else {
            Button[row][column].Status = BTN_PRESSED;
        }
        break;
    default:
        Button[row][column].Status = BTN_IDLE;
        break;
    }
}



/*
 * Keypad_Task
 */
void Keypad_Task (void) {
    uint8_t column;
    uint8_t current_btn;
    uint8_t input;

    switch (Keypad.Status) {
    case KPD_INIT:
    default:
        Keypad_Init();
        Keypad.Status = KPD_ROW_OUTPUT_LOW;
        break;
    case KPD_ROW_OUTPUT_LOW:
        ZHAL_GPIO_Reset_Output (Keypad_Pins.Output[Keypad.CurrentRow].Port, Keypad_Pins.Output[Keypad.CurrentRow].Pin);
        SW_Timer_Init(&Keypad.Timer, 2);
        Keypad.Status = KPD_INPUT_CHECK;
        break;
    case KPD_INPUT_CHECK:
        if (SW_Timer_Is_Timed_Out(&Keypad.Timer)) {
            for (column = 0; column < KEYPAD_MAX_COLUMNS; column++) {
                input = ZHAL_GPIO_Read_Input(Keypad_Pins.Input[column].Port, Keypad_Pins.Input[column].Pin);
                Keypad_Check_Button_Status(Keypad.CurrentRow, column, input);
            }
            Keypad.Status = KPD_ROW_OUTPUT_HIGH;
        }
        break;
    case KPD_ROW_OUTPUT_HIGH:
        ZHAL_GPIO_Set_Output (Keypad_Pins.Output[Keypad.CurrentRow].Port, Keypad_Pins.Output[Keypad.CurrentRow].Pin);

        Keypad.CurrentRow++;
        if (Keypad.CurrentRow >= KEYPAD_MAX_ROWS) {
            Keypad.CurrentRow = 0;
        }

        Keypad.Status = KPD_ROW_OUTPUT_LOW;
        break;
    }
}
