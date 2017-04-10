/**
 * @file main.c
 * @brief Main.
 *
 */




#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>

//
#include "board.h"

// BSP utilities
#include "timer8_drv.h"
#include "timer16_drv.h"
#include "uart_drv.h"
#include "uart_lib.h"
#include "rtc_drv.h"
#include "can_drv.h"
#include "can_lib.h"

//
#include "hobd.h"
#include "time.h"
#include "canbus.h"
#include "diagnostics.h"
#include "obd.h"




#ifdef BUILD_TYPE_DEBUG
#warning "BUILD_TYPE_DEBUG ON"
#endif




// *****************************************************
// static global types/macros
// *****************************************************




// *****************************************************
// static global data
// *****************************************************




// *****************************************************
// static declarations
// *****************************************************

//
static void init( void );




// *****************************************************
// static definitions
// *****************************************************

//
static void init( void )
{
    // disable watchdog
    wdt_disable();

    // init on-board switch pin
    sw0_init();
    sw0_enable_pullup();

    // init LED pin
    led_init();
    led_off();

    // enable watchdog, reset after 120 ms
    wdt_enable( WDTO_120MS );

    // reset watchdog
    wdt_reset();

    //
    rtc_int_init();

    // enable interrupts
    enable_interrupt();

    //
    const uint8_t can_status = canbus_init();

    //
    diagnostics_init();

    // init OBD UART/module
    const uint8_t obd_status = obd_init();

#ifdef BUILD_TYPE_DEBUG
    // debug UART is used instead of IMU
    Uart_select( DEBUG_UART );
    uart_init( CONF_8BIT_NOPAR_1STOP, DEBUG_BAUDRATE );
#endif

    // enable interrupts
    enable_interrupt();

    // reset watchdog
    wdt_reset();

    //
    if( can_status != 0 )
    {
        diagnostics_set_error( HOBD_HEARTBEAT_ERROR_CANBUS );
        DEBUG_PUTS( "init : canbus_init fail\n" );
    }

    //
    if( obd_status != 0 )
    {
        diagnostics_set_error( HOBD_HEARTBEAT_WARN_OBDBUS );
        DEBUG_PUTS( "init : obd_init fail\n" );
    }

    // set the warn clear set, bits that are cleared on a timer
    diagnostics_set_warn_timeout_bits();

    diagnostics_set_warn( HOBD_HEARTBEAT_WARN_NO_OBD_ECU );

    time_sleep_ms( 5 );

    DEBUG_PUTS( "init : pass\n" );

    // reset watchdog
    wdt_reset();

    //
    diagnostics_update();
}




// *****************************************************
// main
// *****************************************************
int main( void )
{
    //
    init();

    // reset watchdog
    wdt_reset();

    //
    diagnostics_set_state( HOBD_HEARTBEAT_STATE_OK );

    //
    while( 1 )
    {
        // reset watchdog
        wdt_reset();

        // process any incoming OBD data, and potentially publish ready CAN frames
        const uint8_t obd_status = obd_update();

        // TODO - better error/warn handling
        if( obd_status != 0 )
        {
            diagnostics_set_warn( HOBD_HEARTBEAT_WARN_OBDBUS );
        }

        //
        diagnostics_update();
    }

   return 0;
}
