/*
 * 
 *
 */


#include <ez8.h>
#include <string.h>
#include "zhal.h"

/*

PA1 - AX0
PA2 - AX1
PA6 - AX2
PA7 - AX3
PC0 - AY0
PC6 - AY1
PC7 - AY2


PC4 - MOSI
PC5 - SCK / Strobe
PB0 - CS Crosspoint switch
PC3 - CS Shift register

PB1 - botão
PC2 - LED1
PC1 - LED debug

PA4 - UART RX
PA5 - UART TX

*/


struct {
    unsigned char Status;
} MATRIX;

struct {
    uint8_t UART_status;
    char MessageReceived[30];
    char DataReceived;
    uint8_t SizeReceived;
    uint8_t MIDI_status;
    uint32_t MIDI_timeout;
    uint8_t MIDI_Message[3];

    uint8_t SPI_status;
    uint8_t SPI_debug;
    uint8_t SPI_data_to_send[10];
} TEST;

char Message[] = "Loopback test!\r\n";

uint8_t UART_Driver_Lock_ID = 0;
uint8_t SPI_Driver_Lock_ID = 0;

struct {
    uint32_t Timeout;
    uint8_t Status;
} BUTTON;

void UART_Driver_Callback (uint8_t data) {


}

void SPI_Driver_Callback (uint8_t data) {
    ZHAL_GPIO_Config_t gpio_config = {
        ZHAL_GPIO_OUTPUT,
        ZHAL_GPIO_NORMAL,
        DISABLE,
        DISABLE,
        DISABLE,
        DISABLE
    };

    ZHAL_GPIO_Set_Output(ZHAL_GPIO_C, GPIO_PIN_3);
    ZHAL_SPI_Driver_Close (SPI_Driver_Lock_ID, ZHAL_SPI_0);

    // Crosspoint switch serial
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_C, GPIO_PIN_4 | GPIO_PIN_5, &gpio_config);
}


void MCU_INIT () {
    ZHAL_GPIO_Config_t gpio_config = {
        ZHAL_GPIO_OUTPUT,
        ZHAL_GPIO_NORMAL,
        DISABLE,
        DISABLE,
        DISABLE,
        DISABLE
    };
    ZHAL_UART_Config_t uart_config;

    // Crosspoint switch addresses
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_A, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_6 | GPIO_PIN_7, &gpio_config);
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_C, GPIO_PIN_0 | GPIO_PIN_6 | GPIO_PIN_7, &gpio_config);

    // Crosspoint switch serial
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_C, GPIO_PIN_4 | GPIO_PIN_5, &gpio_config);
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_B, GPIO_PIN_0, &gpio_config);

    // LED
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_C, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, &gpio_config);

    // botão
    gpio_config.Direction = ZHAL_GPIO_INPUT;
    gpio_config.Pull_Up = ENABLE;
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_B, GPIO_PIN_1, &gpio_config);

    // Teste serial
    uart_config.Parity = ZHAL_UART_NO_PARITY;
    uart_config.CTS = DISABLE;
    uart_config.StopBitSelect = DISABLE;
    //uart_config.BaudRate = 31250;
    uart_config.BaudRate = 38400;

    ZHAL_UART_Driver_Init (&UART_Driver_Lock_ID, ZHAL_UART_0, &uart_config, UART_Driver_Callback);

}


void SPI_CONFIG () {

    ZHAL_SPI_Config_t spi_config;

    spi_config.Mode = ZHAL_SPI_MODE_DEFAULT;
    spi_config.MasterSlave = ZHAL_SPI_MODE_MASTER;
    spi_config.PolarityPhase = ZHAL_SPI_POL_PHASE_0;
    spi_config.WiredOR = DISABLE;
    spi_config.BaudRate = 50000;

    ZHAL_SPI_Driver_Init (&SPI_Driver_Lock_ID, ZHAL_SPI_0, &spi_config, SPI_Driver_Callback);

}




void WAIT (uint16_t count) {

    while (count != 0) {
        count--;
    }
}

