/**
 * @file gear_position.h
 * @brief TODO.
 *
 */


#ifndef GEAR_POSITION_H
#define	GEAR_POSITION_H


#include <stdint.h>

#include "hobd.h"


uint8_t gp_get(
        const uint16_t engine_rpm,
        const uint8_t wheel_speed );


#endif	/* GEAR_POSITION_H */
