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
#include "button.h"
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
} Program_Change_Step_t;

typedef enum {
    PRG_CRTE_INIT = 0,
    PRG_CRTE_INIT_LED,
    PRG_CRTE_INIT_LED_BLINK,
    PRG_CRTE_PROGRAM_CHANGE,
    PRG_CRTE_EXECUTING,
    PRG_CRTE_SAVE,
    PRG_CRTE_OK_LED,
    PRG_CRTE_OK_LED_BLINK,
    PRG_CRTE_CANCEL_LED,
    PRG_CRTE_CANCEL_LED_BLINK
} Program_Create_Step_t;

typedef enum {
    PRG_UNDEFINED = 0,
    PRG_EFF_1,
    PRG_EFF_2,
    PRG_EFF_3,
    PRG_EFF_4,
    PRG_EFF_5,
    PRG_EFF_6
} Program_Effect_t;

typedef enum {
    BTN_1_PRESS = 1,
    BTN_2_PRESS,
    BTN_3_PRESS,
    BTN_4_PRESS,
    BTN_1_LONG_PRESS,
    BTN_2_LONG_PRESS,
    BTN_3_LONG_PRESS,
    BTN_4_LONG_PRESS
} Button_Flag_t;

static struct {
    uint8_t Status;
    uint8_t LedBlinkCounter;
    uint8_t InitStatus;
    SW_Timer_t LedTimer;
    Program_Change_Step_t ProgramChangeStep;
    Button_Flag_t ButtonFlag;
    uint8_t DefaultConfig;
    uint8_t CurrentProgram; // from 0 to SWBOARD_MAX_PROGRAMS - 1

    uint8_t LedStep;
    LED_config_t LedConfig;
    uint8_t LedCount;

