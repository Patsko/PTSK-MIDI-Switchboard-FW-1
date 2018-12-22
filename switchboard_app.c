/*
 * Switchboard application
 *
 * Application created considering usage with guitar effects
 * 6 effects, mono, without paralleling/stereo capabilities
 * 6 LEDs, 4 buttons
 */

#include <ez8.h>
#include <stdio.h>
#include <string.h>
#include "zhal.h"
#include "bsp.h"
#include "sw_timer.h"
#include "output_expander.h"
#include "crosspoint_switch.h"
#include "memory.h"
#include "keypad.h"
#include "switchboard_app.h"
#include "debug.h"


// "uint8_t enum" below is a Zilog C Compiler language extension - The enumeration data type is defined as int as per ANSI C. The C-Compiler provides language extensions to specify the enumeration data type to be other than int to save space.
typedef uint8_t enum {
    LED_ON = 0,
    LED_OFF
} LED_status_t;

typedef uint8_t enum {
    LED_CFG_OFF = 0,
    LED_CFG_CONTINUOUS,
    LED_CFG_BLINK_1,
    LED_CFG_BLINK_2,
    LED_CFG_BLINK_3,
    LED_CFG_BLINK_4,
    LED_CFG_BLINK_5,
    LED_CFG_BLINK_6
} LED_config_t;

typedef enum {
    PRG_CHNG_INIT = 0,
    PRG_CHNG_OPEN_ALL_SWITCHES,
    PRG_CHNG_CLOSE_SWITCHES,
    PRG_CHNG_SET_OUTPUT,
    PRG_CHNG_FINISH
} Program_Change_t;

typedef enum {
    PRG_UNDEFINED = 0,
    PRG_EFF_1,
    PRG_EFF_2,
    PRG_EFF_3,
    PRG_EFF_4,
    PRG_EFF_5,
    PRG_EFF_6
} Program_Effect_t;

static struct {
    uint8_t Status;
    uint8_t InitCount;
    SW_Timer_t LedTimer;
    uint8_t ProgramChangeStep;
    uint8_t ButtonFlag;
    uint8_t DefaultConfig;
    uint8_t CurrentProgram;
    uint8_t LedStep;
    LED_config_t LedConfig;
    uint8_t LedCount;

    struct {
        uint8_t Step;
        SW_Timer_t Timer;
        Program_Effect_t CurrentEffect;
        Program_Effect_t Sequence[SWBOARD_MAX_EFFECTS];
        uint8_t SequenceIndex;
    } ProgramCreate;
} Switchboard;

static struct {
    char Name[SWBOARD_PROGRAM_NAME_SIZE];
    struct {
        Crosspoint_Signals_X_t X;
        Crosspoint_Signals_Y_t Y;
    } Switch[SWBOARD_SIGNAL_ROUTE_MAX];
    LED_config_t LED[SWBOARD_LEDS];     // each value on the array is related to a specific LED
} Program;




