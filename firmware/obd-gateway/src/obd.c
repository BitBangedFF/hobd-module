/**
 * @file obd.c
 * @brief TODO.
 *
 */




#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <math.h>

#include "board.h"
#include "uart_lib.h"
#include "uart_drv.h"
#include "hobd.h"
#include "ring_buffer.h"
#include "time.h"
#include "canbus.h"
#include "diagnostics.h"
#include "hobd_uart.h"
#include "obd.h"




// *****************************************************
// static global types/macros
// *****************************************************

//
#define OBD_BUFFER_SIZE (512)


// UART bridge is on UART1
#define UART_RX_INTERRUPT USART1_RX_vect
#define UART_UCSRA UCSR1A
#define UART_UCSRB UCSR1B
#define UART_UCSRC UCSR1C
#define UART_DATA UDR1


//
#define obd_uart_enable() (UART_UCSRB |= (_BV(RXEN1) | _BV(TXEN1) | _BV(RXCIE1)))


//
#define obd_uart_disable() (UART_UCSRB &= ~(_BV(RXEN1) | _BV(TXEN1) | _BV(RXCIE1)))




// *****************************************************
// static global data
// *****************************************************

// UART rx ring buffer
static volatile ring_buffer_s rx_buffer;


// GPS message/data state
static obd_data_s obd_data;


// last rx time
static uint32_t last_rx_time = 0;


// OBD rx buffer
static uint8_t obd_buffer[ OBD_BUFFER_SIZE ];


// OBD rx packet counters
static uint16_t rx_count_table_16 = 0;
static uint16_t rx_count_table_209 = 0;




// *****************************************************
// static declarations
// *****************************************************

//
static void hw_init( void );


//
static void update_rx_timeout(
        const uint32_t * const now );


//
static uint8_t publish_group_a( void );


//
static uint8_t publish_group_b( void );


//
static uint8_t obd_checksum(
        const uint8_t * const buffer,
        const uint16_t len );


//
static uint8_t obd_packet_type(
        const uint8_t * const buffer,
        const uint16_t len );


//
static uint8_t process_buffer( void );


//
static void parse_response(
        const hobd_table_response_s * const response,
        const uint32_t * const rx_timestamp );




// *****************************************************
// static definitions
// *****************************************************

//
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
            &rx_buffer );
}


//
static void hw_init( void )
{
    Uart_select( OBD_UART );

    Uart_clear();

    Uart_set_ubrr( OBD_BAUDRATE );

    Uart_hw_init( CONF_8BIT_NOPAR_1STOP );

    obd_uart_enable();
}


//
static void update_rx_timeout(
        const uint32_t * const now )
{
    const uint16_t warn = diagnostics_get_warn();

    // if warn not set
    if( (warn & HOBD_HEARTBEAT_WARN_NO_OBD_ECU) == 0 )
    {
        // get time since last rx update
        const uint32_t delta = time_get_delta(
                &last_rx_time,
                now );

        // set warning if interval met/exceeded
        if( delta >= OBD_RX_WARN_TIMEOUT )
        {
            diagnostics_set_warn( HOBD_HEARTBEAT_WARN_NO_OBD_ECU );
        }
    }
}


//
static uint8_t publish_group_a( void )
{
    uint8_t ret = 0;

    ret |= canbus_send(
            HOBD_CAN_ID_OBD_TIME,
            (uint8_t) sizeof(obd_data.group_a.time),
            (const uint8_t *) &obd_data.group_a.time );

    ret |= canbus_send(
            HOBD_CAN_ID_OBD1,
            (uint8_t) sizeof(obd_data.group_a.obd1),
            (const uint8_t *) &obd_data.group_a.obd1 );

    ret |= canbus_send(
            HOBD_CAN_ID_OBD2,
            (uint8_t) sizeof(obd_data.group_a.obd2),
            (const uint8_t *) &obd_data.group_a.obd2 );

    return ret;
}


//
static uint8_t publish_group_b( void )
{
    uint8_t ret = 0;

    ret |= canbus_send(
            HOBD_CAN_ID_OBD_TIME,
            (uint8_t) sizeof(obd_data.group_b.time),
            (const uint8_t *) &obd_data.group_b.time );

    ret |= canbus_send(
            HOBD_CAN_ID_OBD3,
            (uint8_t) sizeof(obd_data.group_b.obd3),
            (const uint8_t *) &obd_data.group_b.obd3 );

    return ret;
}


