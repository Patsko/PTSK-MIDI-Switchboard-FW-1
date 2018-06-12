/*
 * Memory
 *
 * Provides interface to a SPI memory, through SPI driver
 */

#include <ez8.h>
#include <stdio.h>
#include "zhal.h"
#include "bsp.h"
#include "memory.h"
#include "memory_fm25l16.h"


static void Memory_SPI_Callback ();

typedef enum {
    MEM_STAT_UNINITIALIZED = 0,
    MEM_STAT_IDLE,
    MEM_STAT_READ_START,
    MEM_STAT_READING,
    MEM_STAT_READ_DUMP_USELESS_DATA,
    MEM_STAT_READ_FINISHED,
    MEM_STAT_WRITE_START,
    MEM_STAT_WRITE_ENABLING,
    MEM_STAT_WRITE_ADDRESS,
    MEM_STAT_WRITE_READY,
    MEM_STAT_WRITE_DATA,
    MEM_STAT_WRITING
} Memory_Status_t;

struct {
    Memory_Status_t Status;
    uint8_t Bytes;
    uint16_t Address;
} Memory;

static const ZHAL_SPI_Driver_Config_t Memory_SPI_Driver_Config = {
    CS_MEM_PORT,
    CS_MEM_PIN,
    Memory_SPI_Callback,
    NULL
};


/*
 * Memory_SPI_Callback
 */
static void Memory_SPI_Callback () {

    switch (Memory.Status) {
    case MEM_STAT_READING:
        Memory.Status = MEM_STAT_READ_DUMP_USELESS_DATA;
        break;
    case MEM_STAT_WRITE_ENABLING:
        Memory.Status = MEM_STAT_WRITE_ADDRESS;
        break;
    case MEM_STAT_WRITING:
    default:
        Memory.Status = MEM_STAT_IDLE;
        break;
    }
}


/*
 * Memory_Init
 */
void Memory_Init () {

    ZHAL_GPIO_Set_Output(CS_MEM_PORT, CS_MEM_PIN);
    ZHAL_SPI_Driver_Init();
    Memory.Status = MEM_STAT_IDLE;
}

/*
 * Memory_Read_Start
 * Starts the reading process, returning TRUE when
 */
bool_t Memory_Read_Start (uint8_t bytes, uint16_t address) {
    bool_t status = FALSE;

    if (Memory.Status == MEM_STAT_IDLE) {
        if (bytes < MEMORY_MAX_BYTES) {
            Memory.Bytes = bytes;
        } else {
            Memory.Bytes = MEMORY_MAX_BYTES;
        }
        Memory.Address = address;
        Memory.Status = MEM_STAT_READ_START;

        status = TRUE;
    }
    return (status);
}

/*
 * Memory_Read_Data
 * Reads data
 */
bool_t Memory_Read_Data (const uint8_t data[]) {
    bool_t status = FALSE;

    if (Memory.Status == MEM_STAT_READ_FINISHED) {
        ZHAL_SPI_Driver_Get_Data(data, Memory.Bytes);
        Memory.Status = MEM_STAT_IDLE;
        status = TRUE;
    }
    return (status);
}

/*
 * Memory_Write_Start
 */
bool_t Memory_Write_Start (uint16_t address) {
    bool_t status = FALSE;

    if (Memory.Status == MEM_STAT_IDLE) {
        Memory.Address = address;
        Memory.Status = MEM_STAT_WRITE_START;

        status = TRUE;
    }
    return (status);
}

/*
 * Memory_Write_Data
 */
bool_t Memory_Write_Data (const uint8_t data[], const uint8_t bytes) {
    bool_t status = FALSE;

    if (Memory.Status == MEM_STAT_WRITE_READY) {
        ZHAL_SPI_Driver_Put_Data(data, bytes);
        ZHAL_SPI_Driver_Send_Data(&Memory_SPI_Driver_Config);
        Memory.Status = MEM_STAT_WRITING;
        status = TRUE;
    }
    return (status);
}