static bool_t Switchboard_Default_Config () {
    uint8_t i;
    bool_t status = FALSE;

    switch (Switchboard.DefaultConfig) {
    case 0:
        if (Crosspoint_Switch_Close()) {
            Memory_Init();
            Debug_Message("Default config\r\n", sizeof("Default config\r\n") - 1);
            Switchboard.DefaultConfig++;
        }
        break;
    case 1:
        // erase whole FRAM
        if (Memory_Erase(0x0000, 2048)) {
            Switchboard.DefaultConfig++;
        }
        break;
    case 2:
        // Resets the current signal route
        for (i = 0; i < SWBOARD_SIGNAL_ROUTE_MAX; i++) {
            Program.Switch[i].X = 0xFF;
            Program.Switch[i].Y = 0xFF;
        }
        Program.Switch[0].X = SW_IN_1;
        Program.Switch[0].Y = TO_EFF_1;
        Program.Switch[1].X = FROM_EFF_1;
        Program.Switch[1].Y = SW_OUT_1;
        strcpy(Program.Name, "Default 1");
        for (i = 0; i < SWBOARD_LEDS; i++) {
            Program.LED[i] = LED_CFG_OFF;
        }
        Program.LED[0] = LED_CFG_BLINK_1;

        Switchboard.DefaultConfig++;
        break;
    case 3:
        if (Memory_Write_Data(SWBOARD_PROGRAM_FRAM_ADDR(0), (uint8_t *) &Program, SWBOARD_PROGRAM_SIZE)) {
            Switchboard.DefaultConfig++;
        }
        break;
    case 4:
        Program.Switch[0].X = SW_IN_1;
        Program.Switch[0].Y = TO_EFF_2;
        Program.Switch[1].X = FROM_EFF_2;
        Program.Switch[1].Y = SW_OUT_1;
        strcpy(Program.Name, "Default 2");
        for (i = 0; i < SWBOARD_LEDS; i++) {
            Program.LED[i] = LED_CFG_OFF;
        }
        Program.LED[1] = LED_CFG_BLINK_1;

        Switchboard.DefaultConfig++;
        break;
    case 5:
        if (Memory_Write_Data(SWBOARD_PROGRAM_FRAM_ADDR(1), (uint8_t *) &Program, SWBOARD_PROGRAM_SIZE)) {
            Switchboard.DefaultConfig++;
        }
        break;
    case 6:
        Program.Switch[0].X = SW_IN_1;
        Program.Switch[0].Y = TO_EFF_3;
        Program.Switch[1].X = FROM_EFF_3;
        Program.Switch[1].Y = SW_OUT_1;
        strcpy(Program.Name, "Default 3");
        for (i = 0; i < SWBOARD_LEDS; i++) {
            Program.LED[i] = LED_CFG_OFF;
        }
        Program.LED[2] = LED_CFG_BLINK_1;

        Switchboard.DefaultConfig++;
        break;
    case 7:
        if (Memory_Write_Data(SWBOARD_PROGRAM_FRAM_ADDR(2), (uint8_t *) &Program, SWBOARD_PROGRAM_SIZE)) {
            Switchboard.DefaultConfig++;
        }
        break;
    case 8:
        Debug_Message("Default config finished\r\n", sizeof("Default config finished\r\n") - 1);
        Switchboard.DefaultConfig = 0;
        status = TRUE;
        break;
    }
    return (status);
}

/*
 * led - from 0 to 5
 */
static void LED_Change_Status (uint8_t led, LED_status_t status) {

    if (led < 6) {
        // LED turns on with logic 0
        Output_Expander_Pin (led, status);
    }
}

static void LED_All_Off () {
    uint8_t i;

    for (i = 0; i < SWBOARD_LEDS; i++) {
        Output_Expander_Pin (i, LED_OFF);
    }
}

/*
 * LED control during normal operation
 * Turns on LEDs sequentially during program change, from LED_CFG_CONTINUOUS to LED_CFG_BLINK_6, at 500 ticks interval between each one
 */
static void LED_Control () {
    uint8_t i;

    switch (Switchboard.LedStep) {
    case 0:
        LED_All_Off();
        Switchboard.LedConfig = LED_CFG_CONTINUOUS;
        Switchboard.LedStep++;
        break;
    case 1:
        for (i = 0; i < SWBOARD_LEDS; i++) {
            if (Program.LED[i] == Switchboard.LedConfig) {  // Program.LED[0] holds the LED 0 configuration, for example
                LED_Change_Status(i, LED_ON);
            }
        }
        SW_Timer_Init(&Switchboard.LedTimer, 500);
        Switchboard.LedStep++;
        break;
    case 2:
        if (SW_Timer_Is_Timed_Out(&Switchboard.LedTimer)) {
            if (Switchboard.LedConfig < LED_CFG_BLINK_6) {
                Switchboard.LedConfig++;
                Switchboard.LedStep = 1;
            } else {
                Switchboard.LedStep++;
            }
        }
        break;
    case 3: // finished
        break;
    default:
        Switchboard.LedStep = 0;
        break;
    }
}

/*
 * LED blinking control during program creation
 */
