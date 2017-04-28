/**
 * @file sd.h
 * @brief TODO.
 *
 */


#ifndef SD_H
#define	SD_H


#include <stdint.h>


void sd_init( void );


void sd_open( void );


void sd_close( void );


void sd_flush( void );


void sd_write(
        const uint8_t * const data,
        const uint16_t size,
        const uint16_t count );


void sd_puts(
        const char * const data );


#endif	/* TIME_H */
