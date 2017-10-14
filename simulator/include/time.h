/**
 * @file time.h
 * @brief TODO.
 *
 */

#ifndef TIME_H
#define TIME_H

#include <stdint.h>

#define TIME_SIGNAL_INTERVAL_MS (500UL)

void time_init(void);

void time_delay_ms(
        const uint16_t ms);

uint32_t time_get_ms(void);

uint8_t time_get_timer(void);

void time_clear_timer(void);

uint8_t time_get_and_clear_timer(void);

#endif  /* TIME_H */
