/**
 * @file debug.h
 * @brief TODO.
 *
 */

#ifndef DEBUG_H
#define DEBUG_H

#ifdef BUILD_TYPE_DEBUG
    #define debug_init()
    #define debug_puts(s)
    #define debug_printf(f, ...)
#else
    #define debug_init()
    #define debug_puts(s)
    #define debug_printf(f, ...)
#endif

#endif /* DEBUG_H */
