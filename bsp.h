/*
 * BSP
 * Board related defines and typedefs
 */

#ifndef BSP_H  // include guard
#define BSP_H

/*
 * System defines
 */

#define BTN0_PORT       ZHAL_GPIO_B     // BTN0
#define BTN0_PIN        GPIO_PIN_4

#define BTN1_PORT       ZHAL_GPIO_B
#define BTN1_PIN        GPIO_PIN_2

#define BTN2_PORT       ZHAL_GPIO_B
#define BTN2_PIN        GPIO_PIN_3

#define BTN3_PORT       ZHAL_GPIO_B     // BTN3
#define BTN3_PIN        GPIO_PIN_1

#define BTN4_PORT       ZHAL_GPIO_B     // BTN4
#define BTN4_PIN        GPIO_PIN_5

    // For keypad usage

    #define ROW0_PORT       ZHAL_GPIO_B     // BTN0
    #define ROW0_PIN        GPIO_PIN_4

    #define ROW1_PORT       ZHAL_GPIO_B
    #define ROW1_PIN        GPIO_PIN_2

    #define ROW2_PORT       ZHAL_GPIO_B
    #define ROW2_PIN        GPIO_PIN_3

    #define COLUMN0_PORT    ZHAL_GPIO_B     // BTN3
    #define COLUMN0_PIN     GPIO_PIN_1

    #define COLUMN1_PORT    ZHAL_GPIO_B
    #define COLUMN1_PIN     GPIO_PIN_5

    #define COLUMN2_PORT    ZHAL_GPIO_A
    #define COLUMN2_PIN     GPIO_PIN_1

    #if 0
    #define COLUMN3_PORT       ZHAL_GPIO_A
    #define COLUMN3_PIN        GPIO_PIN_0
    #endif


#define SPI_MISO_PORT   ZHAL_GPIO_C
#define SPI_MISO_PIN    GPIO_PIN_1

#define SPI_MOSI_PORT   ZHAL_GPIO_C
#define SPI_MOSI_PIN    GPIO_PIN_4

#define SPI_SCK_PORT    ZHAL_GPIO_C
#define SPI_SCK_PIN     GPIO_PIN_5

#define CS_SWITCH_PORT  ZHAL_GPIO_B
#define CS_SWITCH_PIN   GPIO_PIN_0

#define CS_SHIFT_PORT   ZHAL_GPIO_A
#define CS_SHIFT_PIN    GPIO_PIN_2

#define CS_MEM_PORT     ZHAL_GPIO_C
#define CS_MEM_PIN      GPIO_PIN_2

#define USB_DETECT_PORT ZHAL_GPIO_A
#define USB_DETECT_PIN  GPIO_PIN_3

#define C_SW1_PORT      ZHAL_GPIO_A
#define C_SW1_PIN       GPIO_PIN_6

#define C_SW2_PORT      ZHAL_GPIO_C
#define C_SW2_PIN       GPIO_PIN_7

#define C_SW3_PORT      ZHAL_GPIO_A
#define C_SW3_PIN       GPIO_PIN_0

#define C_SW4_PORT      ZHAL_GPIO_A
#define C_SW4_PIN       GPIO_PIN_7

#define C_SW5_PORT      ZHAL_GPIO_C
#define C_SW5_PIN       GPIO_PIN_0

#define C_SW6_PORT      ZHAL_GPIO_C
#define C_SW6_PIN       GPIO_PIN_3

#define C_SW7_PORT      ZHAL_GPIO_C
#define C_SW7_PIN       GPIO_PIN_6

// "uint8_t enum" below is a Zilog C Compiler language extension - The enumeration data type is defined as int as per ANSI C. The C-Compiler provides language extensions to specify the enumeration data type to be other than int to save space.
typedef uint8_t enum {
    SW_IN_1 = 0,
    SW_IN_2,
    FROM_EFF_1,
    FROM_EFF_2,
    FROM_EFF_3,
    FROM_EFF_11,
    FROM_EFF_4,
    FROM_EFF_5,
    FROM_EFF_6,
    FROM_EFF_7,
    FROM_EFF_8,
    FROM_EFF_9,
    FROM_EFF_10,
    GND
} Crosspoint_Signals_X_t;

typedef uint8_t enum {
    SW_OUT_1 = 0,
    TO_EFF_1,
    TO_EFF_2,
    TO_EFF_3,
    TO_EFF_4,
    TO_EFF_5,
    TO_EFF_6,
    SW_OUT_2_TO_EFF_7
} Crosspoint_Signals_Y_t;


#endif // BSP
