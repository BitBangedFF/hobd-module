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

// board definition/configuration
#include "board.h"

// BSP utilities
#include "timer8_drv.h"
#include "timer16_drv.h"
#include "uart_drv.h"
#include "uart_lib.h"
#include "rtc_drv.h"
#include "can_drv.h"
#include "can_lib.h"

// FAT FS library
#include "fat_filelib.h"

// project includes
#include "error.h"
#include "debug.h"
#include "time.h"
#include "canbus.h"
#include "sd.h"
#include "hobd.h"
#include "sd_protocol.h"


#ifdef BUILD_TYPE_DEBUG
#warning "BUILD_TYPE_DEBUG ON"
#endif


static sd_msg_can_frame_s can_frame_msg;


static void init( void )
{
    wdt_disable();

    sw0_init();
    sw0_enable_pullup();
    sw1_init();
    sw1_enable_pullup();

    led_init();
    led_off();

    wdt_enable(WDTO_500MS);
    wdt_reset();

    rtc_int_init();
    enable_interrupt();

#ifdef BUILD_TYPE_DEBUG
    Uart_select(DEBUG_UART);
    uart_init(CONF_8BIT_NOPAR_1STOP, DEBUG_BAUDRATE);
#endif

    wdt_reset();
    canbus_init();
    sd_init();

    DEBUG_PRINTF("module '%s' initialized\n", MODULE_NAME);
}


int main( void )
{
    init();

    (void) memset(&can_frame_msg, 0, sizeof(can_frame_msg));

    sd_open();

    while(1)
    {
        wdt_reset();

        can_frame_msg.frame.id = 0;
        can_frame_msg.frame.dlc = (uint8_t) sizeof(can_frame_msg.frame.data);

        const uint8_t status = canbus_recv(
                &can_frame_msg.frame.id,
                &can_frame_msg.frame.dlc,
                can_frame_msg.frame.data);

        if(status == ERR_OK)
        {
            led_on();

            can_frame_msg.frame.rx_timestamp = time_get_ms();

            can_frame_msg.header.preamble = SD_MSG_PREAMBLE;
            can_frame_msg.header.type = SD_MSG_TYPE_CAN_FRAME;
            can_frame_msg.header.size = (uint8_t) sizeof(can_frame_msg.frame);

            sd_write(
                    (const uint8_t*) &can_frame_msg,
                    sizeof(can_frame_msg),
                    1);

            led_off();
        }

        if(sw0_get_state() == TRUE)
        {
            led_on();
            while(sw0_get_state() == TRUE)
            {
                wdt_reset();
            }

            sd_close();
            led_off();
        }

        if(time_get_timer() != 0)
        {
            time_clear_timer();
        }
    }

   return 0;
}
