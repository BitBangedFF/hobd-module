/**
 * @file comm.c
 * @brief TODO.
 *
 */

#include <avr/io.h>
#include <avr/wdt.h>
#include <stdint.h>
#include "board.h"
#include "time.h"
#include "obd_uart.h"
#include "hobd_parser.h"
#include "comm.h"

#define ECU_WAKEUP_RETRY_INTERVAL (5000UL)

#define ECU_NO_DATA_TIMEOUT (1000UL)

#define INIT_MSG_TX_DELAY (5UL)

#define KLINE_INIT_LOW_TIME (70UL)
#define KLINE_INIT_WAIT_TIME (120UL)
#define KLINE_INIT_WAIT_TIME_HALF (60UL)

// using the Tx pin to drive the K-line
#define KLINE_PORT_DDR (DDRD)
#define KLINE_PORT_OUT (PORTD)
#define KLINE_PIN (3)
#define KLINE_MASK (1 << KLINE_PIN)

#define kline_in_pu_off() { KLINE_PORT_DDR &= ~KLINE_MASK; KLINE_PORT_OUT &= ~KLINE_PIN; }
#define kline_out() (KLINE_PORT_DDR |= KLINE_MASK)
#define kline_high() (KLINE_PORT_OUT |= KLINE_MASK)
#define kline_low() (KLINE_PORT_OUT &= ~KLINE_MASK)

typedef enum
{
    STATE_ECU_WAKEUP = 0,
    STATE_DIAG_INIT,
    STATE_ACTIVE
} state_kind;

static uint8_t rx_buffer[HOBD_MSG_SIZE_MAX];
static uint8_t tx_buffer[HOBD_MSG_SIZE_MAX];

static hobd_msg_s * const rx_msg = (hobd_msg_s*) &rx_buffer[0];
static hobd_msg_s * const tx_msg = (hobd_msg_s*) &tx_buffer[0];

static hobd_parser_s hobd_parser;
static state_kind comm_state;
static uint32_t last_update;

static void send_msg(
        const uint8_t type,
        const uint8_t size,
        const uint8_t subtype)
{
    if(size < HOBD_MSG_HEADERCS_SIZE)
    {
        hard_reset();
    }

    tx_msg->header.type = type;
    tx_msg->header.size = size;
    tx_msg->header.subtype = subtype;

    tx_msg->data[size - HOBD_MSG_HEADERCS_SIZE] = hobd_parser_checksum(
            &tx_buffer[0],
            size - 1);

    obd_uart_send(&tx_buffer[0], tx_msg->header.size);
}

static uint8_t recv_msg(void)
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
            msg_type = rx_msg->header.type;
            last_update = time_get_ms();
        }
    }

    return msg_type;
}

static void do_ecu_wakeup(void)
{
    const uint32_t now = time_get_ms();

    const uint32_t delta = time_get_delta(&last_update, &now);

    if(delta >= ECU_WAKEUP_RETRY_INTERVAL)
    {
        kline_out();

        kline_low();

        time_delay_ms(KLINE_INIT_LOW_TIME);

        kline_high();

        wdt_reset();
        time_delay_ms(KLINE_INIT_WAIT_TIME_HALF);
        wdt_reset();
        time_delay_ms(KLINE_INIT_WAIT_TIME_HALF);

        kline_in_pu_off();

        obd_uart_init();

        hobd_parser_reset(&hobd_parser);

        // send wake up message
        send_msg(
                HOBD_MSG_TYPE_WAKE_UP,
                HOBD_MSG_HEADERCS_SIZE,
                HOBD_MSG_SUBTYPE_WAKE_UP);

        time_delay_ms(INIT_MSG_TX_DELAY);

        // send the diagnostic init message
        tx_msg->data[0] = HOBD_INIT_DATA;
        send_msg(
                HOBD_MSG_TYPE_QUERY,
                HOBD_MSG_HEADERCS_SIZE + 1,
                HOBD_MSG_SUBTYPE_INIT);

        comm_state = STATE_DIAG_INIT;

        last_update = time_get_ms();
    }
}

static void do_diag_init(void)
{
    // wait for diagnostic init response
    const uint8_t msg_type = recv_msg();

    if(msg_type == HOBD_MSG_TYPE_RESPONSE)
    {
        if(rx_msg->header.subtype == HOBD_MSG_SUBTYPE_INIT)
        {
            comm_state = STATE_ACTIVE;
            last_update = time_get_ms();
        }
    }
}

static void do_active(void)
{
    // TODO
}

void comm_init(void)
{
    comm_deinit();

    hobd_parser_init(
            &rx_buffer[0],
            sizeof(rx_buffer),
            &hobd_parser);

    comm_state = STATE_ECU_WAKEUP;

    last_update = time_get_ms();
}

void comm_deinit(void)
{
    obd_uart_deinit();

    kline_in_pu_off();

    comm_state = STATE_ECU_WAKEUP;
    hobd_parser_reset(&hobd_parser);
}

void comm_update(void)
{
    if(comm_state == STATE_ECU_WAKEUP)
    {
        do_ecu_wakeup();
    }
    else if(comm_state == STATE_DIAG_INIT)
    {
        do_diag_init();
    }
    else if(comm_state == STATE_ACTIVE)
    {
        do_active();
    }
}

void comm_check_timeout_reset(void)
{
    if(comm_state != STATE_ECU_WAKEUP)
    {
        const uint32_t now = time_get_ms();

        const uint32_t delta = time_get_delta(&last_update, &now);

        if(delta >= ECU_NO_DATA_TIMEOUT)
        {
            comm_init();
        }
    }
}
