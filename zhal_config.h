/*
 * ZHAL
 * Configuration header file, with project-specific settings
 */

#ifndef ZHAL_CONFIG_H  // include guard
#define ZHAL_CONFIG_H


/*
 * Compilation defines
 */

#define ZHAL_USE_GPIO           1
#define ZHAL_USE_TIMER          1
#define ZHAL_USE_UART           1
#define ZHAL_USE_UART_DRIVER    1
#define ZHAL_USE_SPI            1
#define ZHAL_USE_SPI_DRIVER     1

/*
 * System defines
 */

#define SYSTEM_CLOCK            11059200    // MCU clock in Hz

#define WATCHDOG_ENABLE         0
#define WATCHDOG_TIMEOUT        0x400   // Reset default value time-out delay: 0x400

#define SYSTICK_TIMER           ZHAL_TIMER_0

#define ZHAL_UART_FIFO_SIZE     75

#define ZHAL_SPI_FIFO_SIZE      100
#define ZHAL_SPI_BAUD_RATE      50000


#endif // ZHAL_CONFIG_H
