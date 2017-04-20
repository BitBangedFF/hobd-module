/**
 * @file gear_position.c
 * @brief TODO.
 *
 * Table calculators and information:
 *   - http://www.gearingcommander.com/
 *   - http://woodsware.aciwebs.com/gears/
 *   - http://www.datamc.org/2013/06/03/gear-position/
 *   - http://www.datamc.org/2016/08/07/tire-rolling-radius-correcting-wheel-speed/
 *   - https://www.weekend-techdiy.com/motorcycle-datalogger
 *   - http://forum.arduino.cc/index.php?topic=8474.15
 *   - http://fritzing.org/media/fritzing-repo/projects/m/motorcycle-computer-based-on-arduino/code/MCGIPv2_2.pde
 *
 */


#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <stdint.h>
#include <math.h>

#include "board.h"

#include "error.h"
#include "gear_position.h"


static const float FINAL_RATIO_TOLERANCE = 1.0f;

static const float FINAL_RATIOS[] =
{
    33.0f,
    30.0f,
    14.5f,
    12.5f,
    9.667f,
    7.4f
};


static uint8_t comparef(
        const float a,
        const float b,
        const float tolerance )
{
    uint8_t is_within = 0;

    const float diff = fabs(a - b);
    const float a_abs = fabs(a);
    const float b_abs = fabs(b);

    const float largest = (b_abs > a_abs) ? b_abs : a_abs;

    if(diff <= (largest * tolerance))
    {
        is_within = 1;
    }

    return is_within;
}


uint8_t gp_get(
        const uint16_t engine_rpm,
        const uint8_t wheel_speed )
{
    uint8_t gear_pos = HOBD_GEAR_POSITION_UNKNOWN;

    if((engine_rpm != 0) && (wheel_speed != 0))
    {
        const float ratio = ((float) engine_rpm / (float) wheel_speed);

        uint8_t idx;
        for(idx = 0; (idx < HOBD_GEAR_POSITION_COUNT) && (gear_pos == HOBD_GEAR_POSITION_UNKNOWN); idx += 1)
        {
            const uint8_t status = comparef(
                    ratio,
                    FINAL_RATIOS[idx],
                    FINAL_RATIO_TOLERANCE);

            if(status != 0)
            {
                gear_pos = (idx + 1);
            }
        }
    }

    return gear_pos;
}
