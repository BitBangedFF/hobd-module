/**
 * @file gear_position.c
 * @brief TODO.
 *
 * Table calculators and information:
 *   - http://www.gearingcommander.com/
 *   - http://woodsware.aciwebs.com/gears/
 *   - http://www.datamc.org/2013/06/03/gear-position/
 *   - http://www.datamc.org/2016/08/07/tire-rolling-radius-correcting-wheel-speed/
 *
 */


#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/pgmspace.h>

#include "board.h"

#include "error.h"
#include "gear_position.h"


// inputs(wheel_speed_kph, engine_rpm)
// outputs(gear_position)
// constant(gear_ratio[])

// gear ratio = engine rpm / countershaft rpm

// TODO - figure out what to put in the table
// is it really needed?

// [engine_rpm][wheel_speed] = gear_position
// [engine_rpm][wheel_speed] = ratio


static const uint8_t TABLE[2][3] PROGMEM =
{
    {0x00, 0x01, 0x02},
    {0x03, 0x04, 0x05}
};


uint8_t gp_get(
        const uint16_t engine_rpm,
        const uint8_t wheel_speed )
{
    uint8_t gear_pos = GEAR_POSITION_UNKNOWN;

    // drive_rpm = wheel_rpm ?
    // ratio = engine_rpm / wheel_rpm

    // for each gear:
    //   if ratio[gear] == ratio
    //     return gear


    // byte = pgm_read_byte(&(mydata[i][j]));
    // word = pgm_read_word(&(mydata[i][j]));

    return gear_pos;
}