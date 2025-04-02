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

#define KEYPAD_MAX_ROWS         3   // rows are outputs
#define KEYPAD_MAX_COLUMNS      3   // columns are inputs
#define KEYPAD_MAX_BUTTONS      (KEYPAD_MAX_ROWS * KEYPAD_MAX_COLUMNS)

#define KEYPAD_DEBOUNCE_TICKS       100
#define KEYPAD_KEPT_PRESSED_TICKS   3000

#define KEYPAD_GPIO_ROWS_STRUCT                                     \
    {                                                               \
         ROW0_PORT, ROW0_PIN,                                       \
         ROW1_PORT, ROW1_PIN,                                       \
         ROW2_PORT, ROW2_PIN,                                       \
    }

#define KEYPAD_GPIO_COLUMNS_STRUCT                                  \
    {                                                               \
         COLUMN0_PORT, COLUMN0_PIN,                                 \
         COLUMN1_PORT, COLUMN1_PIN,                                 \
         COLUMN2_PORT, COLUMN2_PIN,                                 \
    }


/*
 * Typedefs
 */

typedef enum {
    KEYPAD_COLUMN = 1,
    KEYPAD_ROW
} Keypad_Matrix_t;

typedef enum {
    KEYPAD_BTN_PRESSED = 0,
    KEYPAD_BTN_RELEASED,
    KEYPAD_BTN_KEPT_PRESSED
} Keypad_Transition_t;

typedef enum {
    KEYPAD_BTN_PRESSED_ONLY = 0,
    KEYPAD_BTN_RELEASED_ONLY,
    KEYPAD_BTN_PRESSED_OR_RELEASED
} Keypad_Button_Mode_t;

typedef enum {
    KEYPAD_BTN_NORMALLY_OPEN = 0,
    KEYPAD_BTN_NORMALLY_CLOSED
} Keypad_Button_Type_t;

typedef struct {
    Keypad_Button_Mode_t Mode       :2;
    Keypad_Button_Type_t Type       :1;
} Keypad_Button_Config_t;

/*
 * Function prototypes
 */

bool_t Keypad_Config_Button (uint8_t row, uint8_t column, void (* callback) (uint8_t row, uint8_t column, Keypad_Transition_t status), Keypad_Button_Config_t config);
void Keypad_Task (void);




#endif // KEYPAD_H