static void LED_Control_Program_Creation () {
    uint8_t i;

    switch (Switchboard.LedStep) {
    case 0:
        Switchboard.LedCount = 0;
        Switchboard.LedStep++;
        break;
    case 1:
        for (i = Switchboard.LedCount; i < SWBOARD_LEDS; i++) {
            if (Switchboard.ProgramCreate.Sequence[i] != PRG_UNDEFINED) {
                LED_Change_Status((Switchboard.ProgramCreate.Sequence[i] - 1), LED_ON);   // PRG_EFF_1 turns led 0 on
            }
        }
        SW_Timer_Init(&Switchboard.LedTimer, 500);
        Switchboard.LedStep++;
        break;
    case 2:
        if (SW_Timer_Is_Timed_Out(&Switchboard.LedTimer)) {
            for (i = Switchboard.LedCount; i < SWBOARD_LEDS; i++) {
                if (Switchboard.ProgramCreate.Sequence[i] != PRG_UNDEFINED) {
                    LED_Change_Status((Switchboard.ProgramCreate.Sequence[i] - 1), LED_OFF);   // PRG_EFF_1 turns led 0 off
                }
            }
            SW_Timer_Init(&Switchboard.LedTimer, 500);
            Switchboard.LedStep++;
        }
        break;
    case 3:
        if (SW_Timer_Is_Timed_Out(&Switchboard.LedTimer)) {
            Switchboard.LedCount++;
            if ((Switchboard.LedCount < SWBOARD_MAX_EFFECTS) && (Switchboard.ProgramCreate.Sequence[Switchboard.LedCount] != PRG_UNDEFINED)) {
                Switchboard.LedStep = 1;
            } else {
                SW_Timer_Init(&Switchboard.LedTimer, 1000);
                Switchboard.LedStep++;
            }
        }
        break;
    case 4:
        if (SW_Timer_Is_Timed_Out(&Switchboard.LedTimer)) {
            Switchboard.LedStep = 0;
        }
        break;
    default:
        Switchboard.LedStep = 0;
        break;
    }
}




static void Switchboard_Button_Callback (uint8_t row, uint8_t column, Keypad_Transition_t status) {

    if (status == KEYPAD_BTN_PRESSED) {
        if ((row == 0) && (column == 0)) {
            Switchboard.ButtonFlag = 1;
        } else if ((row == 1) && (column == 0)) {
            Switchboard.ButtonFlag = 2;
        } else if ((row == 2) && (column == 0)) {
            Switchboard.ButtonFlag = 3;
        } else if ((row == 0) && (column == 1)) {
            Switchboard.ButtonFlag = 4;
        }
    } else if (status == KEYPAD_BTN_KEPT_PRESSED) {
        if ((row == 0) && (column == 0)) {
            Switchboard.ButtonFlag = 5;
        } else if ((row == 1) && (column == 0)) {
            Switchboard.ButtonFlag = 6;
        } else if ((row == 2) && (column == 0)) {
            Switchboard.ButtonFlag = 7;
        } else if ((row == 0) && (column == 1)) {
            Switchboard.ButtonFlag = 8;
        }
    }
}


/*
 * Checks if the desired effect is already being used or not, during program creation
 */
static bool_t Switchboard_Is_Effect_Available (Program_Effect_t effect) {
    uint8_t i;

    for (i = 0; i < SWBOARD_MAX_EFFECTS; i++) {
        if (Switchboard.ProgramCreate.Sequence[i] == effect) {
            return (FALSE);
        }
    }
    return (TRUE);
}


/*
 * Creates the signal route based on the desired effects order during standalone program creation
 */
