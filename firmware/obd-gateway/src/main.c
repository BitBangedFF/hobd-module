/**
 * @file main.c
 * @brief Main.
 *
 */


#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>

// board definition/configuration
#include "board.h"

// BSP utilities
#include "timer8_drv.h"
#include "timer16_drv.h"
#include "uart_drv.h"
#include "uart_lib.h"
#include "rtc_drv.h"
#include "can_drv.h"
#include "can_lib.h"

// project includes
#include "debug.h"
#include "time.h"
#include "canbus.h"
#include "obd.h"
#include "diagnostics.h"
#include "hobd.h"


#ifdef BUILD_TYPE_DEBUG
#warning "BUILD_TYPE_DEBUG ON"
#endif


static void init( void )
{
    wdt_disable();

    sw0_init();
    sw0_enable_pullup();

    led_init();
    led_off();

    wdt_enable(WDTO_120MS);
    wdt_reset();

    rtc_int_init();
    enable_interrupt();

#ifdef BUILD_TYPE_DEBUG
    Uart_select(DEBUG_UART);
    uart_init(CONF_8BIT_NOPAR_1STOP, DEBUG_BAUDRATE);
#endif

    wdt_reset();
    diagnostics_init();
    canbus_init();
    obd_init();

    DEBUG_PRINTF("module '%s' initialized\n", MODULE_NAME);
}


int main( void )
{
    init();

    diagnostics_update();

    diagnostics_set_state(HOBD_HEARTBEAT_STATE_OK);

    while(1)
    {
        wdt_reset();

        obd_update();

        if(time_get_timer() != 0)
        {
            time_clear_timer();
            obd_update_timeout();
            diagnostics_update();
        }
    }

   return 0;
}
