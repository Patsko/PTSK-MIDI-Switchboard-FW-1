/*
 * 
 *
 */


#include <ez8.h>
#include "zhal.h"

/*

PA1 - AX0
PA2 - AX1
PA6 - AX2
PA7 - AX3
PC0 - AY0
PC6 - AY1
PC7 - AY2


PC4 - Data
PC5 - Strobe
PB0 - CS

PB1 - botão
PC3 - LED
*/


struct {
    unsigned char Status;
} MATRIX;


void MCU_INIT () {
    ZHAL_GPIO_Config_t gpio_config = {
        ZHAL_GPIO_OUTPUT,
        0,
        ENABLE, // Open drain
        DISABLE,
        DISABLE,
        DISABLE
    };

    // Crosspoint switch addresses
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_A, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_6 | GPIO_PIN_7, &gpio_config);
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_C, GPIO_PIN_0 | GPIO_PIN_6 | GPIO_PIN_7, &gpio_config);

    // Crosspoint switch serial
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_C, GPIO_PIN_4 | GPIO_PIN_5, &gpio_config);
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_B, GPIO_PIN_0, &gpio_config);

    // LED
    gpio_config.Open_Drain = DISABLE;
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_C, GPIO_PIN_3, &gpio_config);

    // botão
    gpio_config.Direction = ZHAL_GPIO_INPUT;
    gpio_config.Pull_Up = ENABLE;
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_B, GPIO_PIN_1, &gpio_config);


}

void CROSSPOINT_SWITCH_CONTROL (unsigned char sw, unsigned char status) {

    ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_5);    // Strobe
    ZHAL_GPIO_Reset_Output(ZHAL_GPIO_B, GPIO_PIN_0);    // CS

    ZHAL_GPIO_Set_Output(ZHAL_GPIO_B, GPIO_PIN_0);    // CS
    
    switch (sw) {
    case 0: // Sets X0-Y0 switch
        ZHAL_GPIO_Reset_Output(ZHAL_GPIO_A, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_6 | GPIO_PIN_7);
        ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_0 | GPIO_PIN_6 | GPIO_PIN_7);
        break;
    case 1: // Sets X1-Y1 switch
        ZHAL_GPIO_Reset_Output(ZHAL_GPIO_A, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_6 | GPIO_PIN_7);
        ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_0 | GPIO_PIN_6 | GPIO_PIN_7);
        ZHAL_GPIO_Set_Output(ZHAL_GPIO_A, GPIO_PIN_1);
        ZHAL_GPIO_Set_Output(ZHAL_GPIO_C, GPIO_PIN_0);
        break;
    }

    if (status == 0) {  // Data
        ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_4);
    } else {
        ZHAL_GPIO_Set_Output(ZHAL_GPIO_C, GPIO_PIN_4);
    }

    ZHAL_GPIO_Set_Output(ZHAL_GPIO_C, GPIO_PIN_5);

    ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_5);
    ZHAL_GPIO_Reset_Output(ZHAL_GPIO_B, GPIO_PIN_0);    // CS

#if 0
    PCOUT &= 0xDF;
    PDOUT &= 0xF7;  // CS and strobe at 0
    
    PDOUT |= 0x01;  // CS at 1
    
    switch (sw) {
    case 0: // Sets X0-Y0 switch
        PAOUT &= 0x3B;
        PCOUT &= 0x72;
        break;    
    case 1: // Sets X1-Y1 switch
        PAOUT &= 0x3B;
        PCOUT &= 0x72;
        PAOUT |= 0x04;
        PCOUT |= 0x01;
        break;        
    }
    
    if (status == 0) {  // Data
        PCOUT &= 0xEF;
    } else {
        PCOUT |= 0x10;
    }
    
    PCOUT |= 0x20;  // Strobe at 1    
    
    PCOUT &= 0xDF;  // Strobe at 0
    PDOUT &= 0xF7;  // CS at 0
#endif
}


unsigned char BUTTON () {
    long i;
    
    if (ZHAL_GPIO_Read_Input(ZHAL_GPIO_B, GPIO_PIN_1) == 0) {
        for (i = 100000; i != 0; i--);
        if (ZHAL_GPIO_Read_Input(ZHAL_GPIO_B, GPIO_PIN_1) == 0) {
            return (1);
        }        
    }
    return (0);
}


void main () {

    ZHAL_Init();

    MCU_INIT(); //rotinas de inicialização

    MATRIX.Status = 0;
    
    while (1){

        if (BUTTON()) {
            if (MATRIX.Status == 0) {
                // Closes X1-Y1 switch
                CROSSPOINT_SWITCH_CONTROL(0, 0);
                CROSSPOINT_SWITCH_CONTROL(1, 1);
                
                ZHAL_GPIO_Set_Output(ZHAL_GPIO_C, GPIO_PIN_3);
                
                MATRIX.Status = 1;
            } else {
                // Closes X0-Y0 switch
                CROSSPOINT_SWITCH_CONTROL(0, 1);
                CROSSPOINT_SWITCH_CONTROL(1, 0);

                ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_3);
                
                MATRIX.Status = 0;
            }
        }
    }
}