static void Switchboard_Map_Signal_Route () {
    uint8_t i;

    // Resets the current signal route
    for (i = 0; i < SWBOARD_SIGNAL_ROUTE_MAX; i++) {
        Program.Switch[i].X = 0xFF;
        Program.Switch[i].Y = 0xFF;
    }

    if (Switchboard.ProgramCreate.CurrentEffect == PRG_UNDEFINED) { // if no effect was inserted yet, the switchboard will be on bypass mode (IN -> OUT)
        Program.Switch[0].X = SW_IN_1;
        Program.Switch[0].Y = SW_OUT_1;
    } else {
        Program.Switch[0].X = SW_IN_1;

        for (i = 0; i < SWBOARD_MAX_EFFECTS; i++) {
            switch (Switchboard.ProgramCreate.Sequence[i]) {
            case PRG_UNDEFINED:
            default:
                break;
            case PRG_EFF_1:
                Program.Switch[i].Y = TO_EFF_1;
                Program.Switch[i + 1].X = FROM_EFF_1;
                break;
            case PRG_EFF_2:
                Program.Switch[i].Y = TO_EFF_2;
                Program.Switch[i + 1].X = FROM_EFF_2;
                break;
            case PRG_EFF_3:
                Program.Switch[i].Y = TO_EFF_3;
                Program.Switch[i + 1].X = FROM_EFF_3;
                break;
            case PRG_EFF_4:
                Program.Switch[i].Y = TO_EFF_4;
                Program.Switch[i + 1].X = FROM_EFF_4;
                break;
            case PRG_EFF_5:
                Program.Switch[i].Y = TO_EFF_5;
                Program.Switch[i + 1].X = FROM_EFF_5;
                break;
            case PRG_EFF_6:
                Program.Switch[i].Y = TO_EFF_6;
                Program.Switch[i + 1].X = FROM_EFF_6;
                break;
            }
        }

        Program.Switch[i].Y = SW_OUT_1;
    }
}




/*
 * Must be continuously called until returns TRUE
 */
static bool_t Switchboard_Program_Change () {
    uint8_t i;
    bool_t finished = FALSE;

    switch (Switchboard.ProgramChangeStep) {
    case PRG_CHNG_INIT:
    default:
        if (Output_Expander_Close() && Memory_Close()) {
            Crosspoint_Switch_Init();

            // ground outputs
            Crosspoint_Switch_Set (GND, SW_OUT_1, 1);
            Crosspoint_Switch_Set (GND, SW_OUT_2_TO_EFF_7, 1);
            Switchboard.ProgramChangeStep = PRG_CHNG_OPEN_ALL_SWITCHES;
        }
        break;
    case PRG_CHNG_OPEN_ALL_SWITCHES:
        // open all switches keeping outputs grounded
        Crosspoint_Switch_Open_Switches();

        Crosspoint_Switch_Set (GND, SW_OUT_1, 1);
        Crosspoint_Switch_Set (GND, SW_OUT_2_TO_EFF_7, 1);

        Switchboard.ProgramChangeStep = PRG_CHNG_CLOSE_SWITCHES;
        break;
    case PRG_CHNG_CLOSE_SWITCHES:
        // close the switches keeping outputs grounded
        for (i = 0; i < SWBOARD_SIGNAL_ROUTE_MAX; i++) {
            if ((Program.Switch[i].X == 0xFF) || (Program.Switch[i].Y == 0xFF)) {
                break;  // breaks the for loop
            } else {
                Crosspoint_Switch_Set (Program.Switch[i].X, Program.Switch[i].Y, 1);
            }
        }

        Crosspoint_Switch_Set (GND, SW_OUT_1, 1);
        Crosspoint_Switch_Set (GND, SW_OUT_2_TO_EFF_7, 1);

        Switchboard.ProgramChangeStep = PRG_CHNG_SET_OUTPUT;
        break;
    case PRG_CHNG_SET_OUTPUT:
        Crosspoint_Switch_Set (GND, SW_OUT_1, 0);
        Crosspoint_Switch_Set (GND, SW_OUT_2_TO_EFF_7, 0);

        Switchboard.ProgramChangeStep = PRG_CHNG_FINISH;
        break;
    case PRG_CHNG_FINISH:
        if (Crosspoint_Switch_Close()) {
            Output_Expander_Init();
            Memory_Init();

            finished = TRUE;
            Switchboard.ProgramChangeStep = 0;
        }
        break;
    }

    return (finished);
}


/*
 * Standalone program creation: up to SWBOARD_MAX_EFFECTS effects can be arranged sequentially, using UP/DOWN/OK/CANCEL buttons
 * UP/DOWN: navigates between available effects
 * OK: insert current effect into the sequence. If pressed for more than X seconds, saves the program into nonvolatile memory and finishes the program creation
 * CANCEL: finishes the program creation without saving
 * A timeout is also used to finish the program creation, if the user doesn't press any button in X seconds
 */
