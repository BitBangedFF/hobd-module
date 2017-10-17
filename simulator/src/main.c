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

#define STARTUP_DELAY (400)
#define LED_STARTUP_PERIOD (100)
#define LED_STARTUP_COUNT (20)

int main(void)
{
    CPU_PRESCALE(CPU_16MHZ);

    wdt_disable();
    wdt_enable(WDTO_500MS);
    wdt_reset();

    ecu_deinit();

    led_init();
    led_off();

    time_init();

    ecu_init();

    enable_interrupt();

    wdt_reset();
    time_delay_ms(400);
    wdt_reset();
    
    uint8_t i;
    for(i = 0; i < LED_STARTUP_COUNT; i += 1)
    {
        time_delay_ms(LED_STARTUP_PERIOD);
        wdt_reset();
        led_toggle();
    }

    led_on();
    wdt_reset();
    wdt_disable();
    wdt_enable(WDTO_120MS);
    wdt_reset();

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
