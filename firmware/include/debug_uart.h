/**
 * @file debug_uart.h
 * @brief TODO.
 *
 */

#ifndef DEBUG_UART_H
#define	DEBUG_UART_H

#include <stdint.h>

void debug_uart_init(void);

void debug_uart_deinit(void);

uint8_t debug_uart_test_hit(void);

uint8_t debug_uart_getc(void);

void debug_uart_putc(
        const uint8_t data);

void debug_uart_puts(
        const uint8_t *data);

#endif	/* DEBUG_UART_H */
