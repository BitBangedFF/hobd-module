/**
 * @file obd.h
 * @brief TODO.
 *
 */


#ifndef OBD_H
#define	OBD_H


#include <stdint.h>


void obd_init( void );


void obd_disable( void );


void obd_enable( void );


void obd_update( void );


void obd_update_timeout( void );


#endif	/* OBD_H */
