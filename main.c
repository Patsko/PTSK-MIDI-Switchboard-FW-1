/*
 * 
 *
 */


#include <ez8.h>
#include "zhal.h"

/*

PC0 - AX0
PA6 - AX1
PC2 - AX2
PC3 - AX3
PA2 - AY0
PA7 - AY1
PC7 - AY2


PC4 - Data
PC5 - Strobe
PD0 - CS

PA4 - botão
PB0 - LED
*/


struct {
    unsigned char Status;
} MATRIX;


void MCU_INIT () {

    PAOUT = 0x00;
    PAOC = 0xC4;  // Output control - PA2, PA6, PA7 as open-drain
    PADD = 0x3B;  // Data direction - PA2, PA6, PA7 as outputs
    
    PBOUT = 0x00;
    PBDD = 0xFE;    // Data direction - PB0 as output
    
    PCOUT = 0x00;
    PCOC = 0xBD;  // Output control - PC0, PC2, PC3, PC4, PC5, PC7 as open-drain
    PCDD = 0x42;  // Data direction - PC0, PC2, PC3, PC4, PC5, PC7 as outputs
    
    PDOUT = 0x00;
    PDOC = 0x01;    // Output control - PD0 as open-drain
    PDDD = 0x00;    // Data direction - PD0 as output
}

void CROSSPOINT_SWITCH_CONTROL (unsigned char sw, unsigned char status) {

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
}


unsigned char BUTTON () {
    long i;
    volatile unsigned char btn_value;
    
    btn_value = PAIN;
    
    if ((btn_value & 0x10) == 0) {        
        for (i = 100000; i != 0; i--);
        if ((PAIN & 0x10) == 0) {
            return (1);
        }        
    }
    return (0);
}


void main () {

    MCU_INIT(); //rotinas de inicialização

    MATRIX.Status = 0;
    
    while (1){

        if (BUTTON()) {
            if (MATRIX.Status == 0) {
                // Closes X1-Y1 switch
                CROSSPOINT_SWITCH_CONTROL(0, 0);
                CROSSPOINT_SWITCH_CONTROL(1, 1);
                
                ZHAL_GPIO_SetOutputPin(GPIO_PORTB, GPIO_PIN_0);
                
                MATRIX.Status = 1;
            } else {
                // Closes X0-Y0 switch
                CROSSPOINT_SWITCH_CONTROL(0, 1);
                CROSSPOINT_SWITCH_CONTROL(1, 0);

                ZHAL_GPIO_ResetOutputPin(GPIO_PORTB, GPIO_PIN_0);
                
                MATRIX.Status = 0;
            }
        }
    }
}







