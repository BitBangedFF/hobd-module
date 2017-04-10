/**
 * @file obd.h
 * @brief TODO.
 *
 */


#ifndef OBD_H
#define	OBD_H


#include <stdint.h>


uint8_t obd_init( void );


void obd_disable( void );


void obd_enable( void );


uint8_t obd_update( void );


#endif	/* OBD_H */
