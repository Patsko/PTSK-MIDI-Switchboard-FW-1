/*
 * Output expander
 *
 */

#ifndef OUTPUT_EXPANDER_H // include guard
#define OUTPUT_EXPANDER_H

#include <ez8.h>
#include "zhal.h"

/*
 * Application defines
 */


/*
 * Typedefs
 */


/*
 * Function prototypes
 */

void Output_Expander_Init (void);
void Output_Expander_Task (void);
void Output_Expander_Data (uint8_t data);
void Output_Expander_Pin (uint8_t pin, uint8_t status);
bool_t Output_Expander_Close (void);


#endif // OUTPUT_EXPANDER_H
