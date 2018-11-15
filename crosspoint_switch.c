/*
 * Crosspoint Switch
 *
 * Controls a MT8816 crosspoint switch
 */

#include <ez8.h>
#include "zhal.h"
#include "bsp.h"
#include "crosspoint_switch.h"

typedef enum {
    OPEN = 0,
    CLOSED,
    TO_BE_OPENED,
    TO_BE_CLOSED
} Switch_State_t;

typedef enum {
    SW_STAT_UNINITIALIZED = 0,
    SW_STAT_IDLE,
    SW_STAT_WRITING
} Switch_Status_t;

static struct {
    Switch_State_t Switch[CROSSPOINT_X_SWITCHES][CROSSPOINT_Y_SWITCHES];
    uint8_t Status;
} Crosspoint_Switch;


static void Wait () {
    uint8_t count = 10;

    while (count != 0) {
        count--;
    }
}


/*
 * Crosspoint_Switch_Init
 */
void Crosspoint_Switch_Init () {
    const ZHAL_GPIO_Config_t gpio_config = {
        ZHAL_GPIO_OUTPUT,
        ZHAL_GPIO_NORMAL,
        DISABLE,
        DISABLE,
        DISABLE,
        DISABLE
    };

    ZHAL_GPIO_Config_Pin(CROSSPOINT_AX0_PORT, CROSSPOINT_AX0_PIN, &gpio_config);
    ZHAL_GPIO_Config_Pin(CROSSPOINT_AX1_PORT, CROSSPOINT_AX1_PIN, &gpio_config);
    ZHAL_GPIO_Config_Pin(CROSSPOINT_AX2_PORT, CROSSPOINT_AX2_PIN, &gpio_config);
    ZHAL_GPIO_Config_Pin(CROSSPOINT_AX3_PORT, CROSSPOINT_AX3_PIN, &gpio_config);
    ZHAL_GPIO_Config_Pin(CROSSPOINT_AY0_PORT, CROSSPOINT_AY0_PIN, &gpio_config);
    ZHAL_GPIO_Config_Pin(CROSSPOINT_AY1_PORT, CROSSPOINT_AY1_PIN, &gpio_config);
    ZHAL_GPIO_Config_Pin(CROSSPOINT_AY2_PORT, CROSSPOINT_AY2_PIN, &gpio_config);
    ZHAL_GPIO_Config_Pin(CROSSPOINT_DATA_PORT, CROSSPOINT_DATA_PIN, &gpio_config);
    ZHAL_GPIO_Config_Pin(CROSSPOINT_CS_PORT, CROSSPOINT_CS_PIN, &gpio_config);
    ZHAL_GPIO_Config_Pin(CROSSPOINT_CLK_PORT, CROSSPOINT_CLK_PIN, &gpio_config);

    ZHAL_GPIO_Reset_Output(CROSSPOINT_AX0_PORT, CROSSPOINT_AX0_PIN);
    ZHAL_GPIO_Reset_Output(CROSSPOINT_AX1_PORT, CROSSPOINT_AX1_PIN);
    ZHAL_GPIO_Reset_Output(CROSSPOINT_AX2_PORT, CROSSPOINT_AX2_PIN);
    ZHAL_GPIO_Reset_Output(CROSSPOINT_AX3_PORT, CROSSPOINT_AX3_PIN);
    ZHAL_GPIO_Reset_Output(CROSSPOINT_AY0_PORT, CROSSPOINT_AY0_PIN);
    ZHAL_GPIO_Reset_Output(CROSSPOINT_AY1_PORT, CROSSPOINT_AY1_PIN);
    ZHAL_GPIO_Reset_Output(CROSSPOINT_AY2_PORT, CROSSPOINT_AY2_PIN);
    ZHAL_GPIO_Reset_Output(CROSSPOINT_DATA_PORT, CROSSPOINT_DATA_PIN);
    ZHAL_GPIO_Reset_Output(CROSSPOINT_CS_PORT, CROSSPOINT_CS_PIN);
    ZHAL_GPIO_Reset_Output(CROSSPOINT_CLK_PORT, CROSSPOINT_CLK_PIN);

}


/*
 * Crosspoint_Switch_Control
 * From MT8816 data sheet: Data is presented to the memory on the DATA input.
 * Data is asynchronously written into memory whenever both the CS (Chip Select) and STROBE inputs are high and are latched on the falling edge of STROBE.
 * A logical “1” written into a memory cell turns the corresponding crosspoint switch on and a logical “0” turns the crosspoint off.
 */
