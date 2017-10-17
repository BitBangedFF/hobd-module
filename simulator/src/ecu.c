/**
 * @file ecu.c
 * @brief TODO.
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <string.h>
#include <stdint.h>
#include "board.h"
#include "time.h"
#include "obd_uart.h"
#include "hobd_parser.h"
#include "ecu.h"

#define GPIO_WAIT_DELAY (60)
#define NO_DATA_TIMEOUT (1000)

// OBD UART Rx used for GPIO
#define GPIO_PORT_DDR   (DDRD)
#define GPIO_PORT_OUT   (PORTD)
#define GPIO_PORT_IN    (PIND)
#define GPIO_RX_PIN     (2)
#define GPIO_RX_MASK    (1 << GPIO_RX_PIN)

#define gpio_in_pu_off() { GPIO_PORT_DDR &= ~GPIO_RX_MASK; GPIO_PORT_OUT &= ~GPIO_RX_PIN; }
#define gpio_in_pu_on() { GPIO_PORT_DDR &= ~GPIO_RX_MASK; GPIO_PORT_OUT |= GPIO_RX_PIN; }
#define gpio_get() (GPIO_PORT_IN & GPIO_RX_MASK)

typedef enum
{
    ECU_STATE_GPIO_WAIT = 0,
    ECU_STATE_WAKEUP_WAIT,
    ECU_STATE_INIT_WAIT,
    ECU_STATE_ACTIVE
} ecu_state_kind;

static uint8_t table_0_buffer[HOBD_TABLE_SIZE_MAX];
static uint8_t table_16_buffer[HOBD_TABLE_SIZE_MAX];
static uint8_t table_32_buffer[HOBD_TABLE_SIZE_MAX];
static uint8_t table_209_buffer[HOBD_TABLE_SIZE_MAX];

static uint8_t rx_buffer[HOBD_MSG_SIZE_MAX];
static uint8_t tx_buffer[HOBD_MSG_SIZE_MAX];

static hobd_msg_header_s * const rx_header = (hobd_msg_header_s*) &rx_buffer[0];
static hobd_msg_s * const rx_msg = (hobd_msg_s*) &rx_buffer[0];
static hobd_msg_header_s * const tx_header = (hobd_msg_header_s*) &tx_buffer[0];
static hobd_msg_s * const tx_msg = (hobd_msg_s*) &tx_buffer[0];

static hobd_parser_s hobd_parser;
static ecu_state_kind ecu_state = ECU_STATE_GPIO_WAIT;
static uint32_t last_update = 0;

static void handle_rx_message(void)
{
    if(rx_header->type == HOBD_MSG_TYPE_QUERY)
    {
        if(rx_header->subtype == HOBD_MSG_SUBTYPE_INIT)
        {
            tx_header->type = HOBD_MSG_TYPE_RESPONSE;
            tx_header->size = HOBD_MSG_HEADERCS_SIZE;
            tx_header->subtype = HOBD_MSG_SUBTYPE_INIT;

            tx_msg->data[0] = hobd_parser_checksum(
                    &tx_buffer[0],
                    tx_header->size - 1);

            obd_uart_send(&tx_buffer[0], tx_header->size);
        }
    }
    else if(
            (rx_header->subtype == HOBD_MSG_SUBTYPE_TABLE_SUBGROUP)
            || (rx_header->subtype == HOBD_MSG_SUBTYPE_TABLE))
    {
        const hobd_data_table_query_s * const query =
                (hobd_data_table_query_s*) &rx_msg->data[0];

        hobd_data_table_response_s * const resp =
                (hobd_data_table_response_s*) &tx_msg->data[0];

        tx_header->type = HOBD_MSG_TYPE_RESPONSE;
        tx_header->size = HOBD_MSG_HEADERCS_SIZE;
        tx_header->subtype = rx_header->subtype;

        tx_header->size += (uint8_t) sizeof(*resp);
        tx_header->size += query->count;

        resp->table = query->table;
        resp->offset = query->offset;

        if(query->count != 0)
        {
            uint8_t *src_table_ptr;

            if(query->table == HOBD_TABLE_0)
            {
                src_table_ptr = &table_0_buffer[0];
            }
            else if(query->table == HOBD_TABLE_16)
            {
                src_table_ptr = &table_16_buffer[0];
            }
            else if(query->table == HOBD_TABLE_32)
            {
                src_table_ptr = &table_32_buffer[0];
            }
            else if(query->table == HOBD_TABLE_209)
            {
                src_table_ptr = &table_209_buffer[0];
            }
            else
            {
                src_table_ptr = &table_0_buffer[0];
            }

            (void) memcpy(
                    &resp->data[0],
                    &src_table_ptr[0],
                    query->count);
        }

        tx_msg->data[tx_header->size] = hobd_parser_checksum(
                    &tx_buffer[0],
                    tx_header->size - 1);

        obd_uart_send(&tx_buffer[0], tx_header->size);
    }
}

static uint8_t check_for_message(void)
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
            led_toggle();
            msg_type = rx_header->type;
            last_update = time_get_ms();
        }
    }

    return msg_type;
}

static void gpio_wait_update(void)
{
    gpio_in_pu_on();

    if(gpio_get() == 0)
    {
        time_delay_ms(GPIO_WAIT_DELAY);

        while(gpio_get() == 0);

        wdt_reset();

        time_delay_ms(GPIO_WAIT_DELAY);

        if(gpio_get() != 0)
        {
            ecu_state = ECU_STATE_WAKEUP_WAIT;
            obd_uart_init();
            hobd_parser_reset(&hobd_parser);
            last_update = time_get_ms();
        }
    }
}

static void wakeup_wait_update(void)
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

static void init_wait_update(void)
{
    const uint8_t msg_type = check_for_message();

    if(msg_type == HOBD_MSG_TYPE_QUERY)
    {
        if(rx_header->subtype == HOBD_MSG_SUBTYPE_INIT)
        {
            if(rx_msg->data[0] == HOBD_INIT_DATA)
            {
                ecu_state = ECU_STATE_ACTIVE;
                handle_rx_message();
                hobd_parser_reset(&hobd_parser);
            }
        }
    }
}

static void active_update(void)
{
    const uint8_t msg_type = check_for_message();

    if(msg_type == HOBD_MSG_TYPE_QUERY)
    {
        handle_rx_message();
    }
}

void ecu_init(void)
{
    obd_uart_deinit();

    led_on();

    hobd_parser_init(
            &rx_buffer[0],
            sizeof(rx_buffer),
            &hobd_parser);

    ecu_state = ECU_STATE_GPIO_WAIT;

    last_update = time_get_ms();
}

void ecu_deinit(void)
{
    obd_uart_deinit();
    ecu_state = ECU_STATE_GPIO_WAIT;
}

void ecu_update(void)
{
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

void ecu_check_timeout_reset(void)
{
    const uint32_t now = time_get_ms();

    const uint32_t delta = time_get_delta(&last_update, &now);

    if(delta >= NO_DATA_TIMEOUT)
    {
        ecu_init();
    }
}
