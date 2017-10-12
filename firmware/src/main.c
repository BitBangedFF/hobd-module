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
#include "obd_uart.h"

#ifdef BUILD_TYPE_DEBUG
#warning "BUILD_TYPE_DEBUG ON"
#endif

static void init(void)
{
    wdt_disable();
    wdt_enable(WDTO_120MS);
    wdt_reset();

    obd_uart_deinit();

    sw0_init();
    sw0_enable_pullup();
    sw1_init();
    sw1_enable_pullup();

    led_init();
    led_off();

    wdt_reset();

    time_init();

    obd_uart_init();

    enable_interrupt();

    debug_puts(MODULE_NAME" inialized");
}

int main( void )
{
    init();

    while(1)
    {
        wdt_reset();

        const uint16_t data = obd_uart_getc();

        if(data != RING_BUFFER_NO_DATA)
        {
            led_toggle();
        }

        if(time_get_and_clear_timer() != 0)
        {
            // TODO
        }
    }

   return 0;
}