/*
 * Memory_Task
 * Must be called periodically from main application
 */
void Memory_Task () {
    uint8_t data;
    uint8_t i;
    uint8_t debug [7];

    switch (Memory.Status) {
    case MEM_STAT_UNINITIALIZED:
    case MEM_STAT_IDLE:
    default:
        break;

    case MEM_STAT_READ_START:
        data = MEM_OPC_READ_DATA;
#if 0
        if (ZHAL_SPI_Driver_Put_Data(&data, 1)) {
            Memory.Status = MEM_STAT_READING;
            // dump SPI driver data, if there is something unread into RX buffer
            i = ZHAL_SPI_Driver_Peek(NULL);
            if (i != 0) {
                ZHAL_SPI_Driver_Get_Data(NULL, i);
            }
            ZHAL_SPI_Driver_Put_Data(&Memory.Address, sizeof(Memory.Address));
            for (i = 0; (i < Memory.Bytes) && (i < MEMORY_MAX_BYTES); i++) {
                data = 0xFF;
                ZHAL_SPI_Driver_Put_Data(&data, 1);
            }
            ZHAL_SPI_Driver_Send_Data(&Memory_SPI_Driver_Config);
        }
#endif
        debug[0] = MEM_OPC_READ_DATA;
        debug[1] = (uint8_t) (Memory.Address >> 8);
        debug[2] = (uint8_t) (Memory.Address);
        debug[3] = 0xFF;
        debug[4] = 0xFF;
        debug[5] = 0xFF;
        debug[6] = 0xFF;
        if (ZHAL_SPI_Driver_Put_Data(&debug, 7)) {
            Memory.Status = MEM_STAT_READING;
            ZHAL_SPI_Driver_Send_Data(&Memory_SPI_Driver_Config);
        }
        break;
    case MEM_STAT_READING:
        break;
    case MEM_STAT_READ_DUMP_USELESS_DATA:
        Memory.Status = MEM_STAT_READ_FINISHED;
        ZHAL_SPI_Driver_Get_Data(NULL, 3);  // dump RX buffer data related to opcode and address sent (3 bytes)
        break;
    case MEM_STAT_READ_FINISHED:
        break;
    case MEM_STAT_WRITE_START:
        data = MEM_OPC_WRITE_ENABLE;
        if (ZHAL_SPI_Driver_Put_Data(&data, 1)) {
            Memory.Status = MEM_STAT_WRITE_ENABLING;
            ZHAL_SPI_Driver_Send_Data(&Memory_SPI_Driver_Config);
        }
        break;
    case MEM_STAT_WRITE_ENABLING:
        break;
    case MEM_STAT_WRITE_ADDRESS:
        data = MEM_OPC_WRITE_DATA;
        if (ZHAL_SPI_Driver_Put_Data(&data, 1)) {
            Memory.Status = MEM_STAT_WRITE_READY;
            ZHAL_SPI_Driver_Put_Data(&Memory.Address, sizeof(Memory.Address));
        }
        break;
    case MEM_STAT_WRITE_READY:
    case MEM_STAT_WRITING:
        break;
    }
}


/*
 * Memory_Close
 * Closes the memory module, returning TRUE when doing so
 */
bool_t Memory_Close () {
    bool_t status = FALSE;

    if (Memory.Status == MEM_STAT_IDLE) {
        Memory.Status = MEM_STAT_UNINITIALIZED;
    } else if ((Memory.Status = MEM_STAT_UNINITIALIZED) && (ZHAL_SPI_Driver_Close() == TRUE)) {
        status = TRUE;
    }

    return (status);
}


uint8_t Memory_Get_Status (uint8_t status) {
    switch (status) {
    case 0:
        return (Memory.Status);
        break;
    case 1:
        break;
    case 2:
        break;
    }
}
