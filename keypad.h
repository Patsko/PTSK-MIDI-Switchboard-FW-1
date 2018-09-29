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

#define KEYPAD_MAX_COLUMNS      3   // columns are inputs
#define KEYPAD_MAX_ROWS         3   // rows are outputs
#define KEYPAD_MAX_BUTTONS      (KEYPAD_MAX_COLUMNS * KEYPAD_MAX_ROWS)

#define KEYPAD_DEBOUNCE_TICKS   50

#define KEYPAD_PINS_ROWS_STRUCT                                     \
    {                                                               \
         ROW0_PORT, ROW0_PIN,                                       \
         ROW1_PORT, ROW1_PIN,                                       \
         ROW2_PORT, ROW2_PIN,                                       \
    }

#define KEYPAD_PINS_COLUMNS_STRUCT                                  \
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
    KEYPAD_RISING_EDGE = 0,
    KEYPAD_FALLING_EDGE
} Keypad_Transition_t;

typedef enum {
    KEYPAD_BTN_RISING_ONLY = 0,
    KEYPAD_BTN_FALLING_ONLY,
    KEYPAD_BTN_RISING_FALLING
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

void Keypad_Init (void);
bool_t Keypad_Config_Button (uint8_t button, void (* callback) (uint8_t button, Keypad_Transition_t status), Keypad_Button_Config_t config);
void Keypad_Task (void);




#endif // KEYPAD_H
