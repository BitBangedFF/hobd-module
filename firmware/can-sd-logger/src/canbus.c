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


// 15 MOBs available
#define RX_MOB_CNT (2)


typedef struct
{
    st_cmd_t state;
    uint8_t buffer[8];
} can_rx_mob;


static uint8_t rx_mob_index;
static can_rx_mob rx_mob_array[RX_MOB_CNT];


static can_rx_mob * get_rx_mob( void )
{
    return &rx_mob_array[rx_mob_index];
}


static void inc_rx_mob( void )
{
    rx_mob_index += 1;

    if(rx_mob_index >= RX_MOB_CNT)
    {
        rx_mob_index = 0;
    }
}


static void clear_rx_mob(
        can_rx_mob * const mob )
{
    mob->state.status = STATUS_CLEARED;
    mob->state.cmd = CMD_NONE;
    mob->state.ctrl.rtr = 0;
    mob->state.ctrl.ide = 0;
}


static void load_rx_mob(
        can_rx_mob * const mob )
{
    mob->state.id.std = 0;
    mob->state.dlc = sizeof(mob->buffer);
    mob->state.pt_data = (uint8_t*) mob->buffer;
    mob->state.cmd = CMD_RX;

    // send the data to an available MOB
    uint8_t status = CAN_STATUS_ERROR;
    do
    {
        status = can_cmd(&mob->state);
    }
    while(status != CAN_CMD_ACCEPTED);
}


static void init_rx_mobs( void )
{
    (void) memset(rx_mob_array, 0, sizeof(rx_mob_array));

    rx_mob_index = 0;

    uint8_t idx;
    for(idx = 0; idx < RX_MOB_CNT; idx += 1)
    {
        clear_rx_mob(&rx_mob_array[idx]);
    }

    for(idx = 0; idx < RX_MOB_CNT; idx += 1)
    {
        load_rx_mob(&rx_mob_array[idx]);
    }
}


void canbus_init( void )
{
    // wait for CAN to initialize or wdt will reset
    while(can_init(0) == 0);

    init_rx_mobs();
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

    uint8_t idx;
    for(idx = 0; (idx < RX_MOB_CNT) && (ret == ERR_NO_DATA); idx += 1)
    {
        can_rx_mob * const rx_mob = get_rx_mob();

        if(rx_mob->state.cmd == CMD_NONE)
        {
            load_rx_mob(rx_mob);
        }

        const uint8_t status = can_get_status(&rx_mob->state);

        if(status == CAN_STATUS_ERROR)
        {
            ret = ERR_CANBUS_RX;
            rx_mob->state.cmd = CMD_ABORT;
            (void) can_cmd(&rx_mob->state);
            clear_rx_mob(rx_mob);
        }
        else if(status == CAN_STATUS_COMPLETED)
        {
            *id = (uint16_t) rx_mob->state.id.std;
            *dlc = (uint8_t) rx_mob->state.dlc;
            (void) memcpy(data, rx_mob->buffer, rx_mob->state.dlc);

            clear_rx_mob(rx_mob);
            load_rx_mob(rx_mob);

            ret = ERR_OK;
        }

        inc_rx_mob();
    }

    return ret;
}