//
static uint8_t obd_checksum(
        const uint8_t * const buffer,
        const uint16_t len )
{
    uint16_t cs = 0;
    uint16_t idx = 0;

    for( idx = 0; idx < len; idx += 1 )
    {
        cs += (uint16_t) buffer[ idx ];
    }

    if( cs > 0x0100 )
    {
        cs = (0x0100 - (cs & 0x00FF));
    }
    else
    {
        cs = (0x0100 - cs);
    }

    return (uint8_t) (cs & 0xFF);
}


//
static uint8_t obd_packet_type(
        const uint8_t * const buffer,
        const uint16_t len )
{
    uint8_t packet_type = HOBD_PACKET_TYPE_INVALID;

    const hobd_packet_header_s * const header =
            (hobd_packet_header_s*) buffer;

    // min size = 3 byte header plus 1 byte checksum
    const uint16_t min_len = (uint16_t) (sizeof(*header) + 1);

    // check packet length
    if( (len >= min_len) && ((uint16_t) header->size >= min_len) && ((uint16_t) header->size <= len) )
    {
        // check packet type
        if( header->type != HOBD_PACKET_TYPE_INVALID )
        {
            const uint8_t packet_checksum = buffer[ header->size - 1 ];

            // excludes the checksum byte
            const uint8_t real_checksum = obd_checksum(
                    buffer,
                    (uint16_t) header->size - 1 );

            // check checksum
            if( packet_checksum == real_checksum )
            {
                // valid
                packet_type = header->type;
            }
        }
    }

    return packet_type;
}


//
static uint8_t process_buffer( void )
{
    uint8_t ret = 0;
    uint32_t rx_timestamp = 0;
    uint16_t rb_data = RING_BUFFER_NO_DATA;

    hobd_packet_header_s * const header =
                (hobd_packet_header_s*) &obd_buffer[ 0 ];

    // initialize header
    header->type = HOBD_PACKET_TYPE_INVALID;
    header->size = 0;
    header->subtype = 0x00;

    if( ring_buffer_available( &rx_buffer ) != 0 )
    {
        // get the rx timestamp if we have data to process
        rx_timestamp = time_get_ms();

        rb_data = ring_buffer_getc( &rx_buffer );

        if( rb_data != RING_BUFFER_NO_DATA )
        {
            header->type = (uint8_t) (rb_data & 0xFF);
        }
    }

    if( header->type == HOBD_PACKET_TYPE_RESPONSE )
    {
        // wait for next byte
        while( ring_buffer_available( &rx_buffer ) == 0 );

        rb_data = ring_buffer_getc( &rx_buffer );

        if( rb_data != RING_BUFFER_NO_DATA )
        {
            header->size = (uint8_t) (rb_data & 0xFF);
        }

        // wait for next byte
        while( ring_buffer_available( &rx_buffer ) == 0 );

        rb_data = ring_buffer_getc( &rx_buffer );

        if( rb_data != RING_BUFFER_NO_DATA )
        {
            header->subtype = (uint8_t) (rb_data & 0xFF);
        }
    }

    if(
            (header->type == HOBD_PACKET_TYPE_RESPONSE) &&
            (header->subtype == HOBD_PACKET_SUBTYPE_TABLE_SUBGROUP) )
    {
        uint8_t idx = (uint8_t) sizeof(*header);

        do
        {
            rb_data = ring_buffer_getc( &rx_buffer );

            if( rb_data != RING_BUFFER_NO_DATA )
            {
                obd_buffer[ idx ] = (uint8_t) (rb_data & 0xFF);
                idx += 1;
            }
        }
        while( idx < header->size );

        // check if a valid packet
        const uint8_t packet_type = obd_packet_type(
                &obd_buffer[ 0 ],
                (uint16_t) header->size );

        // process response types
        if( packet_type == HOBD_PACKET_TYPE_RESPONSE )
        {
            if( header->subtype == HOBD_PACKET_SUBTYPE_TABLE_SUBGROUP )
            {
                const hobd_table_response_s * const response =
                        (hobd_table_response_s*) &obd_buffer[ 0 ];

                diagnostics_clear_warn( HOBD_HEARTBEAT_WARN_NO_OBD_ECU );

                parse_response(
                        response,
                        &rx_timestamp );
            }
        }
    }

    return ret;
}


