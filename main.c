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
PC2 - CS Memory
PC1 - MISO

PB1 - botão

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


ZHAL_UART_Driver_t UART_Driver_Handle;
ZHAL_SPI_Driver_t SPI_Driver_Handle;

char Message[] = "Loopback test!\r\n";


struct {
    uint32_t Timeout;
    uint8_t Status;
} BUTTON;

void UART_Driver_Callback (void) {


}

void SPI_Driver_Callback (void) {
    ZHAL_GPIO_Config_t gpio_config = {
        ZHAL_GPIO_OUTPUT,
        ZHAL_GPIO_NORMAL,
        DISABLE,
        DISABLE,
        DISABLE,
        DISABLE
    };

    ZHAL_GPIO_Set_Output(ZHAL_GPIO_C, GPIO_PIN_2 | GPIO_PIN_3); // CS pins back to 1

    if ((TEST.SPI_status == 4) || (TEST.SPI_status == 7)) { // if it is writing to memory or has written to it
        TEST.SPI_status++;
    } else {
        ZHAL_SPI_Driver_Close(&SPI_Driver_Handle);
        // Crosspoint switch serial
        ZHAL_GPIO_Config_Pin(ZHAL_GPIO_C, GPIO_PIN_4 | GPIO_PIN_5, &gpio_config);
    }

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

    // Crosspoint switch addresses
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_A, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_6 | GPIO_PIN_7, &gpio_config);
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_C, GPIO_PIN_0 | GPIO_PIN_6 | GPIO_PIN_7, &gpio_config);

    // Crosspoint switch serial
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_C, GPIO_PIN_4 | GPIO_PIN_5, &gpio_config);
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_B, GPIO_PIN_0, &gpio_config);

    // SPI CS
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_C, GPIO_PIN_2 | GPIO_PIN_3, &gpio_config);

    // botão
    gpio_config.Direction = ZHAL_GPIO_INPUT;
    gpio_config.Pull_Up = ENABLE;
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_B, GPIO_PIN_1, &gpio_config);

    // Teste serial
    UART_Driver_Handle.BaudRate = 38400;
    UART_Driver_Handle.TxCallback = UART_Driver_Callback;

    ZHAL_UART_Driver_Init(&UART_Driver_Handle);

}


void SPI_CONFIG () {

    SPI_Driver_Handle.BaudRate = 50000;
    SPI_Driver_Handle.TxCallback = SPI_Driver_Callback;

    ZHAL_SPI_Driver_Init(&SPI_Driver_Handle);

}




void WAIT () {
    uint8_t count = 100;

    while (count != 0) {
        count--;
    }
}

void CROSSPOINT_SWITCH_CONTROL (unsigned char x, unsigned char y, unsigned char status) {

    ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_5);    // Strobe
    ZHAL_GPIO_Reset_Output(ZHAL_GPIO_B, GPIO_PIN_0);    // CS
    WAIT();

    ZHAL_GPIO_Set_Output(ZHAL_GPIO_B, GPIO_PIN_0);    // CS
    WAIT();
    
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

    WAIT();

    if (status == 0) {  // Data
        ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_4);
    } else {
        ZHAL_GPIO_Set_Output(ZHAL_GPIO_C, GPIO_PIN_4);
    }
    WAIT();

    ZHAL_GPIO_Set_Output(ZHAL_GPIO_C, GPIO_PIN_5);
    WAIT();

    ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_5);    // Clock
    WAIT();

    ZHAL_GPIO_Reset_Output(ZHAL_GPIO_B, GPIO_PIN_0);    // CS
    WAIT();
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
    uint8_t data;

    ZHAL_Init();

    MCU_INIT(); //rotinas de inicialização

    MATRIX.Status = 0;
    CROSSPOINT_SWITCH_CONTROL(0, 0, 1);
    
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

                CROSSPOINT_SWITCH_CONTROL(0, 0, 1);

                TEST.SPI_data_to_send[0] = 0xFF;
                TEST.SPI_status = 2;
                break;
            case 1: // effect 1 - closes X0-Y1 and X2-Y0
                CROSSPOINT_SWITCH_CONTROL(0, 0, 0);

                CROSSPOINT_SWITCH_CONTROL(0, 1, 1);
                CROSSPOINT_SWITCH_CONTROL(2, 0, 1);

                TEST.SPI_data_to_send[0] = 0xFE;
                TEST.SPI_status = 2;
                break;
            case 2: // effect 2 - closes X0-Y2 and X3-Y0
                CROSSPOINT_SWITCH_CONTROL(0, 1, 0);
                CROSSPOINT_SWITCH_CONTROL(2, 0, 0);

                CROSSPOINT_SWITCH_CONTROL(0, 2, 1);
                CROSSPOINT_SWITCH_CONTROL(3, 0, 1);

                TEST.SPI_data_to_send[0] = 0xFD;
                TEST.SPI_status = 2;
                break;
            case 3: // effect 3 - closes X0-Y3 and X4-Y0
                CROSSPOINT_SWITCH_CONTROL(0, 2, 0);
                CROSSPOINT_SWITCH_CONTROL(3, 0, 0);

                CROSSPOINT_SWITCH_CONTROL(0, 3, 1);
                CROSSPOINT_SWITCH_CONTROL(4, 0, 1);

                TEST.SPI_data_to_send[0] = 0xFC;
                TEST.SPI_status = 2;
                break;
            case 4: // mute - closes X15-Y0
                CROSSPOINT_SWITCH_CONTROL(0, 3, 0);
                CROSSPOINT_SWITCH_CONTROL(4, 0, 0);

                CROSSPOINT_SWITCH_CONTROL(15, 0, 1);

                TEST.SPI_data_to_send[0] = 0xFF;
                TEST.SPI_status = 2;
                break;
            }
