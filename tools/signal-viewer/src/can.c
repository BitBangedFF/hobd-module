/**
 * @file can.c
 * @brief TODO.
 *
 */




#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "canlib.h"
#include "time_domain.h"
#include "can_frame.h"
#include "can.h"




// *****************************************************
// static global types/macros
// *****************************************************




// *****************************************************
// static global data
// *****************************************************




// *****************************************************
// static declarations
// *****************************************************




// *****************************************************
// static definitions
// *****************************************************




// *****************************************************
// public definitions
// *****************************************************

//
can_handle_s can_open(
        const unsigned long system_id )
{
    canHandle handle = canOpenChannel( system_id, 0x40 );

    if( handle < 0 )
    {
        printf( "failed to open CAN channel at index %lu\n", system_id );
        handle = CAN_HANDLE_INVALID;
    }

    if( handle >= 0 )
    {
        const canStatus can_stat1 = canSetBusParams(
                handle,
                BAUD_500K,
                4,
                3,
                2,
                1,
                0 );
        if( can_stat1 != canOK )
        {
            printf( "failed to set CAN bus parameters\n" );
            (void) canClose( handle );
            handle = CAN_HANDLE_INVALID;
        }
    }

    if( handle >= 0 )
    {
        const canStatus can_stat2 = canSetBusOutputControl(
                handle,
                canDRIVER_NORMAL );
        if( can_stat2 != canOK )
        {
            printf( "failed to set CAN bus output control\n" );
            (void) canClose( handle );
            handle = CAN_HANDLE_INVALID;
        }
    }

    if( handle >= 0 )
    {
        const canStatus can_stat3 = canBusOn( handle );
        if( can_stat3 != canOK )
        {
            printf( "failed to set CAN bus on-state\n" );
            (void) canClose( handle );
            handle = CAN_HANDLE_INVALID;
        }
    }

    return handle;
}


//
void can_close(
        const can_handle_s handle )
{
    if( handle >= 0 )
    {
        (void) canClose( (canHandle) handle );
    }
}


//
int can_read(
        const can_handle_s handle,
        const timestamp_ms timeout,
        can_frame_s * const frame )
{
    int ret = 1;

    if( handle >= 0 )
    {
        canStatus can_stat = canOK;
        long msg_id = 0;
        unsigned int msg_dlc = 0;
        unsigned int msg_flag = 0;
        unsigned long tstamp = 0;

        if( timeout == 0 )
        {
            // returns immediately
            can_stat = canRead(
                    (canHandle) handle,
                    &msg_id,
                    (void*) frame->data,
                    &msg_dlc,
                    &msg_flag,
                    &tstamp );
        }
        else
        {
            // read with timeout
            can_stat = canReadWait(
                    (canHandle) handle,
                    &msg_id,
                    (void*) frame->data,
                    &msg_dlc,
                    &msg_flag,
                    &tstamp,
                    (unsigned long) timeout );
        }

        if( can_stat == canOK )
        {
            frame->rx_timestamp = time_get_timestamp();
            frame->rx_timestamp_mono = time_get_monotonic_timestamp();
            frame->native_rx_timestamp = (timestamp_ms) tstamp;
            frame->id = (unsigned long) msg_id;
            frame->dlc  = (unsigned long) msg_dlc;

            ret = 0;
        }
    }

    return ret;
}
