/*
 * Memory
 *
 */

#ifndef MEMORY_H // include guard
#define MEMORY_H

#include <ez8.h>
#include "zhal.h"

/*
 * Application defines
 */

#define MEMORY_MAX_BYTES    (ZHAL_SPI_FIFO_SIZE - 4)

/*
 * Typedefs
 */


/*
 * Function prototypes
 */

void Memory_Init ();
bool_t Memory_Read_Data (const uint16_t address, const uint8_t data[], const uint8_t bytes);
bool_t Memory_Write_Data (const uint16_t address, const uint8_t data[], const uint8_t bytes);
void Memory_Task ();
bool_t Memory_Close ();

#endif // MEMORY_H
