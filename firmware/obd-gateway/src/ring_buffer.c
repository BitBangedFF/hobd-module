/**
 * @file ring_buffer.c
 * @brief TODO.
 *
 */




#include <stdlib.h>
#include <avr/io.h>
#include <inttypes.h>

#include "board.h"
#include "ring_buffer.h"




// *****************************************************
// static global types/macros
// *****************************************************




// *****************************************************
// static global data
// *****************************************************




// *****************************************************
// static declarations
// *****************************************************




// *****************************************************
// static definitions
// *****************************************************




// *****************************************************
// public definitions
// *****************************************************

//
void ring_buffer_init(
        volatile ring_buffer_s * const rb )
{
    rb->head = 0;
    rb->tail = 0;
    rb->error = 0;
}


//
uint8_t ring_buffer_available(
        volatile ring_buffer_s * const rb )
{
    const uint16_t size = (RING_BUFFER_SIZE + rb->head - rb->tail) & RING_BUFFER_MASK;

    return (uint8_t) size;
}


//
void ring_buffer_flush(
        volatile ring_buffer_s * const rb )
{
    rb->head = rb->tail;
}


//
uint16_t ring_buffer_putc(
        const uint8_t data,
        volatile ring_buffer_s * const rb )
{
    // calculate new head index
    const uint16_t new_head =  (uint16_t) (rb->head + 1) & RING_BUFFER_MASK;

    if( new_head == (uint8_t) rb->tail )
    {
        // receive buffer overflow error
        rb->error |= (RING_BUFFER_RX_OVERFLOW >> 8);
    }
    else
    {
        // store new index
        rb->head = (uint8_t) new_head;

        // store received data in buffer
        rb->buffer[ new_head ] = data;
    }

    return (uint8_t) (rb->error << 8);
}


//
uint16_t ring_buffer_getc(
        volatile ring_buffer_s * const rb )
{
    uint16_t ret = 0;

    if( rb->head == rb->tail )
    {
        ret = RING_BUFFER_NO_DATA;
    }
    else
    {
        // calculate new buffer index
        const uint16_t new_tail = (uint16_t) (rb->tail + 1) & RING_BUFFER_MASK;

        rb->tail = (uint8_t) new_tail;

        const uint8_t rx_data = rb->buffer[ new_tail ];

        ret = (uint16_t) (rb->error << 8) + rx_data;
    }

    return ret;
}


//
uint16_t ring_buffer_peek(
        volatile ring_buffer_s * const rb )
{
    uint16_t ret = 0;

    if( rb->head == rb->tail )
    {
        ret = RING_BUFFER_NO_DATA;
    }
    else
    {
        // calculate new buffer index
        const uint16_t tmp_tail = (uint16_t) (rb->tail + 1) & RING_BUFFER_MASK;

        const uint8_t rx_data = rb->buffer[ tmp_tail ];

        ret = (uint16_t) (rb->error << 8) + rx_data;
    }

    return ret;
}
