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
#include "hobd.h"
#include "obd.h"


typedef struct
{
    hobd_obd_time_s obd_time;
    hobd_obd1_s obd1;
    hobd_obd2_s obd2;
    hobd_obd3_s obd3;
} obd_data_s;


#define UART_RX_INTERRUPT USART1_RX_vect
#define UART_UCSRA UCSR1A
#define UART_UCSRB UCSR1B
#define UART_UCSRC UCSR1C
#define UART_DATA UDR1

#define obd_uart_enable() (UART_UCSRB |= (_BV(RXEN1) | _BV(TXEN1) | _BV(RXCIE1)))
#define obd_uart_disable() (UART_UCSRB &= ~(_BV(RXEN1) | _BV(TXEN1) | _BV(RXCIE1)))


static volatile ring_buffer_s rx_buffer;

static hobd_parser_s hobd_parser;

static obd_data_s obd_data;


ISR( UART_RX_INTERRUPT )
{
    const uint8_t status  = UART_UCSRA;
    const uint8_t data = UART_DATA;

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


static void process_data( void )
{
    if(hobd_parser.header.type == HOBD_MSG_TYPE_RESPONSE)
    {
        if(hobd_parser.header.subtype == HOBD_MSG_SUBTYPE_TABLE_SUBGROUP)
        {
            const hobd_data_table_response_s * const response =
                    (const hobd_data_table_response_s*) &hobd_parser.data[0];

            obd_data.obd_time.rx_time = hobd_parser.rx_timestamp;

            if( response->table == HOBD_TABLE_16 )
            {
                const hobd_table_16_s * const rx_data =
                        (const hobd_table_16_s*) &response->data[0];

                obd_data.obd_time.counter_1 += 1;

                obd_data.obd1.engine_rpm = rx_data->engine_rpm;
                obd_data.obd1.wheel_speed = rx_data->wheel_speed;
                obd_data.obd1.battery_volt = rx_data->battery_volt;
                obd_data.obd1.tps_volt = rx_data->tps_volt;
                obd_data.obd1.tps_percent = rx_data->tps_percent;

                obd_data.obd2.ect_volt = rx_data->ect_volt;
                obd_data.obd2.ect_temp = rx_data->ect_temp;
                obd_data.obd2.iat_volt = rx_data->iat_volt;
                obd_data.obd2.iat_temp = rx_data->iat_temp;
                obd_data.obd2.map_volt = rx_data->map_volt;
                obd_data.obd2.map_pressure = rx_data->map_pressure;
                obd_data.obd2.fuel_injectors = rx_data->fuel_injectors;
            }
            else if( response->table == HOBD_TABLE_209 )
            {
                const hobd_table_209_s * const rx_data =
                        (const hobd_table_209_s*) &response->data[0];

                obd_data.obd_time.counter_2 += 1;

                obd_data.obd3.engine_on = rx_data->engine_on;
                obd_data.obd3.gear = rx_data->gear;
            }
        }
    }
}


uint8_t obd_init( void )
{
    uint8_t ret = ERR_OK;

    obd_uart_disable();

    (void) memset(&obd_data, 0, sizeof(obd_data));

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

    hobd_parser_init(&hobd_parser);
}


void obd_enable( void )
{
    hobd_parser_init(&hobd_parser);

    ring_buffer_flush(&rx_buffer);

    obd_uart_enable();
}


uint8_t obd_update( void )
{
    uint8_t ret = ERR_OK;

    if(ring_buffer_available(&rx_buffer) != 0)
    {
        const uint16_t rb_data = ring_buffer_getc(&rx_buffer);

        if(rb_data != RING_BUFFER_NO_DATA)
        {
            const uint8_t data = RING_BUFFER_GET_DATA_BYTE(rb_data);

            const uint8_t status = hobd_parser_parse_byte(data, &hobd_parser);

            if( status == ERR_OK )
            {
                DEBUG_PRINTF("  got message 0x%02X\n", hobd_parser.header.type);

                process_data();
            }
        }
    }

    return ret;
}
