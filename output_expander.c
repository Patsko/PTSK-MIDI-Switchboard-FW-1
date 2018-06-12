/*
 * Output Expander
 *
 * Output expander based on a 74HC595 shift register, connected to the SPI
 * Currently compatible with only one shift register
 */

#include <ez8.h>
#include <stdio.h>
#include "zhal.h"
#include "bsp.h"

typedef enum {
    OUT_EXP_UNINITIALIZED = 0,
    OUT_EXP_IDLE,
    OUT_EXP_SEND_DATA
} OutExp_Status_t;


static struct {
    union {
        uint8_t Byte;
        struct {
            unsigned int Pin0   :1;
            unsigned int Pin1   :1;
            unsigned int Pin2   :1;
            unsigned int Pin3   :1;
            unsigned int Pin4   :1;
            unsigned int Pin5   :1;
            unsigned int Pin6   :1;
            unsigned int Pin7   :1;
        } Pins;
    } Output;
    OutExp_Status_t Status;
} OutExp;

static const ZHAL_SPI_Driver_Config_t SPI_Driver_Config = {
    CS_SHIFT_PORT,
    CS_SHIFT_PIN,
    NULL,
    NULL
};


/*
 * Output_Expander_Init
 */
void Output_Expander_Init () {

    ZHAL_GPIO_Set_Output(CS_SHIFT_PORT, CS_SHIFT_PIN);
    ZHAL_SPI_Driver_Init();
    OutExp.Status = OUT_EXP_IDLE;
}

/*
 * Output_Expander_Task
 * Must be called periodically from main application
 */
void Output_Expander_Task () {

    switch (OutExp.Status) {
    case OUT_EXP_UNINITIALIZED:
        break;
    case OUT_EXP_IDLE:
        break;
    case OUT_EXP_SEND_DATA:
        if (ZHAL_SPI_Driver_Put_Data(&OutExp.Output.Byte, 1)) {
            ZHAL_SPI_Driver_Send_Data(&SPI_Driver_Config);
            OutExp.Status = OUT_EXP_IDLE;
        }
        break;
    default:
        OutExp.Status = OUT_EXP_IDLE;
        break;
    }
}

/*
 * Output_Expander_Data
 */
void Output_Expander_Data (uint8_t data) {
    OutExp.Output.Byte = data;
    if (OutExp.Status != OUT_EXP_UNINITIALIZED) {
        OutExp.Status = OUT_EXP_SEND_DATA;
    }
}

/*
 * Output_Expander_Pin
 */
void Output_Expander_Pin (uint8_t pin, uint8_t status) {
    uint8_t data = 0x01;

    while (pin != 0) {
        pin--;
        data <<= 1;
    }
    if (status == 0) {
        data = ~data;
        OutExp.Output.Byte &= data;
    } else {
        OutExp.Output.Byte |= data;
    }
    if (OutExp.Status != OUT_EXP_UNINITIALIZED) {
        OutExp.Status = OUT_EXP_SEND_DATA;
    }
}

/*
 * Output_Expander_Close
 * Closes the output expander module, returning TRUE when doing so
 */
bool_t Output_Expander_Close () {
    bool_t status = FALSE;

    if (OutExp.Status == OUT_EXP_IDLE) {
        OutExp.Status = OUT_EXP_UNINITIALIZED;
    } else if ((OutExp.Status = OUT_EXP_UNINITIALIZED) && (ZHAL_SPI_Driver_Close() == TRUE)) {
        status = TRUE;
    }

    return (status);
}