static bool_t Switchboard_Program_Create () {
    bool_t finished = FALSE;
    uint8_t i;
    Program_Effect_t last_effect;

    switch (Switchboard.ProgramCreate.Step) {
    case 0:
    default:
        Debug_Message("Program create\r\n", sizeof("Program create\r\n") - 1);

        SW_Timer_Init(&Switchboard.ProgramCreate.Timer, 30000);

        Switchboard.ProgramCreate.SequenceIndex = 0;
        for (i = 0; i < SWBOARD_MAX_EFFECTS; i++) {
            Switchboard.ProgramCreate.Sequence[i] = PRG_UNDEFINED;
        }
        Switchboard.ProgramCreate.CurrentEffect = PRG_UNDEFINED;
        Switchboard_Map_Signal_Route();

        LED_All_Off();
        Switchboard.LedStep = 0;

        Switchboard.ProgramCreate.Step = 1;
        break;
    case 1:
        if (Switchboard_Program_Change()) {
            Switchboard.ProgramCreate.Step = 2;
        }
        break;
    case 2:
        switch (Switchboard.ButtonFlag) {
        case 1: // OK
            if (Switchboard.ProgramCreate.SequenceIndex < SWBOARD_MAX_EFFECTS) {
                // only adds into the effects order if it isn't already there
                if (Switchboard_Is_Effect_Available(Switchboard.ProgramCreate.CurrentEffect)) {
                    Switchboard.ProgramCreate.Sequence[Switchboard.ProgramCreate.SequenceIndex] = Switchboard.ProgramCreate.CurrentEffect;
                    Switchboard.ProgramCreate.SequenceIndex++;

                    Debug_Message("Effect added\r\n", sizeof("Effect added\r\n") - 1);
                }
            }

            SW_Timer_Init(&Switchboard.ProgramCreate.Timer, 30000);
            break;
        case 2: // down
            i = Switchboard.ProgramCreate.CurrentEffect;
            last_effect = Switchboard.ProgramCreate.CurrentEffect;
            // checks if any downward effect is available
            while (Switchboard.ProgramCreate.CurrentEffect > PRG_EFF_1) {
                i--;
                if (Switchboard_Is_Effect_Available(i)) {
                    Switchboard.ProgramCreate.CurrentEffect = i;
                    Switchboard_Map_Signal_Route();
                    Switchboard.ProgramCreate.Step = 1;
                    break;  // breaks the loop
                }
            }

            if (last_effect != PRG_UNDEFINED) {
                LED_Change_Status((last_effect - 1), LED_OFF);
            }
            LED_Change_Status((Switchboard.ProgramCreate.CurrentEffect - 1), LED_ON);   // PRG_EFF_1 turns led 0 on

            Debug_Message("Down\r\n", sizeof("Down\r\n") - 1);

            SW_Timer_Init(&Switchboard.ProgramCreate.Timer, 30000);
            break;
        case 3: // up
            i = Switchboard.ProgramCreate.CurrentEffect;
            last_effect = Switchboard.ProgramCreate.CurrentEffect;
            // checks if any upward effect is available
            while (Switchboard.ProgramCreate.CurrentEffect < SWBOARD_MAX_EFFECTS) {
                i++;
                if (Switchboard_Is_Effect_Available(i)) {
                    Switchboard.ProgramCreate.CurrentEffect = i;
                    Switchboard_Map_Signal_Route();
                    Switchboard.ProgramCreate.Step = 1;
                    break;  // breaks the loop
                }
            }

            if (last_effect != PRG_UNDEFINED) {
                LED_Change_Status((last_effect - 1), LED_OFF);
            }
            LED_Change_Status((Switchboard.ProgramCreate.CurrentEffect - 1), LED_ON);   // PRG_EFF_1 turns led 0 on

            Debug_Message("Up\r\n", sizeof("Up\r\n") - 1);

            SW_Timer_Init(&Switchboard.ProgramCreate.Timer, 30000);
            break;
        case 5: // OK - kept pressed
#warning "To do - save in memory"

            Debug_Message("Program creation finished\r\n", sizeof("Program creation finished\r\n") - 1);
            Switchboard.ProgramCreate.Step = 0;
            finished = 1;
            break;
        case 8: // cancel - kept pressed
#warning "To do - read original from memory"

            Debug_Message("Program creation cancelled\r\n", sizeof("Program creation cancelled\r\n") - 1);
            Switchboard.ProgramCreate.Step = 0;
            finished = 1;
            break;
        }
        Switchboard.ButtonFlag = 0;

        // timeout if no button is pressed
        if (SW_Timer_Is_Timed_Out(&Switchboard.ProgramCreate.Timer)) {
#warning "To do - save in memory OR NOT"

            Debug_Message("Program creation timeout\r\n", sizeof("Program creation timeout\r\n") - 1);
            Switchboard.ProgramCreate.Step = 0;
            finished = 1;
        }
        break;
    }

    return (finished);
}




