/**
 * @file diagnostics.c
 * @brief TODO.
 *
 */




#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <inttypes.h>

#include "board.h"
#include "hobd.h"
#include "time.h"
#include "canbus.h"
#include "diagnostics.h"




// *****************************************************
// static global types/macros
// *****************************************************

//
#define WARNING_STATE_IDX_CANBUS (0)
#define WARNING_STATE_IDX_OBDBUS (1)
#define WARNING_STATES_LENGTH (2)


//
typedef enum
{
    LED_STATE_OFF,
    LED_STATE_BLINK,
    LED_STATE_ON
} led_state_kind;


//
typedef struct
{
    //
    //
    uint16_t bit;
    //
    //
    uint32_t last_set;
} warning_set_state_s;




// *****************************************************
// static global data
// *****************************************************

//
static hobd_heartbeat_s hobd_heartbeat;


//
static uint32_t last_tx_heartbeat = 0;


//
static warning_set_state_s warning_states[ WARNING_STATES_LENGTH ];


//
static led_state_kind led_state = LED_STATE_OFF;


//
static uint32_t last_led_toggle = 0;


//
static const uint16_t CAN_ID_HEARTBEAT =
        (uint16_t) (HOBD_CAN_ID_HEARTBEAT_BASE + NODE_ID);


//
static const uint32_t led_blink_intervals[] =
{
    [LED_STATE_OFF] = 0,
    [LED_STATE_BLINK] = DIAGNOSTICS_BLINK_INTERVAL,
    [LED_STATE_ON] = 0
};




// *****************************************************
// static declarations
// *****************************************************


//
static void send_heartbeat(
        const uint32_t * const now,
        const uint8_t send_now );


//
static void clear_warn_set(
        const uint32_t * const now,
        const uint8_t clear_now );


//
static void update_led(
        const uint32_t * const now );




// *****************************************************
// static definitions
// *****************************************************

//
static void send_heartbeat(
        const uint32_t * const now,
        const uint8_t send_now )
{
    // get time since last publish
    const uint32_t delta = time_get_delta(
            &last_tx_heartbeat,
            now );

    // publish if interval met/exceeded
    if( (send_now != 0) || (delta >= HOBD_CAN_TX_INTERVAL_HEARTBEAT) )
    {
        // update last publish timestamp, ms
        last_tx_heartbeat = (*now);

        // update counter
        hobd_heartbeat.counter += 1;

        // publish
        const uint8_t ret = canbus_send(
                CAN_ID_HEARTBEAT,
                (uint8_t) sizeof(hobd_heartbeat),
                (const uint8_t*) &hobd_heartbeat );
        if( ret != 0 )
        {
            diagnostics_set_warn( HOBD_HEARTBEAT_WARN_CANBUS );
        }
    }
}


//
static void clear_warn_set(
        const uint32_t * const now,
        const uint8_t clear_now )
{
    uint8_t idx = 0;
    const uint16_t warnings = diagnostics_get_warn();

    for( idx = 0; idx < WARNING_STATES_LENGTH; idx += 1 )
    {
        // if warning is currently set
        if( (warnings & warning_states[ idx ].bit) != 0 )
        {
            // get time since last set
            const uint32_t delta = time_get_delta(
                    &warning_states[ idx ].last_set,
                    now );

            // clear if interval met/exceeded
            if( (clear_now != 0) || (delta >= DIAGNOSTICS_WARN_SET_CLEAR_INTERVAL) )
            {
                // update last set timestamp, ms
                warning_states[ idx ].last_set = (*now);

                // clear warning bit
                diagnostics_clear_warn( warning_states[ idx ].bit );
            }
        }
    }
}