void CROSSPOINT_SWITCH_CONTROL (unsigned char x, unsigned char y, unsigned char status) {

    ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_5);    // Strobe
    ZHAL_GPIO_Reset_Output(ZHAL_GPIO_B, GPIO_PIN_0);    // CS
    WAIT(100);

    ZHAL_GPIO_Set_Output(ZHAL_GPIO_B, GPIO_PIN_0);    // CS
    WAIT(100);
    
    ZHAL_GPIO_Reset_Output(ZHAL_GPIO_A, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_6 | GPIO_PIN_7);
    ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_0 | GPIO_PIN_6 | GPIO_PIN_7);

    switch (x) {
    case 0:
        break;
    case 1:
        ZHAL_GPIO_Set_Output(ZHAL_GPIO_A, GPIO_PIN_1);
        break;
    case 2:
        ZHAL_GPIO_Set_Output(ZHAL_GPIO_A, GPIO_PIN_2);
        break;
    case 3:
        ZHAL_GPIO_Set_Output(ZHAL_GPIO_A, GPIO_PIN_1 | GPIO_PIN_2);
        break;
    case 4:
        ZHAL_GPIO_Set_Output(ZHAL_GPIO_A, GPIO_PIN_6);
        break;
    case 14:
        ZHAL_GPIO_Set_Output(ZHAL_GPIO_A, GPIO_PIN_2 | GPIO_PIN_6 | GPIO_PIN_7);
        break;
    case 15:
        ZHAL_GPIO_Set_Output(ZHAL_GPIO_A, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_6 | GPIO_PIN_7);
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

    WAIT(100);

    if (status == 0) {  // Data
        ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_4);
    } else {
        ZHAL_GPIO_Set_Output(ZHAL_GPIO_C, GPIO_PIN_4);
    }
    WAIT(100);

    ZHAL_GPIO_Set_Output(ZHAL_GPIO_C, GPIO_PIN_5);
    WAIT(100);

    ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_5);    // Clock
    WAIT(100);

    ZHAL_GPIO_Reset_Output(ZHAL_GPIO_B, GPIO_PIN_0);    // CS
    WAIT(100);
}


unsigned char BUTTON_DETECTION (ZHAL_GPIO_Port_t port, uint8_t pin) {
    
    switch (BUTTON.Status) {
    case 0:
        if (ZHAL_GPIO_Read_Input(port, pin) == 0) {
            BUTTON.Timeout = 10000;
            BUTTON.Status++;
        }
        break;
    case 1:
        if (BUTTON.Timeout != 0) {
            BUTTON.Timeout--;
        } else {
            BUTTON.Status++;
        }
        break;
    case 2:
        if (ZHAL_GPIO_Read_Input(port, pin) == 0) {
            BUTTON.Status = 0;
            return (1);
        }
        break;
    }
    return (0);
}


