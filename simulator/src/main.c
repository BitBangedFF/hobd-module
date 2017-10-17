/**
 * @file main.c
 * @brief Main.
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include "board.h"
#include "error.h"
#include "time.h"
#include "obd_uart.h"
#include "hobd_parser.h"
#include "ecu.h"

int main(void)
{
    CPU_PRESCALE(CPU_16MHZ);

    wdt_disable();
    wdt_enable(WDTO_120MS);
    wdt_reset();

    ecu_deinit();

    led_init();
    led_off();

    time_init();

    ecu_init();

    enable_interrupt();

    while(1)
    {
        wdt_reset();

        ecu_update();

        if(time_get_and_clear_timer() != 0)
        {
            ecu_check_timeout_reset();
        }
    }

    return 0;
}
