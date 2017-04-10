/**
 * @file ring_buffer.h
 * @brief TODO.
 *
 */




#ifndef RING_BUFFER_H
#define	RING_BUFFER_H




#include <inttypes.h>




//
#define RING_BUFFER_SIZE (128)


//
#define RING_BUFFER_MASK (RING_BUFFER_SIZE - 1)


// framing error from UART
#define RING_BUFFER_FRAME_ERROR (0x0800)


// overrun condition from UART
#define RING_BUFFER_OVERRUN_ERROR (0x0400)


// ring buffer receive overflow
#define RING_BUFFER_RX_OVERFLOW (0x0200)


// no receive data available
#define RING_BUFFER_NO_DATA (0x0100)




//
typedef struct
{
    //
    //
    uint8_t head;
    //
    //
    uint8_t tail;
    //
    //
    uint8_t error;
    //
    //
    uint8_t buffer[ RING_BUFFER_SIZE ];
} ring_buffer_s;




//
void ring_buffer_init(
        volatile ring_buffer_s * const rb );


//
uint8_t ring_buffer_available(
        volatile ring_buffer_s * const rb );


//
void ring_buffer_flush(
        volatile ring_buffer_s * const rb );


//
uint16_t ring_buffer_putc(
        const uint8_t data,
        volatile ring_buffer_s * const rb );


//
uint16_t ring_buffer_getc(
        volatile ring_buffer_s * const rb );


//
uint16_t ring_buffer_peek(
        volatile ring_buffer_s * const rb );




#endif	/* RING_BUFFER_H */
