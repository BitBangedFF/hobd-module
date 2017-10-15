/**
 * @file time.c
 * @brief TODO.
 *
 */

#include <avr/io.h>
#include <stdint.h>
#include "board.h"
#include "timer8_drv.h"
#include "error.h"
#include "time.h"

#ifndef RTC_SW_TIMER_TICK
#define RTC_SW_TIMER_TICK (50)
#warning "using default RTC_SW_TIMER_TICK 50"
#endif

static volatile uint32_t rtc_tics = 0;
static volatile uint16_t rtc_counter = 0;
static volatile uint32_t rtc_seconds = 0;

static volatile uint32_t timer_counter = 0;
static volatile uint8_t timer_reached = 0;

static uint8_t is_init = FALSE;

ISR(TIMER2_COMP_vect)
{
    rtc_tics += 1;
    rtc_counter += 1;
    timer_counter += 1;

    if(timer_counter == RTC_SW_TIMER_TICK)
    {
        timer_reached = 1;
        timer_counter = 0;
    }

    if(rtc_counter == 1000)
    {
        rtc_seconds += 1;
        rtc_counter = 0;
    }
}

static void delay_ms(
        const uint16_t ms_count)
{
    uint32_t temp;
    uint16_t i;
    uint8_t  j, k;

    // TODO - clean this up

    if(is_init == TRUE)
    {
        disable_interrupt();
        temp = rtc_tics;
        enable_interrupt();

        temp += ((uint32_t) ms_count);

        while (1)
        {
            disable_interrupt();

            if(rtc_tics == temp)
            {
                break;
            }

            if(rtc_tics == ((uint32_t) ms_count))
            {
                // broken overflow handling
                break;
            }

            enable_interrupt();
        }

        enable_interrupt();
    }
    else
    {
        // no RTC, attempt something arbitrary
        for(i = 0; i < ms_count; i += 1)
        {
            for(j = 0; j < (uint8_t) (FOSC/1000); j += 1)
            {
                for(k = 0; k < 90; k += 1);
            }
        }
    }
}

void time_init(void)
{
    disable_interrupt();

    Timer8_clear();

    // wait to let the Xtal stabilize after a power-on
    uint16_t i;
    for(i=0; i < 0xFFFF; i += 1);

    Timer8_overflow_it_disable();
    Timer8_compare_a_it_disable();

    Timer8_set_mode_output_a(TIMER8_COMP_MODE_NORMAL);
    Timer8_set_waveform_mode(TIMER8_WGM_CTC_OCR);

    Timer8_2_system_clk();

    // prescaler=128, timer on
    // tic interval: ((1/16000000)*128*MAGIC_NUMBER) sec = 1.00000000 msec
    Timer8_set_compare_a(125-1);
    Timer8_set_clock(TIMER8_2_CLKIO_BY_128);

    while(Timer8_2_update_busy() != 0);

    Timer8_clear_compare_a_it();
    Timer8_compare_a_it_enable();

    rtc_tics = 0;
    rtc_counter = 0;
    rtc_seconds = 0;
    timer_counter = 0;
    timer_reached = 0;

    is_init = TRUE;
    enable_interrupt();
}

void time_sleep_ms(
        const uint16_t interval)
{
    if(interval != 0)
    {
        delay_ms(interval);
    }
}

uint32_t time_get_ms(void)
{
    disable_interrupt();
    const uint32_t timestamp = rtc_tics;
    enable_interrupt();

    return timestamp;
}

uint32_t time_get_seconds(void)
{
    disable_interrupt();
    const uint32_t seconds = rtc_seconds;
    enable_interrupt();

    return seconds;
}

uint32_t time_get_delta(
        const uint32_t * const value,
        const uint32_t * const now)
{
    uint32_t delta;

    // check for overflow
    if((*now) < (*value))
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

uint8_t time_get_timer(void)
{
    return timer_reached;
}

void time_clear_timer(void)
{
    timer_reached = 0;
}

uint8_t time_get_and_clear_timer(void)
{
    disable_interrupt();

    const uint8_t status = timer_reached;

    timer_reached = 0;

    enable_interrupt();

    return status;
}
