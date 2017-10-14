/**
 * @file debug_uart.c
 * @brief TODO.
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "board.h"
#include "error.h"
#include "debug_uart.h"

#define UART_PORT_DDR   (DDRE)
#define UART_PORT_OUT   (PORTE)
#define UART_PORT_IN    (PINE)
#define UART_RX_PIN     (0)
#define UART_TX_PIN     (1)
#define UART_RX_MASK    (1 << UART_RX_PIN)
#define UART_TX_MASK    (1 << UART_TX_PIN)

#define UART_RXC_ISR    USART0_RX_vect
#define UART_TXC_ISR    USART0_TX_vect
#define UART_UDRE_ISR   USART0_UDRE_vect
#define UART_UCSRA      UCSR0A
#define UART_UCSRB      UCSR0B
#define UART_UCSRC      UCSR0C
#define UART_UBRRH      UBRR0H
#define UART_UBRRL      UBRR0L
#define UART_DATA       UDR0

#define CONF_8BIT_NOPAR_1STOP ((0 << UPM00) | (0 << USBS0) | (3 <<(UCSZ00-1)))

#define uart_rx_enable() (UART_UCSRB |= _BV(RXEN0))
#define uart_rx_disable() (UART_UCSRB &= ~_BV(RXEN0))

#define uart_tx_enable() (UART_UCSRB |= _BV(TXEN0))
#define uart_tx_disable() (UART_UCSRB &= ~_BV(TXEN0))

#define uart_rxc_int_enable() (UART_UCSRB |= _BV(RXCIE0))
#define uart_rxc_int_disable() (UART_UCSRB &= ~_BV(RXCIE0))

#define uart_txc_int_enable() (UART_UCSRB |= _BV(TXCIE0))
#define uart_txc_int_disable() (UART_UCSRB &= ~_BV(TXCIE0))

#define uart_dre_int_enable() (UART_UCSRB |= _BV(UDRIE0))
#define uart_dre_int_disable() (UART_UCSRB &= ~_BV(UDRIE0))

#define uart_rx_ready() ((UART_UCSRA & _BV(RXC0)) >> RXC0)
#define uart_tx_ready() ((UART_UCSRA & _BV(UDRE0)) >> UDRE0)

#define uart_rx_in_pu_on() { UART_PORT_DDR &= ~UART_RX_MASK; UART_PORT_OUT |= UART_RX_PIN; }
#define uart_rx_in_pu_off() { UART_PORT_DDR &= ~UART_RX_MASK; UART_PORT_OUT &= ~UART_RX_PIN; }

#define uart_tx_in_pu_on() { UART_PORT_DDR &= ~UART_TX_MASK; UART_PORT_OUT |= UART_TX_PIN; }
#define uart_tx_in_pu_off() { UART_PORT_DDR &= ~UART_TX_MASK; UART_PORT_OUT &= ~UART_TX_PIN; }
#define uart_tx_out_clear() { UART_PORT_DDR |= UART_TX_MASK; UART_PORT_OUT &= ~UART_TX_PIN; }

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
    UART_UCSRA |= _BV(UDRE0);
    UART_UCSRB &= ~_BV(UCSZ02);
    UART_UCSRB |= config & _BV(UCSZ02);
    UART_UCSRC = config & ((3 << UPM00) | _BV(USBS0));
    UART_UCSRC |= ((config & (3 << (UCSZ00-1))) << UCSZ00);
}

static __attribute__((always_inline)) inline void uart_set_ubrr(
        const uint32_t baudrate)
{
    UART_UBRRH = (uint8_t)((((((((uint32_t)FOSC*1000)<<1) / ((uint32_t)baudrate*8))+1)>>1)-1)>>8);
    UART_UBRRL = (uint8_t)(((((((uint32_t)FOSC*1000)<<1) / ((uint32_t)baudrate*8))+1)>>1)-1);
    UART_UCSRA |= _BV(U2X1);
}

void debug_uart_init(void)
{
    // rx/tx are set to inputs, pullups off
    uart_rx_in_pu_off();
    uart_tx_in_pu_off();

    uart_hw_deinit();

    uart_hw_init(CONF_8BIT_NOPAR_1STOP);
    uart_set_ubrr(DEBUG_BAUDRATE);

    uart_rx_enable();
    uart_tx_enable();
}

void debug_uart_deinit(void)
{
    uart_hw_deinit();

    uart_rx_in_pu_off();
    uart_tx_in_pu_off();
}

uint8_t debug_uart_test_hit(void)
{
    return (uint8_t) uart_rx_ready();
}

uint8_t debug_uart_getc(void)
{
    while(uart_rx_ready() == 0);

    return (uint8_t) UART_DATA;
}

void debug_uart_putc(
        const uint8_t data)
{
    while(uart_tx_ready() == 0);

    UART_DATA = data;
}

void debug_uart_puts(
        const uint8_t *data)
{
    while(*data != 0)
    {
        debug_uart_putc(*data);
        data += 1;
    }

    // end with CR and LF
    debug_uart_putc(0x0D);
    debug_uart_putc(0x0A);
}
