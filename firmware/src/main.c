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
#include "debug_uart.h"
#include "obd_uart.h"
#include "canbus.h"
#include "diagnostics.h"
#include "comm.h"
#include "hobd_can.h"

#ifdef BUILD_TYPE_DEBUG
#warning "BUILD_TYPE_DEBUG ON"
#endif

static void init(void)
{
    wdt_disable();
    wdt_enable(WDTO_120MS);
    wdt_reset();

    comm_deinit();
    debug_uart_deinit();

    sw0_init();
    sw0_enable_pullup();
    sw1_init();
    sw1_enable_pullup();

    led_init();
    led_off();

    time_init();

    wdt_reset();

    debug_init();

    diagnostics_init();

    canbus_init();

    comm_init();

    enable_interrupt();

    debug_puts(MODULE_NAME" inialized");
}

int main( void )
{
    init();

    diagnostics_update();

    diagnostics_set_state(HOBD_HEARTBEAT_STATE_OK);

    while(1)
    {
        wdt_reset();

        comm_update();

        if(time_get_and_clear_timer() != 0)
        {
            comm_check_timeout_reset();

            diagnostics_update();
        }
    }

   return 0;
}
