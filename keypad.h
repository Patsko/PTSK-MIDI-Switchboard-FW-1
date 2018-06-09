/*
 * Keypad
 *
 */

#ifndef KEYPAD_H // include guard
#define KEYPAD_H

#include <ez8.h>
#include "zhal.h"

/*
 * Application defines
 */

#define KEYPAD_MAX_COLUMNS      3
#define KEYPAD_MAX_ROWS         3
#define KEYPAD_MAX_BUTTONS      (KEYPAD_MAX_COLUMNS * KEYPAD_MAX_ROWS)

#define KEYPAD_DEBOUNCE_TICKS   50

#define KEYPAD_OUTPUT_0         (BTN0_PORT, BTN0_PIN)
#define KEYPAD_OUTPUT_1         (BTN1_PORT, BTN1_PIN)
#define KEYPAD_OUTPUT_2         (BTN2_PORT, BTN2_PIN)

#define KEYPAD_INPUT_0          (BTN3_PORT, BTN3_PIN)
#define KEYPAD_INPUT_1          (BTN4_PORT, BTN4_PIN)
#define KEYPAD_INPUT_2          (BTN5_PORT, BTN5_PIN)

/*
 * Typedefs
 */

typedef enum {
    KEYPAD_COLUMN = 1,
    KEYPAD_ROW
} Keypad_Matrix_t;

typedef enum {
    KEYPAD_RISING_EDGE = 0,
    KEYPAD_FALLING_EDGE
} Keypad_Transition_t;

typedef enum {
    KEYPAD_BTN_RISING_ONLY,
    KEYPAD_BTN_FALLING_ONLY,
    KEYPAD_BTN_RISING_FALLING
} Keypad_Button_Mode_t;

/*
 * Function prototypes
 */

bool_t Keypad_Config_Button (uint8_t button, void (* callback) (uint8_t button, Keypad_Transition_t status), Keypad_Button_Mode_t mode);
void Keypad_Task (void);




#endif // KEYPAD_H
