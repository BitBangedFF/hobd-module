/**
 * @file time.c
 * @brief TODO.
 *
 */




#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <inttypes.h>

#include "rtc_drv.h"
#include "time.h"




// *****************************************************
// static global types/macros
// *****************************************************




// *****************************************************
// static global data
// *****************************************************




// *****************************************************
// static declarations
// *****************************************************




// *****************************************************
// static definitions
// *****************************************************




// *****************************************************
// public definitions
// *****************************************************

//
void time_sleep_ms(
        const uint16_t interval )
{
    if( interval != 0 )
    {
        delay_ms( interval );
    }
}


//
uint32_t time_get_ms( void )
{
    return (uint32_t) rtc_get_ms();
}


//
uint32_t time_get_delta(
        const uint32_t * const value,
        const uint32_t * const now )
{
    uint32_t delta = 0;

    // check for overflow
    if( (*now) < (*value) )
    {
        // time remainder, prior to the overflow
        delta = (UINT32_MAX - (*value));

        // add time since zero
        delta += (*now);
    }
    else
    {
        // normal delta
        delta = ((*now) - (*value));
    }

    return delta;
}
