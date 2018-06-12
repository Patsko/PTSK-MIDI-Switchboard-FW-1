/*
 * Memory
 * FM25L16 memory defines
 */

#ifndef MEMORY_25L16_H // include guard
#define MEMORY_25L16_H

#include <ez8.h>
#include "zhal.h"

/*
 * Application defines
 */

#define MEM_OPC_WRITE_ENABLE    0x06
#define MEM_OPC_WRITE_DISABLE   0x04
#define MEM_OPC_READ_STAT_REG   0x05
#define MEM_OPC_WRITE_STAT_REG  0x01
#define MEM_OPC_READ_DATA       0x03
#define MEM_OPC_WRITE_DATA      0x02

/*
 * Typedefs
 */


/*
 * Function prototypes
 */



#endif // MEMORY_25L16_H