void main () {
    uint8_t i;

    ZHAL_Init();

    MCU_INIT(); //rotinas de inicialização

    MATRIX.Status = 0;
    CROSSPOINT_SWITCH_CONTROL(0, 0, 1); // bypass - X0 and Y0
    
    while (1){

        if (BUTTON_DETECTION(ZHAL_GPIO_B, GPIO_PIN_1)) {
            MATRIX.Status++;
            if (MATRIX.Status > 4) {
                MATRIX.Status = 0;
            }
#if 1
            switch (MATRIX.Status) {
            case 0: // bypass - closes X0-Y0
                CROSSPOINT_SWITCH_CONTROL(15, 0, 0);

                CROSSPOINT_SWITCH_CONTROL(0, 0, 1);     // bypass - closes X0-Y0

                TEST.SPI_data_to_send[0] = 0xFF;
                TEST.SPI_status = 2;
                break;
            case 1: // effect 1 - closes X0-Y1 and X2-Y0
                CROSSPOINT_SWITCH_CONTROL(0, 0, 0);     // opens X0-Y0 - in / out
                CROSSPOINT_SWITCH_CONTROL(15, 0, 1);    // closes X15-Y0 - GND / out

                CROSSPOINT_SWITCH_CONTROL(14, 1, 0);    // opens X14-Y1 - GND / to eff 1
                CROSSPOINT_SWITCH_CONTROL(0, 1, 1);     // closes X0-Y1 - in / to eff 1

                WAIT(60000);
                CROSSPOINT_SWITCH_CONTROL(15, 0, 0);    // opens X15-Y0 - GND / out

                CROSSPOINT_SWITCH_CONTROL(2, 0, 1);     // closes X2-Y0 - from eff 1 / out

                TEST.SPI_data_to_send[0] = 0xFE;
                TEST.SPI_status = 2;
                break;
            case 2: // effect 2 - closes X0-Y2 and X3-Y0
                CROSSPOINT_SWITCH_CONTROL(2, 0, 0);     // opens X2-Y0 - from eff 1 / out
                CROSSPOINT_SWITCH_CONTROL(15, 0, 1);    // closes X15-Y0 - GND / out

                CROSSPOINT_SWITCH_CONTROL(14, 2, 0);    // opens X14-Y2 - GND / to eff 2
                CROSSPOINT_SWITCH_CONTROL(0, 2, 1);

                CROSSPOINT_SWITCH_CONTROL(0, 1, 0);     // opens X0-Y1 - in / to eff 1
                CROSSPOINT_SWITCH_CONTROL(14, 1, 1);    // closes X14-Y1 - GND / to eff 1

                WAIT(60000);
                CROSSPOINT_SWITCH_CONTROL(15, 0, 0);    // opens X15-Y0 - GND / out

                CROSSPOINT_SWITCH_CONTROL(3, 0, 1);

                TEST.SPI_data_to_send[0] = 0xFD;
                TEST.SPI_status = 2;
                break;
            case 3: // effect 3 - closes X0-Y3 and X4-Y0
                CROSSPOINT_SWITCH_CONTROL(3, 0, 0);     // opens X3-Y0 - from eff 2 / out
                CROSSPOINT_SWITCH_CONTROL(15, 0, 1);    // closes X15-Y0 - GND / out

                CROSSPOINT_SWITCH_CONTROL(14, 3, 0);    // opens X14-Y3 - GND / to eff 3
                CROSSPOINT_SWITCH_CONTROL(0, 3, 1);

                CROSSPOINT_SWITCH_CONTROL(0, 2, 0);
                CROSSPOINT_SWITCH_CONTROL(14, 2, 1);    // closes X14-Y2 - GND / to eff 2

                WAIT(60000);
                CROSSPOINT_SWITCH_CONTROL(15, 0, 0);    // opens X15-Y0 - GND / out

                CROSSPOINT_SWITCH_CONTROL(4, 0, 1);

                TEST.SPI_data_to_send[0] = 0xFC;
                TEST.SPI_status = 2;
                break;
            case 4: // mute - closes X15-Y0
                CROSSPOINT_SWITCH_CONTROL(4, 0, 0);     // opens X4-Y0 - from eff 3 / out
                CROSSPOINT_SWITCH_CONTROL(15, 0, 1);

                CROSSPOINT_SWITCH_CONTROL(0, 3, 0);     // opens X0-Y3 - in / to eff 3
                CROSSPOINT_SWITCH_CONTROL(14, 3, 1);    // closes X14-Y3 - GND / to eff 3

                TEST.SPI_data_to_send[0] = 0xFF;
                TEST.SPI_status = 2;
                break;
            }
#endif
        }

        ZHAL_UART_Driver ();
        ZHAL_SPI_Driver ();

#if 0
        switch (TEST.UART_status) {
        // Simple message test
        case 0:
            if ((ZHAL_UART_Driver_Peek (UART_Driver_Lock_ID, &TEST.DataReceived) != 0) && (TEST.DataReceived == 0x13)) {
                TEST.UART_status = 1;
            }
            break;
        case 1:
            ZHAL_UART_Driver_Get_Data(UART_Driver_Lock_ID, TEST.MessageReceived, sizeof(TEST.MessageReceived));
            if (strcmp(TEST.MessageReceived, "LEDON") == 0) {
                ZHAL_GPIO_Set_Output(ZHAL_GPIO_C, GPIO_PIN_1);
            } else if (strcmp(TEST.MessageReceived, "LEDOF") == 0) {
            	ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_1);
            }

            for (i = 0; i < sizeof(TEST.MessageReceived); i++) {
                TEST.MessageReceived[i] = 0;
            }
            TEST.DataReceived = 0;
            TEST.UART_status = 0;
            break;

        // Loopback test
        case 2:
            ZHAL_GPIO_Set_Output(ZHAL_GPIO_C, GPIO_PIN_1);
            ZHAL_UART_Driver_Put_Data (UART_Driver_Lock_ID, Message, sizeof(Message));
            ZHAL_UART_Driver_Control (UART_Driver_Lock_ID, 1);
            TEST.UART_status++;
            break;
        case 3:
        	if ((ZHAL_UART_Driver_Peek (UART_Driver_Lock_ID, &TEST.DataReceived) != 0) && (TEST.DataReceived == 0)) {
                TEST.UART_status++;
            }
            break;
        case 4:
            ZHAL_UART_Driver_Get_Data(UART_Driver_Lock_ID, TEST.MessageReceived, sizeof(TEST.MessageReceived));
            if (strcmp(Message, TEST.MessageReceived) == 0) {  // message received is equal to message sent
                TEST.UART_status = 2;
                TEST.DataReceived = 0;
                for (i = 0; i < sizeof(TEST.MessageReceived); i++) {
                    TEST.MessageReceived[i] = 0;
                }
            } else {
                ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_1);
                TEST.UART_status = 5;
            }
            break;
        case 5:
        default:
            break;
        }
#endif
#if 0
        // MIDI test - when the button is pressed, sends note on, waits some time and sends note off
        switch (TEST.MIDI_status) {
        case 1:
            TEST.MIDI_status++;
            TEST.MIDI_timeout = 100000;
            TEST.MIDI_Message[0] = 0x90;
            TEST.MIDI_Message[1] = 0x3C;
            TEST.MIDI_Message[2] = 0x7F;
            ZHAL_UART_Driver_Put_Data(UART_Driver_Lock_ID, TEST.MIDI_Message, 3);
            ZHAL_UART_Driver_Control(UART_Driver_Lock_ID, 1);
            break;
        case 2:
            TEST.MIDI_timeout--;
            if (TEST.MIDI_timeout == 0) {
                TEST.MIDI_Message[0] = 0x80;
                TEST.MIDI_Message[1] = 0x3C;
                TEST.MIDI_Message[2] = 0x00;
                ZHAL_UART_Driver_Put_Data(UART_Driver_Lock_ID, TEST.MIDI_Message, 3);
                ZHAL_UART_Driver_Control(UART_Driver_Lock_ID, 1);
                TEST.MIDI_status = 0;
            }
            break;
        }