#endif
        }


#if 0
        switch (TEST.UART_status) {
        // Simple message test
        case 0:
            if ((ZHAL_UART_Driver_Peek (&UART_Driver_Handle, &TEST.DataReceived) != 0) && (TEST.DataReceived == 0x13)) {
                TEST.UART_status = 1;
            }
            break;
        case 1:
            ZHAL_UART_Driver_Get_Data(&UART_Driver_Handle, TEST.MessageReceived, sizeof(TEST.MessageReceived));
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
            ZHAL_UART_Driver_Put_Data (&UART_Driver_Handle, Message, sizeof(Message));
            ZHAL_UART_Driver_Send_Data (&UART_Driver_Handle);
            TEST.UART_status++;
            break;
        case 3:
        	if ((ZHAL_UART_Driver_Peek (&UART_Driver_Handle, &TEST.DataReceived) != 0) && (TEST.DataReceived == 0)) {
                TEST.UART_status++;
            }
            break;
        case 4:
            ZHAL_UART_Driver_Get_Data(&UART_Driver_Handle, TEST.MessageReceived, sizeof(TEST.MessageReceived));
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
            ZHAL_UART_Driver_Put_Data(&UART_Driver_Handle, TEST.MIDI_Message, 3);
            ZHAL_UART_Driver_Send_Data(&UART_Driver_Handle);
            break;
        case 2:
            TEST.MIDI_timeout--;
            if (TEST.MIDI_timeout == 0) {
                TEST.MIDI_Message[0] = 0x80;
                TEST.MIDI_Message[1] = 0x3C;
                TEST.MIDI_Message[2] = 0x00;
                ZHAL_UART_Driver_Put_Data(&UART_Driver_Handle, TEST.MIDI_Message, 3);
                ZHAL_UART_Driver_Send_Data(&UART_Driver_Handle);
                TEST.MIDI_status = 0;
            }
            break;
        }
#endif
#if 0
        // MIDI loopback test - receives data and resends it
        switch (TEST.MIDI_status) {
        case 0:
            if ((ZHAL_UART_Driver_Peek (&UART_Driver_Handle, &TEST.DataReceived) != 0) && (TEST.DataReceived == 0x0D)) {
                TEST.MIDI_status = 1;
            }
            break;
        case 1:
            TEST.SizeReceived = ZHAL_UART_Driver_Get_Data(&UART_Driver_Handle, TEST.MessageReceived, sizeof(TEST.MessageReceived));

            ZHAL_UART_Driver_Put_Data (&UART_Driver_Handle, TEST.MessageReceived, TEST.SizeReceived);
            ZHAL_UART_Driver_Send_Data(&UART_Driver_Handle);

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

            ZHAL_UART_Driver_Put_Data (&UART_Driver_Handle, "Start! SPI stat:", 16);
            TEST.SPI_debug = ESPISTAT;
            for (i = 0x80; i != 0; i >>= 1) {
                if ((TEST.SPI_debug & i) != 0) {
                    ZHAL_UART_Driver_Put_Data (&UART_Driver_Handle, "1", 1);
                } else {
                    ZHAL_UART_Driver_Put_Data (&UART_Driver_Handle, "0", 1);
                }
            }
            ZHAL_UART_Driver_Put_Data (&UART_Driver_Handle, "State:", 6);
            TEST.SPI_debug = ESPISTATE;
            for (i = 0x80; i != 0; i >>= 1) {
                if ((TEST.SPI_debug & i) != 0) {
                    ZHAL_UART_Driver_Put_Data (&UART_Driver_Handle, "1", 1);
                } else {
                    ZHAL_UART_Driver_Put_Data (&UART_Driver_Handle, "0", 1);
                }
            }
            ZHAL_UART_Driver_Put_Data(&UART_Driver_Handle, "\n", 1);
            ZHAL_UART_Driver_Send_Data(&UART_Driver_Handle);

            ZHAL_SPI_Driver_Put_Data(&SPI_Driver_Handle, TEST.SPI_data_to_send, 1);
            ZHAL_SPI_Driver_Send_Data(&SPI_Driver_Handle));

            TEST.SPI_status++;
            break;
        case 2:
            break;
        case 3:
            ZHAL_GPIO_Set_Output(ZHAL_GPIO_C, GPIO_PIN_3);
            ZHAL_SPI_Driver_Close(&SPI_Driver_Handle);

            ZHAL_UART_Driver_Put_Data (&UART_Driver_Handle, "Message sent!\n", 14);
            ZHAL_UART_Driver_Send_Data(&UART_Driver_Handle);

            TEST.SPI_status = 0;
            break;
        }
