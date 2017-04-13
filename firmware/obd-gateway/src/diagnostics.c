/**
 * @file diagnostics.c
 * @brief TODO.
 *
 */


#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <stdint.h>

#include "board.h"

#include "error.h"
#include "time.h"
#include "canbus.h"
#include "hobd.h"
#include "diagnostics.h"


static hobd_heartbeat_s hobd_heartbeat_data;
static hobd_obd_uptime_s hobd_uptime_data;

static uint32_t last_tx_heartbeat;
static uint32_t last_tx_uptime;
static uint32_t last_led_toggle;

static const uint16_t CAN_ID_HEARTBEAT =
        (uint16_t) (HOBD_CAN_ID_HEARTBEAT_BASE + NODE_ID);


static void send_uptime(
        const uint32_t * const now,
        const uint8_t send_now )
{
    const uint32_t delta = time_get_delta(
            &last_tx_uptime,
            now);

    if((send_now != 0) || (delta >= HOBD_CAN_TX_INTERVAL_UPTIME))
    {
        last_tx_uptime = (*now);
        hobd_uptime_data.uptime = time_get_seconds();

        const uint8_t ret = canbus_send(
                HOBD_CAN_ID_OBD_UPTIME,
                (uint8_t) sizeof(hobd_uptime_data),
                (const uint8_t*) &hobd_uptime_data);

        if(ret != ERR_OK)
        {
            diagnostics_set_warn(HOBD_HEARTBEAT_WARN_CANBUS_TX);
        }
    }
}


static void send_heartbeat(
        const uint32_t * const now,
        const uint8_t send_now )
{
    const uint32_t delta = time_get_delta(
            &last_tx_heartbeat,
            now);

    if((send_now != 0) || (delta >= HOBD_CAN_TX_INTERVAL_HEARTBEAT))
    {
        last_tx_heartbeat = (*now);
        hobd_heartbeat_data.counter += 1;

        const uint8_t ret = canbus_send(
                CAN_ID_HEARTBEAT,
                (uint8_t) sizeof(hobd_heartbeat_data),
                (const uint8_t*) &hobd_heartbeat_data);

        if(ret != ERR_OK)
        {
            diagnostics_set_warn(HOBD_HEARTBEAT_WARN_CANBUS_TX);
        }
    }
}


static void update_led(
        const uint32_t * const now )
{
    const uint8_t connected = (hobd_heartbeat_data.warning_register & HOBD_HEARTBEAT_WARN_OBDBUS_RX);

    if(hobd_heartbeat_data.state != HOBD_HEARTBEAT_STATE_OK)
    {
        led_off();
    }
    else if(connected != 0)
    {
        led_on();
    }
    else
    {
        const uint32_t delta = time_get_delta(
                &last_led_toggle,
                now);

        if(delta >= DIAGNOSTICS_BLINK_INTERVAL)
        {
            last_led_toggle = (*now);
            led_toggle();
        }
    }
}


void diagnostics_init( void )
{
    led_off();

    last_tx_heartbeat = 0;
    last_tx_uptime = 0;
    last_led_toggle = 0;

    hobd_heartbeat_data.hardware_version = HARDWARE_VERSION;
    hobd_heartbeat_data.firmware_version = FIRMWARE_VERSION;
    hobd_heartbeat_data.node_id = NODE_ID;
    hobd_heartbeat_data.state = HOBD_HEARTBEAT_STATE_INIT;
    hobd_heartbeat_data.counter = 0;
    hobd_heartbeat_data.error_register = 0;
    hobd_heartbeat_data.warning_register = 0;
}


void diagnostics_set_state(
        const uint8_t state )
{
    if(state != hobd_heartbeat_data.state)
    {
        const uint32_t now = time_get_ms();

        hobd_heartbeat_data.state = state;

        // publish immediately
        send_heartbeat(&now, 1);
    }
}


uint8_t diagnostics_get_state( void )
{
    return hobd_heartbeat_data.state;
}


void diagnostics_set_warn(
        const uint16_t warn )
{
    hobd_heartbeat_data.warning_register |= warn;
}


uint16_t diagnostics_get_warn( void )
{
    return hobd_heartbeat_data.warning_register;
}


void diagnostics_clear_warn(
        const uint16_t warn )
{
    hobd_heartbeat_data.warning_register &= ~warn;
}


void diagnostics_set_error(
        const uint16_t error )
{
    hobd_heartbeat_data.error_register |= error;
}


uint16_t diagnostics_get_error( void )
{
    return hobd_heartbeat_data.error_register;
}


void diagnostics_clear_error(
        const uint16_t error )
{
    hobd_heartbeat_data.error_register &= ~error;
}


void diagnostics_update( void )
{
    const uint32_t now = time_get_ms();

    // transition to an error state if any error bits are set
    if(hobd_heartbeat_data.error_register != 0)
    {
        diagnostics_set_state(HOBD_HEARTBEAT_STATE_ERROR);
    }
    else
    {
        // recover from errors
        if(hobd_heartbeat_data.state == HOBD_HEARTBEAT_STATE_ERROR)
        {
            diagnostics_set_state(HOBD_HEARTBEAT_STATE_OK);
        }
    }

    update_led(&now);

    send_heartbeat(&now, 0);

    send_uptime(&now, 0);
}
