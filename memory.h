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
bool_t Memory_Read_Start (uint8_t bytes, uint16_t address);
bool_t Memory_Read_Data (const uint8_t data[]);
bool_t Memory_Write_Start (uint16_t address);
bool_t Memory_Write_Data (const uint8_t data[], const uint8_t bytes);
void Memory_Task ();
bool_t Memory_Close ();
uint8_t Memory_Get_Status ();

#endif // MEMORY_H
