/**
 * @file gear_position.h
 * @brief TODO.
 *
 */


#ifndef GEAR_POSITION_H
#define	GEAR_POSITION_H


#include <stdint.h>


#define GEAR_POSITION_UNKNOWN (0)
#define GEAR_POSITION_1 (1)
#define GEAR_POSITION_2 (2)
#define GEAR_POSITION_3 (3)
#define GEAR_POSITION_4 (4)
#define GEAR_POSITION_5 (5)
#define GEAR_POSITION_6 (6)
#define GEAR_POSITION_COUNT (6)


uint8_t gp_get(
        const uint16_t engine_rpm,
        const uint8_t wheel_speed );


#endif	/* GEAR_POSITION_H */
