/**
 * @file time.h
 * @brief TODO.
 *
 */




#ifndef TIME_H
#define	TIME_H




#include <inttypes.h>




//
void time_sleep_ms(
        const uint16_t interval );


//
uint32_t time_get_ms( void );


//
uint32_t time_get_delta(
        const uint32_t * const value,
        const uint32_t * const now );




#endif	/* TIME_H */