static void Crosspoint_Switch_Control (const uint8_t x, const uint8_t y, const uint8_t status) {

    if ((x < CROSSPOINT_X_SWITCHES) && (y < CROSSPOINT_Y_SWITCHES)) {

        ZHAL_GPIO_Reset_Output(CROSSPOINT_AX0_PORT, CROSSPOINT_AX0_PIN);
        ZHAL_GPIO_Reset_Output(CROSSPOINT_AX1_PORT, CROSSPOINT_AX1_PIN);
        ZHAL_GPIO_Reset_Output(CROSSPOINT_AX2_PORT, CROSSPOINT_AX2_PIN);
        ZHAL_GPIO_Reset_Output(CROSSPOINT_AX3_PORT, CROSSPOINT_AX3_PIN);
        ZHAL_GPIO_Reset_Output(CROSSPOINT_AY0_PORT, CROSSPOINT_AY0_PIN);
        ZHAL_GPIO_Reset_Output(CROSSPOINT_AY1_PORT, CROSSPOINT_AY1_PIN);
        ZHAL_GPIO_Reset_Output(CROSSPOINT_AY2_PORT, CROSSPOINT_AY2_PIN);

        switch (x) {
        case 0:
            break;
        case 1:
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX0_PORT, CROSSPOINT_AX0_PIN);
            break;
        case 2:
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX1_PORT, CROSSPOINT_AX1_PIN);
            break;
        case 3:
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX0_PORT, CROSSPOINT_AX0_PIN);
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX1_PORT, CROSSPOINT_AX1_PIN);
            break;
        case 4:
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX2_PORT, CROSSPOINT_AX2_PIN);
            break;
        case 5:
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX0_PORT, CROSSPOINT_AX0_PIN);
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX2_PORT, CROSSPOINT_AX2_PIN);
            break;
        case 6:
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX3_PORT, CROSSPOINT_AX3_PIN);
            break;
        case 7:
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX0_PORT, CROSSPOINT_AX0_PIN);
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX3_PORT, CROSSPOINT_AX3_PIN);
            break;
        case 8:
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX1_PORT, CROSSPOINT_AX1_PIN);
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX3_PORT, CROSSPOINT_AX3_PIN);
            break;
        case 9:
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX0_PORT, CROSSPOINT_AX0_PIN);
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX1_PORT, CROSSPOINT_AX1_PIN);
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX3_PORT, CROSSPOINT_AX3_PIN);
            break;
        case 10:
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX2_PORT, CROSSPOINT_AX2_PIN);
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX3_PORT, CROSSPOINT_AX3_PIN);
            break;
        case 11:
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX0_PORT, CROSSPOINT_AX0_PIN);
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX2_PORT, CROSSPOINT_AX2_PIN);
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX3_PORT, CROSSPOINT_AX3_PIN);
            break;
        case 12:
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX1_PORT, CROSSPOINT_AX1_PIN);
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX2_PORT, CROSSPOINT_AX2_PIN);
            break;
        case 13:
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX0_PORT, CROSSPOINT_AX0_PIN);
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX1_PORT, CROSSPOINT_AX1_PIN);
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX2_PORT, CROSSPOINT_AX2_PIN);
            break;
        case 14:
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX1_PORT, CROSSPOINT_AX1_PIN);
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX2_PORT, CROSSPOINT_AX2_PIN);
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX3_PORT, CROSSPOINT_AX3_PIN);
            break;
        case 15:
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX0_PORT, CROSSPOINT_AX0_PIN);
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX1_PORT, CROSSPOINT_AX1_PIN);
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX2_PORT, CROSSPOINT_AX2_PIN);
            ZHAL_GPIO_Set_Output(CROSSPOINT_AX3_PORT, CROSSPOINT_AX3_PIN);
            break;
        }
        switch (y) {
        case 0:
            break;
        case 1:
            ZHAL_GPIO_Set_Output(CROSSPOINT_AY0_PORT, CROSSPOINT_AY0_PIN);
            break;
        case 2:
            ZHAL_GPIO_Set_Output(CROSSPOINT_AY1_PORT, CROSSPOINT_AY1_PIN);
            break;
        case 3:
            ZHAL_GPIO_Set_Output(CROSSPOINT_AY0_PORT, CROSSPOINT_AY0_PIN);
            ZHAL_GPIO_Set_Output(CROSSPOINT_AY1_PORT, CROSSPOINT_AY1_PIN);
            break;
        case 4:
            ZHAL_GPIO_Set_Output(CROSSPOINT_AY2_PORT, CROSSPOINT_AY2_PIN);
            break;
        case 5:
            ZHAL_GPIO_Set_Output(CROSSPOINT_AY0_PORT, CROSSPOINT_AY0_PIN);
            ZHAL_GPIO_Set_Output(CROSSPOINT_AY2_PORT, CROSSPOINT_AY2_PIN);
            break;
        case 6:
            ZHAL_GPIO_Set_Output(CROSSPOINT_AY1_PORT, CROSSPOINT_AY1_PIN);
            ZHAL_GPIO_Set_Output(CROSSPOINT_AY2_PORT, CROSSPOINT_AY2_PIN);
            break;
        case 7:
            ZHAL_GPIO_Set_Output(CROSSPOINT_AY1_PORT, CROSSPOINT_AY1_PIN);
            ZHAL_GPIO_Set_Output(CROSSPOINT_AY0_PORT, CROSSPOINT_AY0_PIN);
            ZHAL_GPIO_Set_Output(CROSSPOINT_AY2_PORT, CROSSPOINT_AY2_PIN);
            break;
        }

        if (status == 0) {  // Sets data pin to the desired switch state
            ZHAL_GPIO_Reset_Output(CROSSPOINT_DATA_PORT, CROSSPOINT_DATA_PIN);
        } else {
            ZHAL_GPIO_Set_Output(CROSSPOINT_DATA_PORT, CROSSPOINT_DATA_PIN);
        }

        ZHAL_GPIO_Set_Output(CROSSPOINT_CS_PORT, CROSSPOINT_CS_PIN);    // CS pin goes to high only after address and data are defined
        Wait();     // Address must be stable before STROBE goes high

        // Strobe pulse
        ZHAL_GPIO_Set_Output(CROSSPOINT_CLK_PORT, CROSSPOINT_CLK_PIN);
        Wait();
        ZHAL_GPIO_Reset_Output(CROSSPOINT_CLK_PORT, CROSSPOINT_CLK_PIN);
        Wait();

        ZHAL_GPIO_Reset_Output(CROSSPOINT_CS_PORT, CROSSPOINT_CS_PIN);
        Wait();

        // all outputs are zeroed, to lower MT8816 power consumption.
        // a high level MCU output is at 3,3V and MT8816 supply is 5V, which places its input near the linear region
        ZHAL_GPIO_Reset_Output(CROSSPOINT_AX0_PORT, CROSSPOINT_AX0_PIN);
        ZHAL_GPIO_Reset_Output(CROSSPOINT_AX1_PORT, CROSSPOINT_AX1_PIN);
        ZHAL_GPIO_Reset_Output(CROSSPOINT_AX2_PORT, CROSSPOINT_AX2_PIN);
        ZHAL_GPIO_Reset_Output(CROSSPOINT_AX3_PORT, CROSSPOINT_AX3_PIN);
        ZHAL_GPIO_Reset_Output(CROSSPOINT_AY0_PORT, CROSSPOINT_AY0_PIN);
        ZHAL_GPIO_Reset_Output(CROSSPOINT_AY1_PORT, CROSSPOINT_AY1_PIN);
        ZHAL_GPIO_Reset_Output(CROSSPOINT_AY2_PORT, CROSSPOINT_AY2_PIN);
        ZHAL_GPIO_Reset_Output(CROSSPOINT_DATA_PORT, CROSSPOINT_DATA_PIN);
    }
}

