/*
 * 
 *
 */


#include <ez8.h>
#include <string.h>
#include "zhal.h"

#include "bsp.h"
#include "sw_timer.h"
#include "output_expander.h"
#include "memory.h"


void SPI_Driver_Callback (void);

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

PB1 - bot�o

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

    uint8_t UART_Rx_count;
    uint8_t UART_inserted_bytes;
    uint8_t Flag;

    uint8_t SPI_Debug[ZHAL_SPI_FIFO_SIZE];

    uint8_t TimerStatus;
    SW_Timer_t Timer;
} TEST;


char Message[] = "Loopback test!\r\n";


struct {
    uint32_t Timeout;
    uint8_t Status;
    bool_t IsPressed;
} BUTTON;

const ZHAL_SPI_Driver_Config_t SPI_Driver_Output_Expander = {
    CS_SHIFT_PORT,
    CS_SHIFT_PIN,
    SPI_Driver_Callback,
    NULL
};

const ZHAL_SPI_Driver_Config_t SPI_Driver_Memory = {
    CS_MEM_PORT,
    CS_MEM_PIN,
    SPI_Driver_Callback,
    NULL
};

void UART_Driver_Tx_Callback (void) {

}

void UART_Driver_Rx_Callback (void) {
    TEST.UART_Rx_count++;
}