#endif
#if 0
        // MIDI loopback test - receives data and resends it
        switch (TEST.MIDI_status) {
        case 0:
            if ((ZHAL_UART_Driver_Peek (UART_Driver_Lock_ID, &TEST.DataReceived) != 0) && (TEST.DataReceived == 0x0D)) {
                TEST.MIDI_status = 1;
            }
            break;
        case 1:
            TEST.SizeReceived = ZHAL_UART_Driver_Get_Data(UART_Driver_Lock_ID, TEST.MessageReceived, sizeof(TEST.MessageReceived));

            ZHAL_UART_Driver_Put_Data (UART_Driver_Lock_ID, TEST.MessageReceived, TEST.SizeReceived);
            ZHAL_UART_Driver_Control (UART_Driver_Lock_ID, 1);

            for (i = 0; i < sizeof(TEST.MessageReceived); i++) {
                TEST.MessageReceived[i] = 0;
            }
            TEST.SizeReceived = 0;
            TEST.DataReceived = 0;
            TEST.MIDI_status = 0;
            break;
        }
#endif

#if 0
        // Send byte through SPI when button is pressed
        switch (TEST.SPI_status) {
        case 1:
            ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_3);
            SPI_CONFIG();

            ZHAL_UART_Driver_Put_Data (UART_Driver_Lock_ID, "Start! SPI stat:", 16);
            TEST.SPI_debug = ESPISTAT;
            for (i = 0x80; i != 0; i >>= 1) {
                if ((TEST.SPI_debug & i) != 0) {
                    ZHAL_UART_Driver_Put_Data (UART_Driver_Lock_ID, "1", 1);
                } else {
                    ZHAL_UART_Driver_Put_Data (UART_Driver_Lock_ID, "0", 1);
                }
            }
            ZHAL_UART_Driver_Put_Data (UART_Driver_Lock_ID, "State:", 6);
            TEST.SPI_debug = ESPISTATE;
            for (i = 0x80; i != 0; i >>= 1) {
                if ((TEST.SPI_debug & i) != 0) {
                    ZHAL_UART_Driver_Put_Data (UART_Driver_Lock_ID, "1", 1);
                } else {
                    ZHAL_UART_Driver_Put_Data (UART_Driver_Lock_ID, "0", 1);
                }
            }
            ZHAL_UART_Driver_Put_Data (UART_Driver_Lock_ID, "\n", 1);
            ZHAL_UART_Driver_Control (UART_Driver_Lock_ID, 1);

            ZHAL_SPI_Driver_Put_Data (SPI_Driver_Lock_ID, TEST.SPI_data_to_send, 1);
            ZHAL_SPI_Driver_Control (SPI_Driver_Lock_ID, 1);

            TEST.SPI_status++;
            break;
        case 2:
            break;
        case 3:
            ZHAL_GPIO_Set_Output(ZHAL_GPIO_C, GPIO_PIN_3);
            ZHAL_SPI_Driver_Close (SPI_Driver_Lock_ID, ZHAL_SPI_0);

            ZHAL_UART_Driver_Put_Data (UART_Driver_Lock_ID, "Message sent!\n", 14);
            ZHAL_UART_Driver_Control (UART_Driver_Lock_ID, 1);

            TEST.SPI_status = 0;
            break;
        }
#endif


        // SPI test - receive a byte from UART and sends to Shift register
        switch (TEST.SPI_status) {
        case 0:
            if (ZHAL_UART_Driver_Get_Data(UART_Driver_Lock_ID, &TEST.DataReceived, 1) != 0) {
                TEST.SPI_status = 1;
            }
            break;
        case 1:
            ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_3);
            SPI_CONFIG();

            ZHAL_SPI_Driver_Put_Data (SPI_Driver_Lock_ID, &TEST.DataReceived, 1);
            ZHAL_SPI_Driver_Control (SPI_Driver_Lock_ID, 1);

            TEST.SPI_status = 0;
            break;
        case 2: // from another source
            ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_3);
            SPI_CONFIG();

            ZHAL_SPI_Driver_Put_Data (SPI_Driver_Lock_ID, TEST.SPI_data_to_send, 1);
            ZHAL_SPI_Driver_Control (SPI_Driver_Lock_ID, 1);

            TEST.SPI_status = 0;
            break;
        }
    }
}







