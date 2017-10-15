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

static uint8_t hobd_rx_buffer[HOBD_MSG_SIZE_MAX];
static hobd_parser_s hobd_parser;

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

    hobd_parser_init(
            &hobd_rx_buffer[0],
            sizeof(hobd_rx_buffer),
            &hobd_parser);

    // TODO
    // hobd parser is fake ECU
    // hook up USB debug for logging
    // minitor GPIO -> enable parser ...
    // LED blinks to indicate gear ?
    // update protocol

    obd_uart_putc('H');
    obd_uart_putc('\r');
    obd_uart_putc('\n');

    while(1)
    {
        wdt_reset();

        const uint16_t rx_data = obd_uart_getc();

        if(rx_data != RING_BUFFER_NO_DATA)
        {
            const uint8_t status = hobd_parser_parse_byte(
                    RING_BUFFER_GET_DATA_BYTE(rx_data),
                    &hobd_parser);

            if(status == 0)
            {
                led_toggle();
            }
        }

        if(time_get_and_clear_timer() != 0)
        {
            // TODO
        }
    }

    return 0;
}
