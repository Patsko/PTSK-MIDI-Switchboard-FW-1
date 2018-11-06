/*
 * test.c
 */

#include <ez8.h>
#include <string.h>

#include "zhal.h"
#include "bsp.h"
#include "sw_timer.h"
#include "output_expander.h"
#include "memory.h"
#include "keypad.h"

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
    uint8_t SPI_data[10];

    uint8_t UART_Rx_count;
    uint8_t UART_inserted_bytes;
    uint8_t Flag;

    uint8_t TimerStatus;
    SW_Timer_t Timer;

    uint8_t KeypadStatus;
    uint8_t KeypadLed[4];
} TEST;


char Message[] = "Loopback test!\r\n";


struct {
    uint32_t Timeout;
    uint8_t Status;
    bool_t IsPressed;
} BUTTON;



void MCU_INIT () {
    ZHAL_GPIO_Config_t gpio_config = {
        ZHAL_GPIO_OUTPUT,
        ZHAL_GPIO_NORMAL,
        DISABLE,
        DISABLE,
        DISABLE,
        DISABLE
    };
    ZHAL_UART_Driver_Config_t uart_config = {
        38400,
        NULL,
        NULL
    };

    // Crosspoint switch addresses
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_A, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_6 | GPIO_PIN_7, &gpio_config);
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_C, GPIO_PIN_0 | GPIO_PIN_6 | GPIO_PIN_7, &gpio_config);

    // Crosspoint switch serial
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_C, GPIO_PIN_4 | GPIO_PIN_5, &gpio_config);
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_B, GPIO_PIN_0, &gpio_config);


    // botão
    gpio_config.Direction = ZHAL_GPIO_INPUT;
    gpio_config.Pull_Up = ENABLE;
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_B, GPIO_PIN_1, &gpio_config);

    ZHAL_UART_Driver_Init(&uart_config);

}


void SPI_CONFIG () {
    const ZHAL_GPIO_Config_t gpio_config = {
        ZHAL_GPIO_OUTPUT,
        ZHAL_GPIO_NORMAL,
        DISABLE,
        DISABLE,
        DISABLE,
        DISABLE
    };

    // SPI CS
    ZHAL_GPIO_Config_Pin(CS_MEM_PORT, CS_MEM_PIN, &gpio_config);
    ZHAL_GPIO_Config_Pin(CS_SHIFT_PORT, CS_SHIFT_PIN, &gpio_config);

    ZHAL_GPIO_Set_Output(CS_MEM_PORT, CS_MEM_PIN);
    ZHAL_GPIO_Set_Output(CS_SHIFT_PORT, CS_SHIFT_PIN);

    ZHAL_SPI_Driver_Init();
}



void UART_TEST () {
    uint8_t i;
    uint8_t data;
    char test_message[] = "UART test: ";

    switch (TEST.UART_status) {
    // Loopback test
    case 0:
        if ((ZHAL_UART_Driver_Peek(&data) != 0) && (data == '\r')) {
            TEST.UART_status = 1;
        }
        break;
    case 1:
        i = ZHAL_UART_Driver_Peek(NULL);   // gets the available bytes quantity

        for (; i != 0; i--) {
            ZHAL_UART_Driver_Get_Data(&data, 1);
            ZHAL_UART_Driver_Put_Data(&data, 1);
        }
        ZHAL_UART_Driver_Send_Data();

        TEST.UART_status = 0;
        break;
    }
}


void SPI_TEST () {
    uint8_t i;
    uint8_t data;
    uint8_t data_buf[4];
    uint16_t data16;

    // Send byte through SPI when button is pressed
    switch (TEST.SPI_status) {
    case 0:
        if (ZHAL_UART_Driver_Peek(&data) != 0) {
            if (data == 'W') { // write to memory
                ZHAL_UART_Driver_Get_Data(&data, 1);
                TEST.SPI_status = 3;
            } else if (data == 'R') {  // read from memory
                ZHAL_UART_Driver_Get_Data(&data, 1);
                TEST.SPI_status = 1;
            } else if (data == 'E') { // "erase" memory
                ZHAL_UART_Driver_Get_Data(&data, 1);
                TEST.SPI_status = 4;
            }
        }
        break;

    case 1: // FRAM read
        if (Memory_Read_Data(0x00, &TEST.SPI_data, 4)) {
            TEST.SPI_status++;
        }
        break;
    case 2:
        if (ZHAL_UART_Driver_Put_Data ("Read finished", sizeof("Read finished") - 1)) {
            ZHAL_UART_Driver_Put_Data (&TEST.SPI_data, 4);
            ZHAL_UART_Driver_Send_Data();
            TEST.SPI_status = 0;
        }
        break;

    case 3:
        TEST.SPI_data[0] = 5;
        TEST.SPI_data[1] = 6;
        TEST.SPI_data[2] = 7;
        TEST.SPI_data[3] = 8;
        if (Memory_Write_Data(0x00, &TEST.SPI_data, 4)) {
            ZHAL_UART_Driver_Put_Data ("Write started\n", sizeof("Write started\n"));
            ZHAL_UART_Driver_Send_Data();
            TEST.SPI_status = 0;
        }
        break;

    case 4:
        TEST.SPI_data[0] = 0;
        TEST.SPI_data[1] = 0;
        TEST.SPI_data[2] = 0;
        TEST.SPI_data[3] = 0;
        if (Memory_Write_Data(0x00, &TEST.SPI_data, 4)) {
            ZHAL_UART_Driver_Put_Data ("Write started\n", sizeof("Write started\n"));
            ZHAL_UART_Driver_Send_Data();
            TEST.SPI_status = 0;
        }
        break;
    }
}


