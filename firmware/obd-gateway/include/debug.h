/**
 * @file debug.h
 * @brief TODO.
 *
 */


#ifndef DEBUG_H
#define DEBUG_H


#ifdef BUILD_TYPE_DEBUG
    #include <inttypes.h>
    #include "board.h"
    #include "uart_lib.h"
    #define DEBUG_PUTS( x ) \
    { \
        Uart_select( DEBUG_UART ); \
        uart_put_string( (uint8_t*) x ); \
    }
    #define DEBUG_PRINTF( ... ) \
    { \
        Uart_select( DEBUG_UART ); \
        uart_mini_printf( __VA_ARGS__ ); \
    }
#else
    #define DEBUG_PUTS( x )
    #define DEBUG_PRINTF( ... )
#endif


#endif // DEBUG_H
