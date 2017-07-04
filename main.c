/*
 * 
 *
 */


#include <ez8.h>
#include "zhal.h"

/*

PA1 - AX0       OBS: alterado para PA0
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
PC2 - LED1
PC3 - LED2

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
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_A, GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_6 | GPIO_PIN_7, &gpio_config);
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_C, GPIO_PIN_0 | GPIO_PIN_6 | GPIO_PIN_7, &gpio_config);

    // Crosspoint switch serial
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_C, GPIO_PIN_4 | GPIO_PIN_5, &gpio_config);
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_B, GPIO_PIN_0, &gpio_config);

    // LED
    gpio_config.Open_Drain = DISABLE;
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_C, GPIO_PIN_2 | GPIO_PIN_3, &gpio_config);

    // botão
    gpio_config.Direction = ZHAL_GPIO_INPUT;
    gpio_config.Pull_Up = ENABLE;
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_B, GPIO_PIN_1, &gpio_config);


}

void CROSSPOINT_SWITCH_CONTROL (unsigned char x, unsigned char y, unsigned char status) {

    ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_5);    // Strobe
    ZHAL_GPIO_Reset_Output(ZHAL_GPIO_B, GPIO_PIN_0);    // CS

    ZHAL_GPIO_Set_Output(ZHAL_GPIO_B, GPIO_PIN_0);    // CS
    
    ZHAL_GPIO_Reset_Output(ZHAL_GPIO_A, GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_6 | GPIO_PIN_7);
    ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_0 | GPIO_PIN_6 | GPIO_PIN_7);

    switch (x) {
    case 0:
        break;
    case 1:
        ZHAL_GPIO_Set_Output(ZHAL_GPIO_A, GPIO_PIN_0);
        break;
    case 2:
        ZHAL_GPIO_Set_Output(ZHAL_GPIO_A, GPIO_PIN_2);
        break;
    case 3:
        ZHAL_GPIO_Set_Output(ZHAL_GPIO_A, GPIO_PIN_0 | GPIO_PIN_2);
        break;
    case 4:
        ZHAL_GPIO_Set_Output(ZHAL_GPIO_A, GPIO_PIN_6);
        break;
    case 15:
        ZHAL_GPIO_Set_Output(ZHAL_GPIO_A, GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_6 | GPIO_PIN_7);
        break;
    }
    switch (y) {
    case 0:
        break;
    case 1:
        ZHAL_GPIO_Set_Output(ZHAL_GPIO_C, GPIO_PIN_0);
        break;
    case 2:
        ZHAL_GPIO_Set_Output(ZHAL_GPIO_C, GPIO_PIN_6);
        break;
    case 3:
        ZHAL_GPIO_Set_Output(ZHAL_GPIO_C, GPIO_PIN_0 | GPIO_PIN_6);
        break;
    case 4:
        ZHAL_GPIO_Set_Output(ZHAL_GPIO_C, GPIO_PIN_7);
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
    CROSSPOINT_SWITCH_CONTROL(0, 0, 1);
    
    while (1){

        if (BUTTON()) {
            MATRIX.Status++;
            if (MATRIX.Status > 4) {
                MATRIX.Status = 0;
            }

            switch (MATRIX.Status) {
            case 0: // bypass - closes X0-Y0
                CROSSPOINT_SWITCH_CONTROL(0, 0, 1);

                ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_2 | GPIO_PIN_3);
                break;
            case 1: // effect 1 - closes X0-Y1 and X2-Y0
                CROSSPOINT_SWITCH_CONTROL(0, 0, 0);

                CROSSPOINT_SWITCH_CONTROL(0, 1, 1);
                CROSSPOINT_SWITCH_CONTROL(2, 0, 1);

                ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_2 | GPIO_PIN_3);
                ZHAL_GPIO_Set_Output(ZHAL_GPIO_C, GPIO_PIN_2);
                break;
            case 2: // effect 2 - closes X0-Y2 and X3-Y0
                CROSSPOINT_SWITCH_CONTROL(0, 1, 0);
                CROSSPOINT_SWITCH_CONTROL(2, 0, 0);

                CROSSPOINT_SWITCH_CONTROL(0, 2, 1);
                CROSSPOINT_SWITCH_CONTROL(3, 0, 1);

                ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_2 | GPIO_PIN_3);
                ZHAL_GPIO_Set_Output(ZHAL_GPIO_C, GPIO_PIN_3);
                break;
            case 3: // effect 3 - closes X0-Y3 and X4-Y0
                CROSSPOINT_SWITCH_CONTROL(0, 2, 0);
                CROSSPOINT_SWITCH_CONTROL(3, 0, 0);

                CROSSPOINT_SWITCH_CONTROL(0, 3, 1);
                CROSSPOINT_SWITCH_CONTROL(4, 0, 1);

                ZHAL_GPIO_Set_Output(ZHAL_GPIO_C, GPIO_PIN_2 | GPIO_PIN_3);
                break;
            case 4: // mute - closes X15-Y0
                CROSSPOINT_SWITCH_CONTROL(0, 3, 0);
                CROSSPOINT_SWITCH_CONTROL(4, 0, 0);

                CROSSPOINT_SWITCH_CONTROL(15, 0, 1);

                ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_2 | GPIO_PIN_3);
                break;
            }
        }
    }
}







