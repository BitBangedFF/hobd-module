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
#include "time.h"
#include "obd_uart.h"

int main(void)
{
    CPU_PRESCALE(CPU_16MHZ);

    wdt_disable();

    obd_uart_deinit();

    led_init();
    led_off();

    time_init();

    obd_uart_init();

    enable_interrupt();

    while(1)
    {
        //time_delay_ms(100);
        //led_toggle();


        if(time_get_and_clear_timer() != 0)
        {
            led_toggle();
        }
    }

    return 0;
}
