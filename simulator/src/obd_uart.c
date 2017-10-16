/**
 * @file obd_uart.c
 * @brief TODO.
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "board.h"
#include "ring_buffer.h"
#include "obd_uart.h"

#define UART_PORT_DDR   (DDRD)
#define UART_PORT_OUT   (PORTD)
#define UART_PORT_IN    (PIND)
#define UART_RX_PIN     (2)
#define UART_TX_PIN     (3)
#define UART_RX_MASK    (1 << UART_RX_PIN)
#define UART_TX_MASK    (1 << UART_TX_PIN)

#define UART_RXC_ISR    USART1_RX_vect
#define UART_TXC_ISR    USART1_TX_vect
#define UART_UDRE_ISR   USART1_UDRE_vect
#define UART_UCSRA      UCSR1A
#define UART_UCSRB      UCSR1B
#define UART_UCSRC      UCSR1C
#define UART_UBRRH      UBRR1H
#define UART_UBRRL      UBRR1L
#define UART_DATA       UDR1

#define CONF_8BIT_NOPAR_1STOP ((0 << UPM10) | (0 << USBS1) | (3 <<(UCSZ10-1)))

#define uart_rx_enable() (UART_UCSRB |= _BV(RXEN1))
#define uart_rx_disable() (UART_UCSRB &= ~_BV(RXEN1))

#define uart_tx_enable() (UART_UCSRB |= _BV(TXEN1))
#define uart_tx_disable() (UART_UCSRB &= ~_BV(TXEN1))

#define uart_rxc_int_enable() (UART_UCSRB |= _BV(RXCIE1))
#define uart_rxc_int_disable() (UART_UCSRB &= ~_BV(RXCIE1))

#define uart_txc_int_enable() (UART_UCSRB |= _BV(TXCIE1))
#define uart_txc_int_disable() (UART_UCSRB &= ~_BV(TXCIE1))

#define uart_dre_int_enable() (UART_UCSRB |= _BV(UDRIE1))
#define uart_dre_int_disable() (UART_UCSRB &= ~_BV(UDRIE1))

#define uart_rx_in_pu_on() { UART_PORT_DDR &= ~UART_RX_MASK; UART_PORT_OUT |= UART_RX_PIN; }
#define uart_rx_in_pu_off() { UART_PORT_DDR &= ~UART_RX_MASK; UART_PORT_OUT &= ~UART_RX_PIN; }

#define uart_tx_in_pu_on() { UART_PORT_DDR &= ~UART_TX_MASK; UART_PORT_OUT |= UART_TX_PIN; }
#define uart_tx_in_pu_off() { UART_PORT_DDR &= ~UART_TX_MASK; UART_PORT_OUT &= ~UART_TX_PIN; }

static volatile ring_buffer_s rx_buffer;
static volatile ring_buffer_s tx_buffer;

ISR(UART_RXC_ISR)
{
    const uint8_t data = UART_DATA;

    ring_buffer_putc(
            data,
            &rx_buffer);
}

ISR(UART_TXC_ISR)
{
    // data register is empty now, disable transmitter, and re-enable receiver
    uart_txc_int_disable();
    uart_tx_disable();

    uart_rx_enable();
    uart_rxc_int_enable();
}

ISR(UART_UDRE_ISR)
{
    const uint16_t rb_data = ring_buffer_getc(&tx_buffer);

    if(rb_data != RING_BUFFER_NO_DATA)
    {
        UART_DATA = (uint8_t) RING_BUFFER_GET_DATA_BYTE(rb_data);

        // enable TXC interrupt, this will disable the trasmitter and
        // enable the receiver when complete
        uart_txc_int_enable();
    }
    else
    {
        // nothing left to transmit, disable the DRE interrupt
        uart_dre_int_disable();
    }
}

static __attribute__((always_inline)) inline void uart_hw_deinit(void)
{
    UART_UCSRB = 0;
    UART_UCSRC = UART_DATA;
    UART_UCSRA = 0x40;
    UART_UCSRC = 0x06;
    UART_UBRRH = 0;
    UART_UBRRL = 0;

    uart_rxc_int_disable();
    uart_txc_int_disable();
    uart_dre_int_disable();

    uart_rx_disable();
    uart_tx_disable();
}

static __attribute__((always_inline)) inline void uart_hw_init(
        const uint8_t config)
{
    UART_UCSRA |= _BV(UDRE1);
    UART_UCSRB &= ~_BV(UCSZ12);
    UART_UCSRB |= config & _BV(UCSZ12);
    UART_UCSRC = config & ((3 << UPM10) | _BV(USBS1));
    UART_UCSRC |= ((config & (3 << (UCSZ10-1))) << UCSZ10);
}

static __attribute__((always_inline)) inline void uart_set_ubrr(
        const uint32_t baudrate)
{
    UART_UBRRH = (uint8_t)((((((((uint32_t)FOSC*1000)<<1) / ((uint32_t)baudrate*8))+1)>>1)-1)>>8);
    UART_UBRRL = (uint8_t)(((((((uint32_t)FOSC*1000)<<1) / ((uint32_t)baudrate*8))+1)>>1)-1);
    UART_UCSRA |= _BV(U2X1);
}

void obd_uart_init(void)
{
    // rx/tx are set to inputs, pullups off
    uart_rx_in_pu_off();
    uart_tx_in_pu_off();

    uart_hw_deinit();

    ring_buffer_init(&rx_buffer);
    ring_buffer_init(&tx_buffer);

    uart_hw_init(CONF_8BIT_NOPAR_1STOP);
    uart_set_ubrr(OBD_BAUDRATE);

    ring_buffer_flush(&rx_buffer);
    ring_buffer_flush(&tx_buffer);

    uart_rx_enable();
    uart_rxc_int_enable();
}

void obd_uart_deinit(void)
{
    uart_hw_deinit();

    uart_rx_in_pu_off();
    uart_tx_in_pu_off();

    ring_buffer_flush(&rx_buffer);
    ring_buffer_flush(&tx_buffer);
}

uint16_t obd_uart_getc(void)
{
    return ring_buffer_getc(&rx_buffer);
}

void obd_uart_putc(
        const uint8_t data)
{
    obd_uart_send(&data, 1);
}

void obd_uart_send(
        const uint8_t * const data,
        const uint16_t size)
{
    uint16_t i;
    for(i = 0; i < size; i += 1)
    {
        ring_buffer_putc(
                data[i],
                &tx_buffer);

        if((tx_buffer.error & (RING_BUFFER_RX_OVERFLOW >> 8)) != 0)
        {
            // TODO
        }
    }

    // disable receiver
    uart_rxc_int_disable();
    uart_rx_disable();

    // enable transmitter and DRE
    uart_tx_enable();
    uart_dre_int_enable();
}
