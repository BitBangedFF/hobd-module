/**
 * @file debug.h
 * @brief TODO.
 *
 */

#ifndef DEBUG_H
#define DEBUG_H

#ifdef BUILD_TYPE_DEBUG
    #include "debug_uart.h"
    #define debug_init() debug_uart_init()
    #define debug_puts(s) debug_uart_puts((uint8_t*) s)
    #define debug_printf(f, ...)
#else
    #define debug_init()
    #define debug_puts(s)
    #define debug_printf(f, ...)
#endif

#endif /* DEBUG_H */
