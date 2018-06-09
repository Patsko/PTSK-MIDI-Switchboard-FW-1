/*
 * Crosspoint switch
 *
 */

#ifndef CROSSPOINT_SWITCH_H // include guard
#define CROSSPOINT_SWITCH_H

#include <ez8.h>
#include "zhal.h"
#include "bsp.h"

/*
 * Application defines
 */

#define CROSSPOINT_DATA_PORT    SPI_MOSI_PORT
#define CROSSPOINT_DATA_PIN     SPI_MOSI_PIN
#define CROSSPOINT_CLK_PORT     SPI_SCK_PORT
#define CROSSPOINT_CLK_PIN      SPI_SCK_PIN
#define CROSSPOINT_CS_PORT      CS_SWITCH_PORT
#define CROSSPOINT_CS_PIN       CS_SWITCH_PIN

#define CROSSPOINT_AX0_PORT  C_SW1_PORT
#define CROSSPOINT_AX0_PIN   C_SW1_PIN
#define CROSSPOINT_AX1_PORT  C_SW2_PORT
#define CROSSPOINT_AX1_PIN   C_SW2_PIN
#define CROSSPOINT_AX2_PORT  C_SW3_PORT
#define CROSSPOINT_AX2_PIN   C_SW3_PIN
#define CROSSPOINT_AX3_PORT  C_SW4_PORT
#define CROSSPOINT_AX3_PIN   C_SW4_PIN
#define CROSSPOINT_AY0_PORT  C_SW5_PORT
#define CROSSPOINT_AY0_PIN   C_SW5_PIN
#define CROSSPOINT_AY1_PORT  C_SW6_PORT
#define CROSSPOINT_AY1_PIN   C_SW6_PIN
#define CROSSPOINT_AY2_PORT  C_SW7_PORT
#define CROSSPOINT_AY2_PIN   C_SW7_PIN

#define CROSSPOINT_X_SWITCHES   16
#define CROSSPOINT_Y_SWITCHES   8

/*
 * Typedefs
 */


/*
 * Function prototypes
 */

void Crosspoint_Switch_Init (void);
void Crosspoint_Switch_Open_Switches ();
void Crosspoint_Switch_Set (const uint8_t x, const uint8_t y, const uint8_t status);
uint8_t Crosspoint_Switch_Get (const uint8_t x, const uint8_t y);
void Crosspoint_Switch_Task (void);

#endif // CROSSPOINT_SWITCH_H
