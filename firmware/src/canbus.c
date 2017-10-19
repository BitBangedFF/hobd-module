/**
 * @file canbus.c
 * @brief TODO.
 *
 */

#include <avr/io.h>
#include <stdint.h>
#include "board.h"
#include "can_lib.h"
#include "error.h"
#include "hobd_can.h"
#include "diagnostics.h"
#include "canbus.h"

void canbus_init(void)
{
    // wait for CAN to initialize or wdt will reset
    while(can_init(0) == 0);
}

uint8_t canbus_send(
        const uint16_t id,
        const uint8_t dlc,
        const uint8_t * const data)
{
    uint8_t ret = ERR_OK;
    uint8_t status = CAN_STATUS_ERROR;
    st_cmd_t cmd;

    // construct canlib command
    cmd.id.std = id;
    cmd.dlc = dlc;
    cmd.pt_data = (uint8_t*) data;
    cmd.status = 0;
    cmd.ctrl.rtr = 0;
    cmd.ctrl.ide = 0;

    // command type - send data
    cmd.cmd = CMD_TX_DATA;

    // send the data to an available MOB
    do
    {
        status = can_cmd(&cmd);
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

    if(ret == ERR_OK)
    {
        diagnostics_clear_warn(HOBD_HEARTBEAT_WARN_CANBUS_TX);
    }

    return ret;
}
