/*
 * Button
 *
 */

#ifndef BUTTON_H // include guard
#define BUTTON_H

#include <ez8.h>
#include "zhal.h"

/*
 * Application defines
 */

#define BUTTON_MAX_QTY             4

#define BUTTON_DEBOUNCE_TICKS       100
#define BUTTON_KEPT_PRESSED_TICKS   3000

#define BUTTON_GPIO_STRUCT                                         \
    {                                                               \
        BTN0_PORT, BTN0_PIN,                                        \
        BTN1_PORT, BTN1_PIN,                                        \
        BTN2_PORT, BTN2_PIN,                                        \
        BTN4_PORT, BTN4_PIN,                                        \
    }


/*
 * Typedefs
 */


typedef enum {
    BUTTON_BTN_PRESSED = 0,
    BUTTON_BTN_RELEASED,
    BUTTON_BTN_KEPT_PRESSED
} Button_Transition_t;

typedef enum {
    BUTTON_BTN_PRESSED_ONLY = 0,
    BUTTON_BTN_RELEASED_ONLY,
    BUTTON_BTN_PRESSED_OR_RELEASED
} Button_Mode_t;

typedef enum {
    BUTTON_BTN_NORMALLY_OPEN = 0,
    BUTTON_BTN_NORMALLY_CLOSED
} Button_Type_t;

typedef struct {
    Button_Mode_t Mode       :2;
    Button_Type_t Type       :1;
} Button_Config_t;

/*
 * Function prototypes
 */

bool_t Button_Config (uint8_t button, void (* callback) (uint8_t button, Button_Transition_t status), Button_Config_t config);
void Button_Task (void);




#endif // BUTTON_H