//
static void parse_response(
        const hobd_table_response_s * const response,
        const uint32_t * const rx_timestamp )
{
    // number of bytes in the table response payload
    const uint8_t registers_size = (response->header.size - sizeof(*response) - 1);

     if( response->table == HOBD_TABLE_16 )
     {
         if( registers_size >= (uint8_t) sizeof(hobd_table_16_s) )
         {
            DEBUG_PUTS( "obd_table_0x10(16)\n" );

            const hobd_table_16_s * const table_data =
                    (hobd_table_16_s*) &((uint8_t*) response)[ sizeof(*response) ];

            rx_count_table_16 += 1;

            obd_data.group_a.time.rx_time = (*rx_timestamp);
            obd_data.group_a.time.counter_1 = rx_count_table_16;
            obd_data.group_a.time.counter_2 = rx_count_table_209;

            obd_data.group_a.obd1.engine_rpm = table_data->engine_rpm;
            obd_data.group_a.obd1.wheel_speed = table_data->wheel_speed;
            obd_data.group_a.obd1.battery_volt = table_data->battery_volt;
            obd_data.group_a.obd1.tps_volt = table_data->tps_volt;
            obd_data.group_a.obd1.tps_percent = table_data->tps_percent;

            obd_data.group_a.obd2.ect_volt = table_data->ect_volt;
            obd_data.group_a.obd2.ect_temp = table_data->ect_temp;
            obd_data.group_a.obd2.iat_volt = table_data->iat_volt;
            obd_data.group_a.obd2.iat_temp = table_data->iat_temp;
            obd_data.group_a.obd2.map_volt = table_data->map_volt;
            obd_data.group_a.obd2.map_pressure = table_data->map_pressure;
            obd_data.group_a.obd2.fuel_injectors = table_data->fuel_injectors;

            last_rx_time = (*rx_timestamp);

            obd_set_group_ready( OBD_GROUP_A_READY );
         }
     }
     else if( response->table == HOBD_TABLE_209 )
     {
         if( registers_size >= (uint8_t) sizeof(hobd_table_209_s) )
         {
            DEBUG_PUTS( "obd_table_0xD1(209)\n" );

            const hobd_table_209_s * const table_data =
                       (hobd_table_209_s*) &((uint8_t*) response)[ sizeof(*response) ];

            rx_count_table_209 += 1;

            obd_data.group_b.time.rx_time = (*rx_timestamp);
            obd_data.group_b.time.counter_1 = rx_count_table_16;
            obd_data.group_b.time.counter_2 = rx_count_table_209;

            obd_data.group_b.obd3.engine_on = table_data->engine_on;
            obd_data.group_b.obd3.gear = table_data->gear;

            last_rx_time = (*rx_timestamp);

            obd_set_group_ready( OBD_GROUP_B_READY );
         }
     }
}




// *****************************************************
// public definitions
// *****************************************************

//
uint8_t obd_init( void )
{
    uint8_t ret = 0;

    memset( &obd_data, 0, sizeof(obd_data) );

    ring_buffer_init( &rx_buffer );

    hw_init();

    // clear all ready groups
    obd_clear_all_group_ready();

    // flush rx buffer
    ring_buffer_flush( &rx_buffer );

    return ret;
}


//
void obd_disable( void )
{
    // disable UART
    obd_uart_disable();

    // flush rx buffer
    ring_buffer_flush( &rx_buffer );
}


//
void obd_enable( void )
{
    // flush rx buffer
    ring_buffer_flush( &rx_buffer );

    // enable UART
    obd_uart_enable();
}


//
void obd_set_group_ready(
        const uint16_t group )
{
    obd_data.ready_groups |= group;
}


//
void obd_clear_group_ready(
        const uint16_t group )
{
    obd_data.ready_groups &= ~group;
}


//
void obd_clear_all_group_ready( void )
{
    obd_data.ready_groups = OBD_GROUP_NONE_READY;
}


//
uint8_t obd_is_group_ready(
        const uint16_t group )
{
    return ((obd_data.ready_groups & group) == 0) ? 0 : 1;
}


//
uint8_t obd_update( void )
{
    uint8_t ret = 0;

    // process any available data in the rx buffer
    ret = process_buffer();

    // check for any ready groups
    if( obd_data.ready_groups != OBD_GROUP_NONE_READY )
    {
        // handle groups in order/priority
        if( obd_is_group_ready( OBD_GROUP_A_READY ) != 0 )
        {
            ret |= publish_group_a();
            obd_clear_group_ready( OBD_GROUP_A_READY );
        }

        if( obd_is_group_ready( OBD_GROUP_B_READY ) != 0 )
        {
            ret |= publish_group_b();
            obd_clear_group_ready( OBD_GROUP_B_READY );
        }
    }

    // get current time
    const uint32_t now = time_get_ms();

    // update rx timeout status/warning
    update_rx_timeout( &now );

    return ret;
}