#endif


        // SPI test - receive a byte from UART and sends to Shift register
        switch (TEST.SPI_status) {
        case 0:
#if 0
            // write to SPI data from UART
            if (ZHAL_UART_Driver_Get_Data(&UART_Driver_Handle, &TEST.DataReceived, 1) != 0) {
                TEST.SPI_status = 1;
            }
#endif
            if (ZHAL_UART_Driver_Get_Data(&UART_Driver_Handle, &TEST.DataReceived, 1) != 0) {
                if (TEST.DataReceived == 'W') { // write to memory
                    TEST.SPI_status = 3;
                } else if (TEST.DataReceived == 'R') {  // read from memory
                    TEST.SPI_status = 6;
                }
            }
            break;
        case 1:
            ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_3);
            SPI_CONFIG();

            ZHAL_SPI_Driver_Put_Data(&SPI_Driver_Handle, &TEST.DataReceived, 1);
            ZHAL_SPI_Driver_Send_Data(&SPI_Driver_Handle);

            TEST.SPI_status = 0;
            break;
        case 2: // from another source
            ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_3);
            SPI_CONFIG();

            ZHAL_SPI_Driver_Put_Data(&SPI_Driver_Handle, TEST.SPI_data_to_send, 1);
            ZHAL_SPI_Driver_Send_Data(&SPI_Driver_Handle);

            TEST.SPI_status = 0;

            break;
        case 3: // FRAM write enable
            ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_2);    // Memory Chip select
            SPI_CONFIG();

            TEST.SPI_data_to_send[0] = 0x06;    // WREN
            ZHAL_SPI_Driver_Put_Data(&SPI_Driver_Handle, TEST.SPI_data_to_send, 1);
            ZHAL_SPI_Driver_Send_Data(&SPI_Driver_Handle);

            TEST.SPI_status = 4;
            break;
        case 4: // waiting write enable
            break;
        case 5: // FRAM write
            ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_2);    // Memory Chip select

            TEST.SPI_data_to_send[0] = 0x02;    // WRITE
            TEST.SPI_data_to_send[1] = 0x00;    // addr high
            TEST.SPI_data_to_send[2] = 0x00;    // addr low
            TEST.SPI_data_to_send[3] = 0x01;    // data
            TEST.SPI_data_to_send[4] = 0x02;    // data
            TEST.SPI_data_to_send[5] = 0x03;    // data
            TEST.SPI_data_to_send[6] = 0x04;    // data
            ZHAL_SPI_Driver_Put_Data(&SPI_Driver_Handle, TEST.SPI_data_to_send, 7);
            ZHAL_SPI_Driver_Send_Data(&SPI_Driver_Handle);

            TEST.SPI_status = 0;
            break;
        case 6: // FRAM read
            ZHAL_GPIO_Reset_Output(ZHAL_GPIO_C, GPIO_PIN_2);    // Memory Chip select
            SPI_CONFIG();

            TEST.SPI_data_to_send[0] = 0x03;    // READ
            TEST.SPI_data_to_send[1] = 0x00;    // addr high
            TEST.SPI_data_to_send[2] = 0x00;    // addr low
            TEST.SPI_data_to_send[3] = 0xFF;    // dummy
            TEST.SPI_data_to_send[4] = 0xFF;    // dummy
            TEST.SPI_data_to_send[5] = 0xFF;    // dummy
            TEST.SPI_data_to_send[6] = 0xFF;    // dummy
            ZHAL_SPI_Driver_Put_Data(&SPI_Driver_Handle, TEST.SPI_data_to_send, 7);
            ZHAL_SPI_Driver_Send_Data(&SPI_Driver_Handle);

            TEST.SPI_status = 7;
            break;
        case 7:
            break;
        case 8:
            // send all data to UART

            i = ZHAL_SPI_Driver_Peek(&SPI_Driver_Handle, NULL);  // get the number of bytes available

            while (i > 0) {
                ZHAL_SPI_Driver_Get_Data(&SPI_Driver_Handle, &data, 1);
                ZHAL_UART_Driver_Put_Data (&UART_Driver_Handle, &data, 1);
                i--;
            }
            ZHAL_UART_Driver_Send_Data(&UART_Driver_Handle);

            TEST.SPI_status = 0;
            break;
        }
    }
}