#if 0
// Timer test with blocking delay
void TIMER_TEST () {
    uint8_t data;

    switch (TEST.TimerStatus) {
    case 0:
        Output_Expander_Pin(1, 1);
        SW_Timer_Init(&TEST.Timer, 1000);
        TEST.TimerStatus++;
        break;
    case 1:
        if (SW_Timer_Is_Timed_Out(&TEST.Timer)) {
            Output_Expander_Pin(1, 0);
            TEST.TimerStatus++;
        }
        break;
    case 2:
        SW_Timer_Blocking_Delay_1ms(2000);
        TEST.TimerStatus = 0;
        break;
    }
}
#endif

// Timer test WITHOUT blocking delay
void TIMER_TEST () {
    uint8_t data;

    switch (TEST.TimerStatus) {
    case 0:
        Output_Expander_Pin(1, 1);
        SW_Timer_Init(&TEST.Timer, 1000);
        TEST.TimerStatus++;
        break;
    case 1:
        if (SW_Timer_Is_Timed_Out(&TEST.Timer)) {
            Output_Expander_Pin(1, 0);
            SW_Timer_Init(&TEST.Timer, 2000);
            TEST.TimerStatus++;
        }
        break;
    case 2:
        if (SW_Timer_Is_Timed_Out(&TEST.Timer)) {
            TEST.TimerStatus = 0;
        }
        break;
    }
}

void WAIT () {
    uint8_t count = 100;

    while (count != 0) {
        count--;
    }
}

#if 0
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
#endif

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
            BUTTON.IsPressed = 1;
            return (1);
        }
        break;
    }
    BUTTON.IsPressed = 0;
    return (0);
}

void CROSSPOINT_SWITCH_TEST () {
        if (BUTTON_DETECTION(ZHAL_GPIO_B, GPIO_PIN_1)) {
            MATRIX.Status++;
            if (MATRIX.Status > 4) {
                MATRIX.Status = 0;
            }
#if 0
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
}


void MIDI_TEST () {
#if 0
        // MIDI test - when the button is pressed, sends note on, waits some time and sends note off
        switch (TEST.MIDI_status) {
        case 1:
            TEST.MIDI_status++;
            TEST.MIDI_timeout = 100000;
            TEST.MIDI_Message[0] = 0x90;
            TEST.MIDI_Message[1] = 0x3C;
            TEST.MIDI_Message[2] = 0x7F;
            ZHAL_UART_Driver_Put_Data(TEST.MIDI_Message, 3);
            ZHAL_UART_Driver_Send_Data();
            break;
        case 2:
            TEST.MIDI_timeout--;
            if (TEST.MIDI_timeout == 0) {
                TEST.MIDI_Message[0] = 0x80;
                TEST.MIDI_Message[1] = 0x3C;
                TEST.MIDI_Message[2] = 0x00;
                ZHAL_UART_Driver_Put_Data(TEST.MIDI_Message, 3);
                ZHAL_UART_Driver_Send_Data();
                TEST.MIDI_status = 0;
            }
            break;
        }
#endif
#if 0
        // MIDI loopback test - receives data and resends it
        switch (TEST.MIDI_status) {
        case 0:
            if ((ZHAL_UART_Driver_Peek (&data) != 0) && (data == 0x0D)) {
                TEST.MIDI_status = 1;
            }
            break;
        case 1:
            TEST.SizeReceived = ZHAL_UART_Driver_Get_Data(TEST.MessageReceived, sizeof(TEST.MessageReceived));

            ZHAL_UART_Driver_Put_Data (TEST.MessageReceived, TEST.SizeReceived);
            ZHAL_UART_Driver_Send_Data();

            for (i = 0; i < sizeof(TEST.MessageReceived); i++) {
                TEST.MessageReceived[i] = 0;
            }
            TEST.SizeReceived = 0;
            TEST.DataReceived = 0;
            TEST.MIDI_status = 0;
            break;
        }
#endif
}

void KEYPAD_CALLBACK (uint8_t row, uint8_t column, Keypad_Transition_t status) {
    uint8_t data;

    if ((row == 0) && (column == 0)) {
        TEST.KeypadLed[0] = ~TEST.KeypadLed[0];
        Output_Expander_Pin(0, TEST.KeypadLed[0]);
    } else if ((row == 0) && (column == 1)) {
        TEST.KeypadLed[1] = ~TEST.KeypadLed[1];
        Output_Expander_Pin(2, TEST.KeypadLed[1]);
    } else if ((row == 0) && (column == 2)) {
        TEST.KeypadLed[2] = ~TEST.KeypadLed[2];
        Output_Expander_Pin(3, TEST.KeypadLed[2]);
    } else if ((row == 1) && (column == 0)) {
        TEST.KeypadLed[3] = ~TEST.KeypadLed[3];
        Output_Expander_Pin(4, TEST.KeypadLed[3]);
    }

}


void KEYPAD_TEST () {
    Keypad_Button_Config_t config;

    switch (TEST.KeypadStatus) {
    case 0:
        config.Mode = KEYPAD_BTN_RELEASED_ONLY;
        config.Type = KEYPAD_BTN_NORMALLY_CLOSED;
        Keypad_Config_Button (0, 0, KEYPAD_CALLBACK, config);
        Keypad_Config_Button (0, 1, KEYPAD_CALLBACK, config);
        Keypad_Config_Button (0, 2, KEYPAD_CALLBACK, config);
        Keypad_Config_Button (1, 0, KEYPAD_CALLBACK, config);

        TEST.KeypadLed[0] = 255;
        TEST.KeypadLed[1] = 255;
        TEST.KeypadLed[2] = 255;
        TEST.KeypadLed[3] = 255;

        TEST.KeypadStatus++;
        break;
    case 1:

        break;
    }
}

