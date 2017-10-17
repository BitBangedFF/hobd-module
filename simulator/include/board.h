/**
 * @file board.h
 * @brief TODO.
 *
 */

#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

#define OFF (0)
#define ON  (1)

#define FALSE (0)
#define TRUE  (1)

#define HIGH (1)
#define LOW (0)

#define MAX(a, b) ( (a)>(b) ? (a) : (b) )
#define MIN(a, b) ( (a)<(b) ? (a) : (b) )

#define bit_get(p,m) ((p) & (m))
#define bit_set(p,m) ((p) |= (m))
#define bit_clear(p,m) ((p) &= ~(m))
#define bit_flip(p,m) ((p) ^= (m))
#define bit_write(c,p,m) (c ? bit_set(p,m) : bit_clear(p,m))
#define BIT(x) (0x01 << (x))

#define CPU_16MHZ (0x00)

#define CPU_PRESCALE(n) {CLKPR = 0x80; CLKPR = (n);}

#define hard_reset() {WDTCR |= 1<<WDE;  while(1);}

#define enable_interrupt() {asm ("sei"::);}
#define disable_interrupt() {asm ("cli"::);}

#define OBD_BAUDRATE (10400UL)

#define LED_PIN PIND
#define LED_DDR DDRD
#define LED_PORT PORTD
#define LED_BIT (6)

#define led_init() bit_set(LED_DDR, BIT(LED_BIT))
#define led_on() bit_set(LED_PORT, BIT(LED_BIT))
#define led_off() bit_clear(LED_PORT, BIT(LED_BIT))
#define led_toggle() bit_flip(LED_PORT, BIT(LED_BIT))

#endif  /* BOARD_H */