void SPI_Driver_Callback (void) {
    TEST.SPI_debug++;
    if (TEST.SPI_status != 0) {
        TEST.SPI_status++;
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
    ZHAL_UART_Driver_Config_t uart_config = {
        38400,
        UART_Driver_Tx_Callback,
        UART_Driver_Rx_Callback
    };

    // Crosspoint switch addresses
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_A, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_6 | GPIO_PIN_7, &gpio_config);
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_C, GPIO_PIN_0 | GPIO_PIN_6 | GPIO_PIN_7, &gpio_config);

    // Crosspoint switch serial
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_C, GPIO_PIN_4 | GPIO_PIN_5, &gpio_config);
    ZHAL_GPIO_Config_Pin(ZHAL_GPIO_B, GPIO_PIN_0, &gpio_config);


    // bot�o
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

    // Send byte through SPI when button is pressed
    switch (TEST.SPI_status) {
    case 0:
        if (ZHAL_UART_Driver_Peek(&data) != 0) {
            if (data == 'W') { // write to memory
                ZHAL_UART_Driver_Get_Data(&data, 1);
                TEST.SPI_status = 5;
            } else if (data == 'R') {  // read from memory
                ZHAL_UART_Driver_Get_Data(&data, 1);
                TEST.SPI_status = 8;
            } else if (data == 'S') {  // write to shift register
                ZHAL_UART_Driver_Get_Data(&data, 1);
                TEST.SPI_status = 1;
            } else if (data == 'E') {
                ZHAL_UART_Driver_Get_Data(&data, 1);
                TEST.SPI_status = 12;
            } else if (data == 'I') {
                ZHAL_UART_Driver_Get_Data(&data, 1);
                TEST.SPI_status = 11;
            }
        }
        break;

    case 1:
        if (ZHAL_UART_Driver_Get_Data(&data, 1) != 0) {
            ZHAL_SPI_Driver_Put_Data(&data, 1);
            ZHAL_SPI_Driver_Send_Data(&SPI_Driver_Output_Expander);

            TEST.SPI_status++;
        }
        break;
    case 2:
        break;
    case 3:
        ZHAL_UART_Driver_Put_Data ("Message sent!\n", 14);
        ZHAL_UART_Driver_Send_Data();

        TEST.SPI_status = 0;
        break;

    case 5: // FRAM write enable
        TEST.SPI_data_to_send[0] = 0x06;    // WREN
        ZHAL_SPI_Driver_Put_Data(TEST.SPI_data_to_send, 1);
        ZHAL_SPI_Driver_Send_Data(&SPI_Driver_Memory);

        TEST.SPI_status++;
        break;
    case 6: // waiting write enable
        break;
    case 7: // FRAM write
        TEST.SPI_data_to_send[0] = 0x02;    // WRITE
        TEST.SPI_data_to_send[1] = 0x00;    // addr high
        TEST.SPI_data_to_send[2] = 0x00;    // addr low
        TEST.SPI_data_to_send[3] = 0x01;    // data
        TEST.SPI_data_to_send[4] = 0x02;    // data
        TEST.SPI_data_to_send[5] = 0x03;    // data
        TEST.SPI_data_to_send[6] = 0x04;    // data
        ZHAL_SPI_Driver_Put_Data(TEST.SPI_data_to_send, 7);
        ZHAL_SPI_Driver_Send_Data(&SPI_Driver_Memory);

        TEST.SPI_status = 0;
        break;

    case 8: // FRAM read
#if 1
        TEST.SPI_data_to_send[0] = 0x03;    // READ
        TEST.SPI_data_to_send[1] = 0x00;    // addr high
        TEST.SPI_data_to_send[2] = 0x00;    // addr low
        TEST.SPI_data_to_send[3] = 0xFF;    // dummy
        TEST.SPI_data_to_send[4] = 0xFF;    // dummy
        TEST.SPI_data_to_send[5] = 0xFF;    // dummy
        TEST.SPI_data_to_send[6] = 0xFF;    // dummy

        ZHAL_SPI_Driver_Put_Data(TEST.SPI_data_to_send, 7);
        ZHAL_SPI_Driver_Send_Data(&SPI_Driver_Memory);
#else
        if (Memory_Read_Start(4, 0x00)) {

            ZHAL_UART_Driver_Put_Data ("Read started\n", sizeof("Read started\n"));
            ZHAL_UART_Driver_Send_Data();
            TEST.SPI_status++;
        }
#endif
        break;
    case 9:
#if 0
        if (ZHAL_UART_Driver_Peek(&data) != 0) {
            if (data == 'T') {
                ZHAL_UART_Driver_Get_Data(&data, 1);

                ZHAL_UART_Driver_Put_Data ("\nTX: ", 5);
                data = Memory_Get_Status(0);
                ZHAL_UART_Driver_Put_Data (&data, 1);
                SPI_Driver_Get_Status (3, &TEST.SPI_Debug);
                ZHAL_UART_Driver_Put_Data (&TEST.SPI_Debug, ZHAL_SPI_FIFO_SIZE);
                ZHAL_UART_Driver_Send_Data();
            } else if (data == 'R') {
                ZHAL_UART_Driver_Get_Data(&data, 1);

                ZHAL_UART_Driver_Put_Data ("\nRX: ", 5);
                data = Memory_Get_Status(0);
                ZHAL_UART_Driver_Put_Data (&data, 1);
                SPI_Driver_Get_Status (4, &TEST.SPI_Debug);
                ZHAL_UART_Driver_Put_Data (&TEST.SPI_Debug, ZHAL_SPI_FIFO_SIZE);
                ZHAL_UART_Driver_Send_Data();
            }
        }

        if (Memory_Read_Data(&data_buf)) {
            ZHAL_UART_Driver_Put_Data ("Read finished\n", sizeof("Read finished\n"));
            ZHAL_UART_Driver_Send_Data();

            ZHAL_UART_Driver_Put_Data (&data_buf, 4);
            ZHAL_UART_Driver_Send_Data();

            TEST.SPI_status = 0;
        }
#endif
        break;
    case 10:
        // send all data to UART
        i = ZHAL_SPI_Driver_Peek(NULL);  // get the number of bytes available

        while (i > 0) {
            ZHAL_SPI_Driver_Get_Data(&data, 1);
            ZHAL_UART_Driver_Put_Data (&data, 1);
            i--;
        }
        ZHAL_UART_Driver_Send_Data();

        TEST.SPI_status = 0;
        break;

    case 11:
        data = ZHAL_SPI_Driver_Peek(NULL);
        ZHAL_UART_Driver_Put_Data (&data, 1);
        ZHAL_SPI_Driver_Peek(&data);
        ZHAL_UART_Driver_Put_Data (&data, 1);
        ZHAL_UART_Driver_Put_Data (&TEST.SPI_debug, 1);

        ZHAL_UART_Driver_Send_Data();

        TEST.SPI_status = 0;
        break;

    case 12: // FRAM write enable
        TEST.SPI_data_to_send[0] = 0x06;    // WREN
        ZHAL_SPI_Driver_Put_Data(TEST.SPI_data_to_send, 1);
        ZHAL_SPI_Driver_Send_Data(&SPI_Driver_Memory);

        TEST.SPI_status++;
        break;
    case 13: // waiting write enable
        break;
    case 14: // FRAM write
        TEST.SPI_data_to_send[0] = 0x02;    // WRITE
        TEST.SPI_data_to_send[1] = 0x00;    // addr high
        TEST.SPI_data_to_send[2] = 0x00;    // addr low
        TEST.SPI_data_to_send[3] = 0x00;    // data
        TEST.SPI_data_to_send[4] = 0x00;    // data
        TEST.SPI_data_to_send[5] = 0x00;    // data
        TEST.SPI_data_to_send[6] = 0x00;    // data
        ZHAL_SPI_Driver_Put_Data(TEST.SPI_data_to_send, 7);
        ZHAL_SPI_Driver_Send_Data(&SPI_Driver_Memory);

        TEST.SPI_status = 0;
        break;
    }
}