//
static void update_led(
        const uint32_t * const now )
{
    const uint8_t connected = ((hobd_heartbeat.warning_register & HOBD_HEARTBEAT_WARN_NO_OBD_ECU) == 0) ? 1 : 0;

    // update state
    if( hobd_heartbeat.state != HOBD_HEARTBEAT_STATE_OK )
    {
        // node is in an error state
        led_state = LED_STATE_OFF;
    }
    else if( connected != 0 )
    {
        // connected
        led_state = LED_STATE_ON;
    }
    else
    {
        // waiting for data
        led_state = LED_STATE_BLINK;
    }

    // update LED
    if( led_state == LED_STATE_OFF )
    {
        led_off();
    }
    else if( led_state == LED_STATE_ON )
    {
        led_on();
    }
    else
    {
        // get time since last toggle
        const uint32_t delta = time_get_delta(
                &last_led_toggle,
                now );

        // toggle if interval met/exceeded
        if( delta >= led_blink_intervals[ led_state ] )
        {
            // update last toggle timestamp, ms
            last_led_toggle = (*now);

            led_toggle();
        }
    }
}




// *****************************************************
// public definitions
// *****************************************************

//
void diagnostics_init( void )
{
    hobd_heartbeat.hardware_version = HARDWARE_VERSION;
    hobd_heartbeat.firmware_version = FIRMWARE_VERSION;
    hobd_heartbeat.node_id = NODE_ID;
    hobd_heartbeat.state = HOBD_HEARTBEAT_STATE_INIT;
    hobd_heartbeat.counter = 0;
    hobd_heartbeat.error_register = 0;
    hobd_heartbeat.warning_register = 0;

    memset( &warning_states, 0, sizeof(warning_states) );

    led_off();
}


//
void diagnostics_set_state(
        const uint8_t state )
{
    if( state != hobd_heartbeat.state )
    {
        // get current time
        const uint32_t now = time_get_ms();

        hobd_heartbeat.state = state;

        // publish heartbeat message immediately
        send_heartbeat( &now, 1 );
    }
}


//
uint8_t diagnostics_get_state( void )
{
    return hobd_heartbeat.state;
}


//
void diagnostics_set_warn(
        const uint16_t warn )
{
    hobd_heartbeat.warning_register |= warn;

    if( (warn & HOBD_HEARTBEAT_WARN_CANBUS) != 0 )
    {
        warning_states[ WARNING_STATE_IDX_CANBUS ].last_set = time_get_ms();
    }

    if( (warn & HOBD_HEARTBEAT_WARN_OBDBUS) != 0 )
    {
        warning_states[ WARNING_STATE_IDX_OBDBUS ].last_set = time_get_ms();
    }
}


//
uint16_t diagnostics_get_warn( void )
{
    return hobd_heartbeat.warning_register;
}


//
void diagnostics_clear_warn(
        const uint16_t warn )
{
    hobd_heartbeat.warning_register &= ~warn;
}


//
void diagnostics_set_error(
        const uint16_t error )
{
    hobd_heartbeat.error_register |= error;
}


//
uint16_t diagnostics_get_error( void )
{
    return hobd_heartbeat.error_register;
}


//
void diagnostics_clear_error(
        const uint16_t error )
{
    hobd_heartbeat.error_register &= ~error;
}


//
void diagnostics_set_warn_timeout_bits( void )
{
    // get current time
    const uint32_t now = time_get_ms();

    warning_states[ WARNING_STATE_IDX_CANBUS ].bit = HOBD_HEARTBEAT_WARN_CANBUS;
    warning_states[ WARNING_STATE_IDX_CANBUS ].last_set = now;

    warning_states[ WARNING_STATE_IDX_OBDBUS ].bit = HOBD_HEARTBEAT_WARN_OBDBUS;
    warning_states[ WARNING_STATE_IDX_OBDBUS ].last_set = now;
}


//
void diagnostics_update( void )
{
    // get current time
    const uint32_t now = time_get_ms();

    // transition to an error state if any error bits are set
    if( hobd_heartbeat.error_register != 0 )
    {
        diagnostics_set_state( HOBD_HEARTBEAT_STATE_ERROR );
    }
    else
    {
        // recover from errors
        if( hobd_heartbeat.state == HOBD_HEARTBEAT_STATE_ERROR )
        {
            diagnostics_set_state( HOBD_HEARTBEAT_STATE_OK );
        }
    }

    // clear warning bits as needed
    clear_warn_set( &now, 0 );

    // update LED indicator
    update_led( &now );

    // publish heartbeat message as needed
    send_heartbeat( &now, 0 );
}
