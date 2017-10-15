/**
 * @file ecu.c
 * @brief TODO.
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <stdint.h>
#include "board.h"
#include "time.h"
#include "obd_uart.h"
#include "hobd_parser.h"
#include "ecu.h"

#define GPIO_WAIT_DELAY (60)

// OBD UART Rx used for GPIO
#define GPIO_PORT_DDR   (DDRD)
#define GPIO_PORT_OUT   (PORTD)
#define GPIO_PORT_IN    (PIND)
#define GPIO_RX_PIN     (2)
#define GPIO_RX_MASK    (1 << GPIO_RX_PIN)

#define gpio_in_pu_off() { GPIO_PORT_DDR &= ~GPIO_RX_MASK; GPIO_PORT_OUT &= ~GPIO_RX_PIN; }
#define gpio_get() (GPIO_PORT_IN & GPIO_RX_MASK)

typedef enum
{
    ECU_STATE_GPIO_WAIT = 0,
    ECU_STATE_WAKEUP_WAIT,
    ECU_STATE_INIT_WAIT,
    ECU_STATE_ACTIVE
} ecu_state_kind;

static uint8_t rx_buffer[HOBD_MSG_SIZE_MAX];

static hobd_msg_header_s * const rx_header =
        (hobd_msg_header_s*) &rx_buffer[0];

static hobd_msg_s * const rx_msg =
        (hobd_msg_s*) &rx_buffer[0];

static hobd_parser_s hobd_parser;

static ecu_state_kind ecu_state = ECU_STATE_GPIO_WAIT;

static __attribute__((always_inline)) inline uint8_t check_for_message(void)
{
    uint8_t msg_type = HOBD_MSG_TYPE_INVALID;

    const uint16_t rx_data = obd_uart_getc();

    if(rx_data != RING_BUFFER_NO_DATA)
    {
        const uint8_t status = hobd_parser_parse_byte(
                RING_BUFFER_GET_DATA_BYTE(rx_data),
                &hobd_parser);

        if(status == 0)
        {
            msg_type = rx_header->type;
        }
    }

    return msg_type;
}

static __attribute__((always_inline)) inline void gpio_wait_update(void)
{   
    gpio_in_pu_off();

    if(gpio_get() == 0)
    {
        time_delay_ms(GPIO_WAIT_DELAY);

        while(gpio_get() == 0);

        time_delay_ms(GPIO_WAIT_DELAY);

        if(gpio_get() != 0)
        {
            ecu_state = ECU_STATE_WAKEUP_WAIT;
            obd_uart_init();
            hobd_parser_reset(&hobd_parser);
        }
    }
}

static __attribute__((always_inline)) inline void wakeup_wait_update(void)
{   
    const uint8_t msg_type = check_for_message();

    if(msg_type == HOBD_MSG_TYPE_WAKE_UP)
    {
        if(rx_header->subtype == HOBD_MSG_SUBTYPE_WAKE_UP)
        {
            ecu_state = ECU_STATE_WAKEUP_WAIT;
            hobd_parser_reset(&hobd_parser);
        }
    }
}

static __attribute__((always_inline)) inline void init_wait_update(void)
{
    const uint8_t msg_type = check_for_message();

    if(msg_type == HOBD_MSG_TYPE_QUERY)
    {
        if(rx_header->subtype == HOBD_MSG_SUBTYPE_INIT)
        {
            if(rx_msg->data[0] == HOBD_INIT_DATA)
            {
                ecu_state = ECU_STATE_ACTIVE;
                hobd_parser_reset(&hobd_parser);
            }
        }
    }
}

static __attribute__((always_inline)) inline void active_update(void)
{
    const uint8_t msg_type = check_for_message();

    if(msg_type == HOBD_MSG_TYPE_QUERY)
    {
        if(rx_header->subtype == HOBD_MSG_SUBTYPE_TABLE)
        {
            // TODO
            led_toggle();
        }
        else if(rx_header->subtype == HOBD_MSG_SUBTYPE_TABLE_SUBGROUP)
        {   
            // TODO
            led_toggle();
        }
    }
}

void ecu_init(void)
{
    obd_uart_deinit();

    hobd_parser_init(
            &rx_buffer[0],
            sizeof(rx_buffer),
            &hobd_parser);

    ecu_state = ECU_STATE_GPIO_WAIT;
}

void ecu_deinit(void)
{
    obd_uart_deinit();
    ecu_state = ECU_STATE_GPIO_WAIT;
}

void ecu_update(void)
{
    // TODO - need to hook up the timeout mechanism
    // TODO - add response messages

    if(ecu_state == ECU_STATE_GPIO_WAIT)
    {
        gpio_wait_update();
    }
    else if(ecu_state == ECU_STATE_WAKEUP_WAIT)
    {
        wakeup_wait_update();
    }
    else if(ecu_state == ECU_STATE_INIT_WAIT)
    {
        init_wait_update();
    }
    else if(ecu_state == ECU_STATE_ACTIVE)
    {
        active_update();
    }
}
