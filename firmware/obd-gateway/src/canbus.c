/**
 * @file canbus.c
 * @brief TODO.
 *
 */


#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <inttypes.h>

#include "board.h"
#include "error.h"
#include "can_lib.h"
#include "canbus.h"


uint8_t canbus_init( void )
{
    uint8_t ret = ERR_OK;

    // wait for CAN to initialize or wdt will reset
    while(can_init(0) == 0)
    {
        ret = ERR_CANBUS_INIT;
    }

    return ret;
}


uint8_t canbus_send(
        const uint16_t id,
        const uint8_t dlc,
        const uint8_t * const data )
{
    uint8_t ret = ERR_OK;
    uint8_t status = CAN_STATUS_ERROR;
    st_cmd_t cmd;

    // zero state
    cmd.status = 0;
    cmd.ctrl.rtr = 0;
    cmd.ctrl.ide = 0;

    // construct canlib command
    cmd.id.std = id;
    cmd.dlc = dlc;
    cmd.pt_data = (uint8_t*) data;

    // command type - send data
    cmd.cmd = CMD_TX_DATA;

    // send the data to an available MOB
    do
    {
        status = can_cmd( &cmd );
    }
    while(status != CAN_CMD_ACCEPTED);

    // wait for completion
    do
    {
        status = can_get_status(&cmd);

        if(status == CAN_STATUS_ERROR)
        {
            ret = ERR_CANBUS_TX;

            // force return now
            status = CAN_STATUS_COMPLETED;
        }
    }
    while(status != CAN_STATUS_COMPLETED);

    return ret;
}
