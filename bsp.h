/*
 * BSP
 *
 */

#ifndef BSP_H  // include guard
#define BSP_H

/*
 * System defines
 */

#define BTN0_PORT       ZHAL_GPIO_B
#define BTN0_PIN        GPIO_PIN_4

#define BTN1_PORT       ZHAL_GPIO_B
#define BTN1_PIN        GPIO_PIN_2

#define BTN2_PORT       ZHAL_GPIO_B
#define BTN2_PIN        GPIO_PIN_3

#define BTN3_PORT       ZHAL_GPIO_B
#define BTN3_PIN        GPIO_PIN_1

#define BTN4_PORT       ZHAL_GPIO_B
#define BTN4_PIN        GPIO_PIN_5

#define BTN5_PORT       ZHAL_GPIO_A
#define BTN5_PIN        GPIO_PIN_1

#if 0
#define BTN6_PORT       ZHAL_GPIO_A
#define BTN6_PIN        GPIO_PIN_0
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



#endif // BSP
