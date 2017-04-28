/**
 * @file canbus.c
 * @brief TODO.
 *
 */


#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <stdint.h>

#include "board.h"
#include "can_drv.h"
#include "can_lib.h"

#include "error.h"
#include "canbus.h"


static st_cmd_t rx_cmd;


static void clear_rx( void )
{
    rx_cmd.status = STATUS_CLEARED;
    rx_cmd.cmd = CMD_NONE;
    rx_cmd.ctrl.rtr = 0;
    rx_cmd.ctrl.ide = 0;
}


static void cmd_rx(
        const uint8_t dlc,
        uint8_t * const data_buffer )
{
    rx_cmd.id.std = 0;
    rx_cmd.dlc = dlc;
    rx_cmd.pt_data = (uint8_t*) data_buffer;

    rx_cmd.cmd = CMD_TX_DATA;

    // send the data to an available MOB
    uint8_t status = CAN_STATUS_ERROR;
    do
    {
        status = can_cmd(&rx_cmd);
    }
    while(status != CAN_CMD_ACCEPTED);
}


void canbus_init( void )
{
    clear_rx();

    // wait for CAN to initialize or wdt will reset
    while(can_init(0) == 0) {}
}


uint8_t canbus_send(
        const uint16_t id,
        const uint8_t dlc,
        const uint8_t * const data )
{
    uint8_t ret = ERR_OK;
    uint8_t status = CAN_STATUS_ERROR;
    st_cmd_t cmd;

    cmd.status = 0;
    cmd.ctrl.rtr = 0;
    cmd.ctrl.ide = 0;

    cmd.id.std = id;
    cmd.dlc = dlc;
    cmd.pt_data = (uint8_t*) data;

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

    return ret;
}


uint8_t canbus_recv(
        uint16_t * const id,
        uint8_t * const dlc,
        uint8_t * const data )
{
    uint8_t ret = ERR_NO_DATA;

    if(rx_cmd.cmd == CMD_NONE)
    {
        cmd_rx(*dlc, data);
    }

    // check for completion
    uint8_t status = can_get_status(&rx_cmd);

    if(status == CAN_STATUS_ERROR)
    {
        ret = ERR_CANBUS_RX;
        rx_cmd.cmd = CMD_ABORT;
        (void) can_cmd(&rx_cmd);
        clear_rx();
    }
    else if(status == CAN_STATUS_COMPLETED)
    {
        *id = (uint16_t) rx_cmd.id.std;
        (*dlc) = (uint8_t) rx_cmd.dlc;

        clear_rx();
        cmd_rx(*dlc, data);
        ret = ERR_OK;
    }

    return ret;
}