/*
 * Switchboard_Task
 * Must be called periodically from main application
 */
void Switchboard_Task () {
    Keypad_Button_Config_t config;
    uint8_t i;

    switch (Switchboard.Status) {
    case 0:
    default:
        config.Mode = KEYPAD_BTN_PRESSED_ONLY;
        config.Type = KEYPAD_BTN_NORMALLY_CLOSED;
        Keypad_Config_Button (0, 0, Switchboard_Button_Callback, config);
        Keypad_Config_Button (1, 0, Switchboard_Button_Callback, config);
        Keypad_Config_Button (2, 0, Switchboard_Button_Callback, config);
        Keypad_Config_Button (0, 1, Switchboard_Button_Callback, config);

        Switchboard.InitCount = 0;
        Switchboard.Status++;
        break;
    case 1:
        SW_Timer_Init(&Switchboard.LedTimer, 100);
        LED_Change_Status(Switchboard.InitCount, LED_ON);
        Switchboard.Status++;
        break;
    case 2:
        if (SW_Timer_Is_Timed_Out(&Switchboard.LedTimer)) {
            LED_Change_Status(Switchboard.InitCount, LED_OFF);
            Switchboard.InitCount++;
            if (Switchboard.InitCount >= 6) {
                Switchboard.Status++;

                Debug_Message("Initialization finished\r\n", sizeof("Initialization finished\r\n") - 1);
            } else {
                Switchboard.Status = 1;
            }
        }
        break;
    case 3:
        LED_Control();

        switch (Switchboard.ButtonFlag) {
        case 1:
        case 2:
        case 3:
        case 4:
            Debug_Message("Button pressed!\r\n", sizeof("Button pressed!\r\n") - 1);
            Switchboard.CurrentProgram = Switchboard.ButtonFlag - 1;
            Switchboard.ButtonFlag = 0;
            Switchboard.Status = 6;
#warning "To do - read program from memory"
            break;
        case 5:
        case 6:
        case 7:
        case 8:
            Switchboard.CurrentProgram = Switchboard.ButtonFlag - 5;
            Switchboard.ButtonFlag = 0;
            Switchboard.Status = 5;
            break;
        }
        break;
    case 4:
        if (Switchboard_Program_Change()) {
            Switchboard.Status = 3;
        }
        break;
    case 5:
        if (Switchboard_Program_Create()) {
            Switchboard.LedStep = 0;
            Switchboard.Status = 3;
        }
        LED_Control_Program_Creation();
        break;

    case 6:
        if (Memory_Read_Data(SWBOARD_PROGRAM_FRAM_ADDR(Switchboard.CurrentProgram), (uint8_t *) &Program, SWBOARD_PROGRAM_SIZE)) {
            Switchboard.Status++;
        }
        break;
    case 7:
        if (Switchboard_Program_Change()) {
            Switchboard.LedStep = 0;
            Switchboard.Status = 3;
        }
        break;
#if 0
    case 8:
        LED_All_Off();
        for (i = 0; i < SWBOARD_LEDS; i++) {
            if (Program.LED[i] != LED_CFG_OFF) {
                LED_Change_Status(i, LED_ON);
            }
        }
        Switchboard.Status = 3;
        break;
#endif
#if 0
#warning "Test"

    case 6:
        // Resets the current signal route
        for (i = 0; i < SWBOARD_SIGNAL_ROUTE_MAX; i++) {
            Program.Switch[i].X = 0xFF;
            Program.Switch[i].Y = 0xFF;
        }

        switch (Switchboard.ButtonFlag) {
        case 1:
            Program.Switch[0].X = SW_IN_1;
            Program.Switch[0].Y = TO_EFF_1;
            Program.Switch[1].X = FROM_EFF_1;
            Program.Switch[1].Y = SW_OUT_1;
            break;
        case 2:
            Program.Switch[0].X = SW_IN_1;
            Program.Switch[0].Y = TO_EFF_2;
            Program.Switch[1].X = FROM_EFF_2;
            Program.Switch[1].Y = SW_OUT_1;
            break;
        case 3:
            Program.Switch[0].X = SW_IN_1;
            Program.Switch[0].Y = TO_EFF_3;
            Program.Switch[1].X = FROM_EFF_3;
            Program.Switch[1].Y = SW_OUT_1;
            break;
        }
        Switchboard.Status++;
        break;
    case 7:
        if (Switchboard_Program_Change()) {
            Switchboard.Status++;
        }
        break;
    case 8:
        switch (Switchboard.ButtonFlag) {
        case 1:
            LED_Change_Status(0, LED_ON);
            LED_Change_Status(1, LED_OFF);
            LED_Change_Status(2, LED_OFF);
            break;
        case 2:
            LED_Change_Status(0, LED_OFF);
            LED_Change_Status(1, LED_ON);
            LED_Change_Status(2, LED_OFF);
            break;
        case 3:
            LED_Change_Status(0, LED_OFF);
            LED_Change_Status(1, LED_OFF);
            LED_Change_Status(2, LED_ON);
            break;
        }

        Switchboard.ButtonFlag = 0;
        Switchboard.Status = 3;
        break;
#endif
    case 9:
        if (Switchboard_Default_Config()) {
            Switchboard.Status = 3;
        }
        break;
#if 0
#warning "Basic switching test"
    case 6:
        if (Output_Expander_Close() && Memory_Close()) {
            Crosspoint_Switch_Init();
            Crosspoint_Switch_Open_Switches();
            switch (Switchboard.ButtonFlag) {
            case 1:
                Crosspoint_Switch_Set (SW_IN_1, TO_EFF_1, 1);
                Crosspoint_Switch_Set (FROM_EFF_1, SW_OUT_1, 1);
                break;
            case 2:
                Crosspoint_Switch_Set (SW_IN_1, TO_EFF_2, 1);
                Crosspoint_Switch_Set (FROM_EFF_2, SW_OUT_1, 1);
                break;
            case 3:
                Crosspoint_Switch_Set (SW_IN_1, TO_EFF_3, 1);
                Crosspoint_Switch_Set (FROM_EFF_3, SW_OUT_1, 1);
                break;
            }
            Switchboard.Status++;
        }
        break;
    case 7:
        if (Crosspoint_Switch_Close()) {
            Output_Expander_Init();
            Memory_Init();

            switch (Switchboard.ButtonFlag) {
            case 1:
                Output_Expander_Pin(1, 0);
                Output_Expander_Pin(2, 1);
                Output_Expander_Pin(3, 1);
                break;
            case 2:
                Output_Expander_Pin(1, 1);
                Output_Expander_Pin(2, 0);
                Output_Expander_Pin(3, 1);
                break;
            case 3:
                Output_Expander_Pin(1, 1);
                Output_Expander_Pin(2, 1);
                Output_Expander_Pin(3, 0);
                break;
            }

            Switchboard.ButtonFlag = 0;
            Switchboard.Status = 3;
        }
        break;
#endif
    }
}

#warning "Test"
void Switchboard_Test () {
    Switchboard.Status = 9;
}