/*
 * Crosspoint_Switch_Open_Switches
 * Open all switches
 */
void Crosspoint_Switch_Open_Switches () {
    uint8_t x, y;

    for (x = 0; x < CROSSPOINT_X_SWITCHES; x++) {
        for (y = 0; y < CROSSPOINT_Y_SWITCHES; y++) {
            Crosspoint_Switch.Switch[x][y] = TO_BE_OPENED;
        }
    }

    if (Crosspoint_Switch.Status != SW_STAT_UNINITIALIZED) {
        Crosspoint_Switch.Status = SW_STAT_WRITING;
    }
}


/*
 * Crosspoint_Switch_Set
 */
void Crosspoint_Switch_Set (const uint8_t x, const uint8_t y, const uint8_t status) {

    if ((x < CROSSPOINT_X_SWITCHES) && (y < CROSSPOINT_Y_SWITCHES)) {
        if (status == 0) {
            Crosspoint_Switch.Switch[x][y] = TO_BE_OPENED;
        } else {
            Crosspoint_Switch.Switch[x][y] = TO_BE_CLOSED;
        }
    }
    if (Crosspoint_Switch.Status != SW_STAT_UNINITIALIZED) {
        Crosspoint_Switch.Status = SW_STAT_WRITING;
    }
}


/*
 * Crosspoint_Switch_Get
 */
uint8_t Crosspoint_Switch_Get (const uint8_t x, const uint8_t y) {

    return (Crosspoint_Switch.Switch[x][y]);
}


/*
 * Crosspoint_Switch_Task
 * Must be called periodically from main application
 */
void Crosspoint_Switch_Task () {
    uint8_t x, y;

    switch (Crosspoint_Switch.Status) {
    case SW_STAT_UNINITIALIZED:
    case SW_STAT_IDLE:
        break;
    case SW_STAT_WRITING:
        for (x = 0; x < CROSSPOINT_X_SWITCHES; x++) {
            for (y = 0; y < CROSSPOINT_Y_SWITCHES; y++) {
                if (Crosspoint_Switch.Switch[x][y] == TO_BE_OPENED) {
                    Crosspoint_Switch_Control(x, y, OPEN);
                } else if (Crosspoint_Switch.Switch[x][y] == TO_BE_CLOSED) {
                    Crosspoint_Switch_Control(x, y, CLOSED);
                }
            }
        }
        Crosspoint_Switch.Status = SW_STAT_IDLE;
        break;
    default:
        Crosspoint_Switch.Status = SW_STAT_UNINITIALIZED;
        break;
    }
}


/*
 * Crosspoint_Switch_Close
 */
bool_t Crosspoint_Switch_Close () {
    bool_t status = FALSE;

    if (Crosspoint_Switch.Status != SW_STAT_WRITING) {
        Crosspoint_Switch.Status = SW_STAT_UNINITIALIZED;
        status = TRUE;
    }

    return (status);
}




