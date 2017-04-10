/**
 * @file board.h
 * @brief TODO.
 *
 * @todo board details.
 *
 * UART0 - pins ? wired to DB9
 * UART1 - ...
 *
 */




#ifndef BOARD_H
#define BOARD_H




#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>




//
#define NODE_ID (0x05)


//
#define HARDWARE_VERSION (1)


//
#define FIRMWARE_VERSION (1)


#ifndef BOOL
#include <inttypes.h>
typedef uint8_t BOOL;
#endif


//
#define OFF (0)
#define ON  (1)


//
#define FALSE (0)
#define TRUE  (1)


//
#define MAX(a, b) ( (a)>(b) ? (a) : (b) )
#define MIN(a, b) ( (a)<(b) ? (a) : (b) )


//
#define enable_interrupt() { asm ("sei"::) ; }


//
#define disable_interrupt() { asm ("cli"::) ; }


//
#define hard_reset() { WDTCR |= 1<<WDE;  while(1); }


// chip
#ifndef __AVR_AT90CAN128__
#define __AVR_AT90CAN128__ 1
#endif


// 16 MHz
#define FOSC (16000)
#define F_CPU (FOSC * 1000)


// uart config
#define USE_TIMER16 BOTH_TIMER16
#define USE_UART BOTH_UART
#define UART_BAUDRATE VARIABLE_UART_BAUDRATE


// RTC config
#define USE_TIMER8 TIMER8_2
#define RTC_TIMER (2)
#define RTC_CLOCK (0)


// CAN
#define CAN_BAUDRATE (500)


//
#define DEBUG_UART UART_0
#define OBD_UART UART_1


//
#define DEBUG_BAUDRATE (57600UL)
#define OBD_BAUDRATE (115200UL)


//
#define LED_PORT_IN PINE
#define LED_PORT_DDR DDRE
#define LED_PORT_OUT PORTE
#define LED_STATUS_0 (4)


#define SW0_PORT_IN PINE
#define SW0_PORT_DDR DDRE
#define SW0_PORT_OUT PORTE
#define SW0_PIN (5)


#define SW1_PORT_IN PINC
#define SW1_PORT_DDR DDRC
#define SW1_PORT_OUT PORTC
#define SW1_PIN (6)


#define led_init() (LED_PORT_DDR |= (1 << LED_STATUS_0))
#define led_on() (LED_PORT_OUT &= 0b11101111)
#define led_off() (LED_PORT_OUT |= 0b00010000)
#define led_toggle() (LED_PORT_OUT ^= (1 << LED_STATUS_0))


#define sw0_init() (SW0_PORT_DDR &= ~(1 << SW0_PIN))
#define sw0_enable_pullup() (SW0_PORT_OUT |= (1 << SW0_PIN))
#define sw0_get_state() (!(SW0_PORT_IN & (1 << SW0_PIN)))


#define sw1_init() (SW1_PORT_DDR &= ~(1 << SW1_PIN))
#define sw1_enable_pullup() (SW1_PORT_OUT |= (1 << SW1_PIN))
#define sw1_get_state() (!(SW1_PORT_IN & (1 << SW1_PIN)))


//
#ifdef BUILD_TYPE_DEBUG
#define DEBUG_PUTS( x ) \
{ \
    Uart_select( DEBUG_UART ); \
    uart_put_string( (uint8_t*) x ); \
}

#define DEBUG_PRINTF( ... ) \
{ \
    Uart_select( DEBUG_UART ); \
    uart_mini_printf( __VA_ARGS__ ); \
}
#else
#define DEBUG_PUTS( x )
#define DEBUG_PRINTF( ... )
#endif



#endif // BOARD_H
