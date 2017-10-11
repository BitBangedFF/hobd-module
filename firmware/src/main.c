/**
 * @file main.c
 * @brief Main.
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <stdint.h>
#include "board.h"
#include "debug.h"
#include "time.h"

#ifdef BUILD_TYPE_DEBUG
#warning "BUILD_TYPE_DEBUG ON"
#endif

static void init(void)
{
    wdt_disable();

    sw0_init();
    sw0_enable_pullup();
    sw1_init();
    sw1_enable_pullup();

    led_init();
    led_off();

    wdt_enable(WDTO_120MS);
    wdt_reset();

    time_init();
    enable_interrupt();
}

int main( void )
{
    init();

    while(1)
    {
        // TODO
        wdt_reset();
    }

   return 0;
}