void TIMER_TEST () {
    uint8_t data;

    switch (TEST.TimerStatus) {
    case 0:
        data = 0;
        ZHAL_UART_Driver_Put_Data ("\nMem stat: ", sizeof("\nMem stat: "));
        data = Memory_Get_Status(0);
        ZHAL_UART_Driver_Put_Data (&data, 1);
        data = SPI_Driver_Get_Status (1, NULL);
        ZHAL_UART_Driver_Put_Data (&data, 1);
        data = SPI_Driver_Get_Status (2, NULL);
        ZHAL_UART_Driver_Put_Data (&data, 1);
        ZHAL_UART_Driver_Send_Data();
        //Output_Expander_Pin(1, 1);
        SW_Timer_Init(&TEST.Timer, 1000);
        TEST.TimerStatus++;
        break;
    case 1:
        if (SW_Timer_Is_Timed_Out(&TEST.Timer)) {
            data = 0xFF;
            ZHAL_UART_Driver_Put_Data ("\nMem stat: ", sizeof("\nMem stat: "));
            data = Memory_Get_Status(0);
            ZHAL_UART_Driver_Put_Data (&data, 1);
            data = SPI_Driver_Get_Status (1, NULL);
            ZHAL_UART_Driver_Put_Data (&data, 1);
            data = SPI_Driver_Get_Status (2, NULL);
            ZHAL_UART_Driver_Put_Data (&data, 1);
            ZHAL_UART_Driver_Send_Data();
            //Output_Expander_Pin(1, 0);
            TEST.TimerStatus++;
        }
        break;
    case 2:
        SW_Timer_Blocking_Delay_1ms(2000);
        TEST.TimerStatus = 0;
        break;
    }
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
            BUTTON.IsPressed = 1;
            return (1);
        }
        break;
    }
    BUTTON.IsPressed = 0;
    return (0);
}


void main () {
    uint8_t i;
    uint8_t data;

    ZHAL_Init();

    MCU_INIT(); //rotinas de inicializa��o

    SPI_CONFIG();

#if 0
    Output_Expander_Init();
    Output_Expander_Data(0xFF);
#endif

    Memory_Init();

    MATRIX.Status = 0;
#if 0
    CROSSPOINT_SWITCH_CONTROL(0, 0, 1);
#endif
    
    while (1){

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

        UART_TEST();

        SPI_TEST();

        TIMER_TEST();

        Output_Expander_Task();

        Memory_Task();

#if 0

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
}