    struct {
        Program_Create_Step_t Step;
        SW_Timer_t Timer;
        Program_Effect_t CurrentEffect;
        Program_Effect_t Sequence[SWBOARD_MAX_EFFECTS];
        uint8_t SequenceIndex;  // from 0 to SWBOARD_MAX_EFFECTS - 1
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



/*
 * Switchboard_Default_Config
 * Erases whole FRAM, creates and saves the default program configuration
 * Program 1 - effect 1, Program 2 - effect 2, Program 3 - effect 3
 */
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
 * Turns on LEDs sequentially after program change, from LED_CFG_CONTINUOUS to LED_CFG_BLINK_6, at 500 ticks interval between each one
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
 * First effect - 1 blink, second effect - 2 blinks
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
                SW_Timer_Init(&Switchboard.LedTimer, 2000);
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




static void Switchboard_Button_Callback (uint8_t button, Button_Transition_t status) {

    if (status == BUTTON_BTN_PRESSED) {
        if (button == 0) {
            Switchboard.ButtonFlag = BTN_1_PRESS;
        } else if (button == 1) {
            Switchboard.ButtonFlag = BTN_2_PRESS;
        } else if (button == 2) {
            Switchboard.ButtonFlag = BTN_3_PRESS;
        } else if (button == 3) {
            Switchboard.ButtonFlag = BTN_4_PRESS;
        }
    } else if (status == BUTTON_BTN_KEPT_PRESSED) {
        if (button == 0) {
            Switchboard.ButtonFlag = BTN_1_LONG_PRESS;
        } else if (button == 1) {
            Switchboard.ButtonFlag = BTN_2_LONG_PRESS;
        } else if (button == 2) {
            Switchboard.ButtonFlag = BTN_3_LONG_PRESS;
        } else if (button == 3) {
            Switchboard.ButtonFlag = BTN_4_LONG_PRESS;
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

        for (i = 0; (i < SWBOARD_MAX_EFFECTS && Switchboard.ProgramCreate.Sequence[i] != PRG_UNDEFINED); i++) {
            switch (Switchboard.ProgramCreate.Sequence[i]) {
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
 * Switchboard_Program_Change
 * Inits the crosspoint switch, open all switches and ground outputs, close desired switches, unground outputs and closes the crosspoint switch module
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
 * Switchboard_Program_Create
 * Standalone program creation: up to SWBOARD_MAX_EFFECTS effects can be arranged sequentially, using UP/DOWN/OK/CANCEL buttons
 * UP/DOWN: navigates between available effects
 * OK: insert current effect into the sequence. If pressed for more than X seconds, saves the program into nonvolatile memory and finishes the program creation
 * CANCEL: Pressed for more than X seconds, finishes the program creation without saving
 * A timeout is also used to finish the program creation, if the user doesn't press any button in Y seconds
 */
static bool_t Switchboard_Program_Create () {
    bool_t finished = FALSE;
    uint8_t i;
    Program_Effect_t effect_aux;
    Program_Effect_t last_effect;

    switch (Switchboard.ProgramCreate.Step) {
    case PRG_CRTE_INIT:
    default:
        Debug_Message("Program create\r\n", sizeof("Program create\r\n") - 1);

        SW_Timer_Init(&Switchboard.ProgramCreate.Timer, 30000);

        // initializes program creation variables, starting with all effects disabled
        Switchboard.ProgramCreate.SequenceIndex = 0;
        for (i = 0; i < SWBOARD_MAX_EFFECTS; i++) {
            Switchboard.ProgramCreate.Sequence[i] = PRG_UNDEFINED;
        }
        Switchboard.ProgramCreate.CurrentEffect = PRG_UNDEFINED;
        Switchboard_Map_Signal_Route();
        for (i = 0; i < SWBOARD_LEDS; i++) {
            Program.LED[i] = LED_CFG_OFF;
        }

        LED_All_Off();
        Switchboard.LedStep = 0;

        Switchboard.ProgramCreate.Step = PRG_CRTE_INIT_LED;
        break;
    case PRG_CRTE_INIT_LED: // LED blinking
        SW_Timer_Init(&Switchboard.LedTimer, 500);
        LED_Change_Status(0, LED_ON);
        LED_Change_Status(1, LED_ON);
        LED_Change_Status(2, LED_ON);
        LED_Change_Status(3, LED_ON);
        LED_Change_Status(4, LED_ON);
        LED_Change_Status(5, LED_ON);
        Switchboard.ProgramCreate.Step = PRG_CRTE_INIT_LED_BLINK;
        break;
    case PRG_CRTE_INIT_LED_BLINK: // LED blinking
        if (SW_Timer_Is_Timed_Out(&Switchboard.LedTimer)) {
            LED_All_Off();
            Switchboard.ProgramCreate.Step = PRG_CRTE_PROGRAM_CHANGE;
        }
        break;
    case PRG_CRTE_PROGRAM_CHANGE:
        if (Switchboard_Program_Change()) {
            Switchboard.ProgramCreate.Step = PRG_CRTE_EXECUTING;
        }
        break;
    case PRG_CRTE_EXECUTING:
        LED_Control_Program_Creation();

        switch (Switchboard.ButtonFlag) {
        case BTN_1_PRESS: // OK
            if ((Switchboard.ProgramCreate.SequenceIndex < SWBOARD_MAX_EFFECTS) && (Switchboard.ProgramCreate.CurrentEffect != PRG_UNDEFINED)) {
                Switchboard.ProgramCreate.Sequence[Switchboard.ProgramCreate.SequenceIndex] = Switchboard.ProgramCreate.CurrentEffect;

                // creates the LED configuration
                // If CurrentEffect is PRG_EFF_2 (2) and is the second effect added (SequenceIndex == 1), so Program.LED[1] = LED_CFG_BLINK_2 (3)
                Program.LED[Switchboard.ProgramCreate.CurrentEffect - 1] = Switchboard.ProgramCreate.SequenceIndex + 2;

                Debug_Message("Effect added\r\n", sizeof("Effect added\r\n") - 1);

                Switchboard.ProgramCreate.SequenceIndex++;
                Switchboard.ProgramCreate.CurrentEffect = PRG_UNDEFINED;
            }

            SW_Timer_Init(&Switchboard.ProgramCreate.Timer, 30000);
            break;
        case BTN_2_PRESS: // down
            effect_aux = Switchboard.ProgramCreate.CurrentEffect;
            last_effect = Switchboard.ProgramCreate.CurrentEffect;
            // checks if any downward effect is available
            while (effect_aux > PRG_EFF_1) {
                effect_aux--;
                if (Switchboard_Is_Effect_Available(effect_aux)) {
                    Switchboard.ProgramCreate.CurrentEffect = effect_aux;
                    Switchboard.ProgramCreate.Sequence[Switchboard.ProgramCreate.SequenceIndex] = effect_aux;
                    Switchboard_Map_Signal_Route();
                    Switchboard.ProgramCreate.Step = PRG_CRTE_PROGRAM_CHANGE;
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
        case BTN_3_PRESS: // up
            effect_aux = Switchboard.ProgramCreate.CurrentEffect;
            last_effect = Switchboard.ProgramCreate.CurrentEffect;
            // checks if any upward effect is available
            while (effect_aux < SWBOARD_MAX_EFFECTS) {
                effect_aux++;
                if (Switchboard_Is_Effect_Available(effect_aux)) {
                    Switchboard.ProgramCreate.CurrentEffect = effect_aux;
                    Switchboard.ProgramCreate.Sequence[Switchboard.ProgramCreate.SequenceIndex] = effect_aux;
                    Switchboard_Map_Signal_Route();
                    Switchboard.ProgramCreate.Step = PRG_CRTE_PROGRAM_CHANGE;
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
        case BTN_1_LONG_PRESS: // OK - kept pressed
            strcpy(Program.Name, "Program ");
            Program.Name[8] = '1' + Switchboard.CurrentProgram;
            Program.Name[9] = 0;

            Switchboard.ProgramCreate.Step = PRG_CRTE_SAVE;
            break;
        case BTN_4_LONG_PRESS: // cancel - kept pressed - the original program configuration will be read back from memory in Switchboard_Task
            Debug_Message("Program creation cancelled\r\n", sizeof("Program creation cancelled\r\n") - 1);
            Switchboard.ProgramCreate.Step = PRG_CRTE_CANCEL_LED;
            break;
        }
        Switchboard.ButtonFlag = 0;

        // timeout if no button is pressed
        if (SW_Timer_Is_Timed_Out(&Switchboard.ProgramCreate.Timer)) {
#warning "To do - save in memory OR NOT"

            Debug_Message("Program creation timeout\r\n", sizeof("Program creation timeout\r\n") - 1);
            Switchboard.ProgramCreate.Step = PRG_CRTE_CANCEL_LED;
        }
        break;
    case PRG_CRTE_SAVE:
        if (Memory_Write_Data(SWBOARD_PROGRAM_FRAM_ADDR(Switchboard.CurrentProgram), (uint8_t *) &Program, SWBOARD_PROGRAM_SIZE)) {
            Debug_Message("Program creation finished\r\n", sizeof("Program creation finished\r\n") - 1);
            Switchboard.ProgramCreate.Step = PRG_CRTE_OK_LED;
        }
        break;
    case PRG_CRTE_OK_LED: // LED blinking - OK
        SW_Timer_Init(&Switchboard.LedTimer, 100);
        LED_All_Off();
        Switchboard.LedBlinkCounter = 0;
        Switchboard.ProgramCreate.Step = PRG_CRTE_OK_LED_BLINK;
        break;
    case PRG_CRTE_OK_LED_BLINK: // LED blinking
        if (SW_Timer_Is_Timed_Out(&Switchboard.LedTimer)) {
            SW_Timer_Init(&Switchboard.LedTimer, 100);
            LED_Change_Status(Switchboard.LedBlinkCounter, LED_ON);
            Switchboard.LedBlinkCounter++;
            if (Switchboard.LedBlinkCounter >= 6) {
                LED_All_Off();
                Switchboard.ProgramCreate.Step = PRG_CRTE_INIT;
                finished = 1;
            }
        }
        break;
    case PRG_CRTE_CANCEL_LED: // LED blinking - cancel
        SW_Timer_Init(&Switchboard.LedTimer, 100);
        LED_Change_Status(0, LED_ON);
        LED_Change_Status(1, LED_ON);
        LED_Change_Status(2, LED_ON);
        LED_Change_Status(3, LED_ON);
        LED_Change_Status(4, LED_ON);
        LED_Change_Status(5, LED_ON);
        Switchboard.LedBlinkCounter = 5;
        Switchboard.ProgramCreate.Step = PRG_CRTE_CANCEL_LED_BLINK;
        break;
    case PRG_CRTE_CANCEL_LED_BLINK: // LED blinking
        if (SW_Timer_Is_Timed_Out(&Switchboard.LedTimer)) {
            SW_Timer_Init(&Switchboard.LedTimer, 100);
            LED_Change_Status(Switchboard.LedBlinkCounter, LED_OFF);
            if (Switchboard.LedBlinkCounter == 0) {
                Switchboard.ProgramCreate.Step = PRG_CRTE_INIT;
                finished = 1;
            } else {
                Switchboard.LedBlinkCounter--;
            }
        }
        break;
    }

    return (finished);
}



/*
 * Switchboard_Init
 * Must be continuously called until returns TRUE
 */
static bool_t Switchboard_Init () {
    Button_Config_t config;
    bool_t status = FALSE;

    switch (Switchboard.InitStatus) {
    case 0:
        config.Mode = BUTTON_BTN_PRESSED_ONLY;
        config.Type = BUTTON_BTN_NORMALLY_CLOSED;
        Button_Config (0, Switchboard_Button_Callback, config);
        Button_Config (1, Switchboard_Button_Callback, config);
        Button_Config (2, Switchboard_Button_Callback, config);
        Button_Config (3, Switchboard_Button_Callback, config);

        Switchboard.LedBlinkCounter = 0;
        Switchboard.InitStatus++;
        break;
    case 1:
        // reads what was the last program used to reload it
        if (Memory_Read_Data(SWBOARD_CURR_PROGRAM_FRAM_ADDR, &Switchboard.CurrentProgram, 1)){
            if (Switchboard.CurrentProgram >= SWBOARD_MAX_PROGRAMS) {
                Switchboard.CurrentProgram = 0;
            }
            Switchboard.InitStatus++;
        }
        break;
    case 2:
        // reads the program configuration
        if (Memory_Read_Data(SWBOARD_PROGRAM_FRAM_ADDR(Switchboard.CurrentProgram), (uint8_t *) &Program, SWBOARD_PROGRAM_SIZE)) {
            Switchboard.InitStatus++;
        }
        break;
    case 3: // LED blinking
        SW_Timer_Init(&Switchboard.LedTimer, 100);
        LED_Change_Status(Switchboard.LedBlinkCounter, LED_ON);
        Switchboard.InitStatus++;
        break;
    case 4:  // LED blinking
        if (SW_Timer_Is_Timed_Out(&Switchboard.LedTimer)) {
            LED_Change_Status(Switchboard.LedBlinkCounter, LED_OFF);
            Switchboard.LedBlinkCounter++;
            if (Switchboard.LedBlinkCounter >= 6) {
                Switchboard.InitStatus++;
            } else {
                Switchboard.InitStatus = 3;
            }
        }
        break;
    case 5:
        // reloads the last program
        if (Switchboard_Program_Change()) {
            Switchboard.LedStep = 0;
            Debug_Message("Initialization finished\r\n", sizeof("Initialization finished\r\n") - 1);
            status = TRUE;
        }
        break;
    default:
        Switchboard.InitStatus = 0;
        break;
    }

    return (status);
}


/*
 * Switchboard_Task
 * Must be called periodically from main application
 */
void Switchboard_Task () {
    uint8_t i;

    switch (Switchboard.Status) {
    case 0:
        if (Switchboard_Init()) {
            Switchboard.Status = 1;
        }
        break;
    case 1:
        LED_Control();

        switch (Switchboard.ButtonFlag) {
        case BTN_1_PRESS:
        case BTN_2_PRESS:
        case BTN_3_PRESS:
        case BTN_4_PRESS:
            Debug_Message("Button pressed!\r\n", sizeof("Button pressed!\r\n") - 1);
            Switchboard.CurrentProgram = Switchboard.ButtonFlag - 1;
            Switchboard.ButtonFlag = 0;
            Switchboard.Status = 6;
            break;
        case BTN_1_LONG_PRESS:
        case BTN_2_LONG_PRESS:
        case BTN_3_LONG_PRESS:
        case BTN_4_LONG_PRESS:
            Switchboard.CurrentProgram = Switchboard.ButtonFlag - 5;
            Switchboard.ButtonFlag = 0;
            Switchboard.Status = 5;
            break;
        }
        break;

    case 5:
        if (Switchboard_Program_Create()) {
            Switchboard.Status = 6;
        }
        break;

    // Program change
    case 6:
        if (Memory_Read_Data(SWBOARD_PROGRAM_FRAM_ADDR(Switchboard.CurrentProgram), (uint8_t *) &Program, SWBOARD_PROGRAM_SIZE)) {
            Switchboard.Status++;
        }
        break;
    case 7:
        if (Switchboard_Program_Change()) {
            Switchboard.Status++;
        }
        break;
    case 8:
        if (Memory_Write_Data(SWBOARD_CURR_PROGRAM_FRAM_ADDR, &Switchboard.CurrentProgram, 1)) {
            Switchboard.LedStep = 0;
            Switchboard.Status = 1;
        }
        break;

    case 20:
        if (Switchboard_Default_Config()) {
            Switchboard.Status = 1;
        }
        break;
    }
}
