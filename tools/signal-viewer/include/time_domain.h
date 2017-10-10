/**
 * @file time_domain.h
 * @brief TODO.
 *
 */




#ifndef TIME_DOMAIN_H
#define	TIME_DOMAIN_H




#include <time.h>




//
typedef unsigned long long timestamp_ms;




/**
 * @brief Convert seconds to milliseconds. [unsigned long long]
 *
 */
#define SEC_TO_MILLI(time) (time * 1000ULL)


/**
 * @brief Convert milliseconds to seconds. [unsigned long long]
 *
 */
#define MILLI_TO_SEC(time) (time / 1000ULL)


/**
 * @brief Convert nanoseconds to milliseconds. [unsigned long long]
 *
 */
#define NANO_TO_MILLI(time) (time / 1000000ULL)


//
#define DAY_SUNDAY (0)
#define DAY_MONDAY (1)
#define DAY_TUESDAY (2)
#define DAY_WEDNESDAY (3)
#define DAY_THURSDAY (4)
#define DAY_FRIDAY (5)
#define DAY_SATURDAY (6)
#define DAY_MONDAY_THROUGH_FRIDAY (7)
#define DAY_SATURDAY_AND_SUNDAY (8)




//
void time_sleep_ms(
        const timestamp_ms interval );


//
timestamp_ms time_get_timestamp( void );


//
timestamp_ms time_get_monotonic_timestamp( void );


//
timestamp_ms time_get_since(
        const timestamp_ms const value );


//
timestamp_ms time_get_since_monotonic(
        const timestamp_ms const value );


//
timestamp_ms time_get_until(
        const timestamp_ms const value );


//
timestamp_ms time_get_until_monotonic(
        const timestamp_ms const value );


//
struct tm *time_get_localtime(
        const timestamp_ms const timestamp );


//
unsigned long time_get_week_day( void );


//
unsigned long time_get_hour( void );


//
unsigned long time_get_minute( void );




#endif	/* TIME_DOMAIN_H */
