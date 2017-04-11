/**
 * @file obd.c
 * @brief TODO.
 *
 */


#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <math.h>

#include "board.h"
#include "uart_lib.h"
#include "uart_drv.h"

#include "debug.h"
#include "error.h"
#include "ring_buffer.h"
#include "time.h"
#include "hobd_protocol.h"
#include "hobd_parser.h"
#include "obd.h"


#define UART_RX_INTERRUPT USART1_RX_vect
#define UART_UCSRA UCSR1A
#define UART_UCSRB UCSR1B
#define UART_UCSRC UCSR1C
#define UART_DATA UDR1


#define obd_uart_enable() (UART_UCSRB |= (_BV(RXEN1) | _BV(TXEN1) | _BV(RXCIE1)))
#define obd_uart_disable() (UART_UCSRB &= ~(_BV(RXEN1) | _BV(TXEN1) | _BV(RXCIE1)))


static volatile ring_buffer_s rx_buffer;

static hobd_parser_s hobd_parser;


ISR( UART_RX_INTERRUPT )
{
    // read UART status register and UART data register
    const uint8_t status  = UART_UCSRA;
    const uint8_t data = UART_DATA;

    // read error status
    rx_buffer.error = (status & (_BV(FE1) | _BV(DOR1)) );

    // push data into the rx buffer, error is updated with return status
    (void) ring_buffer_putc(
            data,
            &rx_buffer);
}


static void hw_init( void )
{
    Uart_select(OBD_UART);

    Uart_clear();

    Uart_set_ubrr(OBD_BAUDRATE);

    Uart_hw_init(CONF_8BIT_NOPAR_1STOP);
}


uint8_t obd_init( void )
{
    uint8_t ret = ERR_OK;

    obd_uart_enable();

    hobd_parser_init(&hobd_parser);

    ring_buffer_init(&rx_buffer);

    hw_init();

    ring_buffer_flush(&rx_buffer);

    obd_uart_enable();

    return ret;
}


void obd_disable( void )
{
    obd_uart_disable();

    ring_buffer_flush(&rx_buffer);
}


void obd_enable( void )
{
    ring_buffer_flush(&rx_buffer);

    obd_uart_enable();
}


uint8_t obd_update( void )
{
    uint8_t ret = ERR_OK;

    // TESTING
#warning "TESTING"

    if(ring_buffer_available(&rx_buffer) != 0)
    {
        const uint16_t rb_data = ring_buffer_getc(&rx_buffer);

        if(rb_data != RING_BUFFER_NO_DATA)
        {
            const uint8_t data = RING_BUFFER_GET_DATA_BYTE(rb_data);

            DEBUG_PRINTF("%02X\n", data);

            ret = hobd_parser_parse_byte(data, &hobd_parser);

            if( ret == ERR_OK )
            {
                DEBUG_PRINTF("  got message 0x%02X\n", hobd_parser.header.type);
            }

            ret = ERR_OK;
        }
    }

    return ret;
}
