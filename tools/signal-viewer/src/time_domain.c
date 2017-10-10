/**
 * @file time_domain.c
 * @brief TODO.
 *
 */




#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#include "time_domain.h"




// *****************************************************
// static global types/macros
// *****************************************************




// *****************************************************
// static global data
// *****************************************************




// *****************************************************
// static declarations
// *****************************************************

//
static timestamp_ms get_timestamp(
        const clockid_t clk_id );




// *****************************************************
// static definitions
// *****************************************************

//
static timestamp_ms get_timestamp(
        const clockid_t clk_id )
{
    struct timespec time;
    timestamp_ms timestamp;

    // get time
    clock_gettime( clk_id, &time );

    // convert to milliseconds
    timestamp = (timestamp_ms) SEC_TO_MILLI( (timestamp_ms) time.tv_sec );

    // convert nanosecond remainder to milliseconds
    timestamp += (timestamp_ms) NANO_TO_MILLI( (timestamp_ms) time.tv_nsec );

    return timestamp;
}




// *****************************************************
// public definitions
// *****************************************************

//
void time_sleep_ms(
        const timestamp_ms interval )
{
    if( interval != 0 )
    {
        struct timespec req = { .tv_sec = 0, .tv_nsec = 0 };
        struct timespec rem = { .tv_sec = 0, .tv_nsec = 0 };
        const time_t sec = (time_t) (interval / 1000ULL);

        // subtract seconds
        const timestamp_ms rtime = interval - (sec * 1000UL);

        // get remainder
        req.tv_sec = sec;
        req.tv_nsec = (long int) (rtime * 1000000ULL);

        // sleep, account for interrupts
        while( nanosleep( &req, &rem ) == -1 )
        {
            // swap
            req.tv_sec = rem.tv_sec;
            req.tv_nsec = rem.tv_nsec;

            // error
            if( errno == EINVAL )
            {
                break;
            }
        }
    }
}


//
timestamp_ms time_get_timestamp( void )
{
    return get_timestamp( CLOCK_REALTIME );
}


//
timestamp_ms time_get_monotonic_timestamp( void )
{
    return get_timestamp( CLOCK_MONOTONIC );
}


//
timestamp_ms time_get_since(
        const timestamp_ms const value )
{
    timestamp_ms delta = 0;

    const timestamp_ms now = time_get_timestamp();

    if( now > value )
    {
        delta = (now - value);
    }

    return delta;
}


//
timestamp_ms time_get_since_monotonic(
        const timestamp_ms const value )
{
    timestamp_ms delta = 0;

    const timestamp_ms now = time_get_monotonic_timestamp();

    if( now > value )
    {
        delta = (now - value);
    }

    return delta;
}


//
timestamp_ms time_get_until(
        const timestamp_ms const value )
{
    timestamp_ms delta = 0;

    const timestamp_ms now = time_get_timestamp();

    if( value > now )
    {
        delta = (value - now);
    }

    return delta;
}


//
timestamp_ms time_get_until_monotonic(
        const timestamp_ms const value )
{
    timestamp_ms delta = 0;

    const timestamp_ms now = time_get_monotonic_timestamp();

    if( value > now )
    {
        delta = (value - now);
    }

    return delta;
}


//
struct tm *time_get_localtime(
        const timestamp_ms const timestamp )
{
    // convert milliseconds to seconds
    const time_t time_sec = (time_t) MILLI_TO_SEC( timestamp );

    return localtime( &time_sec );
}


//
unsigned long time_get_week_day( void )
{
    const struct tm * const date = time_get_localtime( time_get_timestamp() );

    return (unsigned long) date->tm_wday;
}


//
unsigned long time_get_hour( void )
{
    const struct tm * const date = time_get_localtime( time_get_timestamp() );

    return (unsigned long) date->tm_hour;
}


//
unsigned long time_get_minute( void )
{
    const struct tm * const date = time_get_localtime( time_get_timestamp() );

    return (unsigned long) date->tm_min;
}
