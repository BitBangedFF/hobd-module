/**
 * @file can_replay.c
 * @brief TODO.
 *
 */




#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <glib-2.0/glib.h>

#include "polysync_core.h"
#include "polysync_node_ref.h"
#include "polysync_message.h"
#include "polysync_logfile.h"
#include "polysync_can.h"
#include "time_domain.h"
#include "can_frame.h"
#include "can.h"




// *****************************************************
// static global types/macros
// *****************************************************




// *****************************************************
// static global data
// *****************************************************

//
static ps_node_ref node_ref = PSYNC_NODE_REF_INVALID;


//
static ps_msg_type can_frame_msg_type = PSYNC_MSG_TYPE_INVALID;


//
static GAsyncQueue *msg_queue = NULL;



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
can_handle_s can_replay_open(
        const char * const file )
{
    can_handle_s handle = CAN_HANDLE_INVALID;
    int ret = DTC_NONE;

    ret = psync_init(
            "hobd-can-reader",
            PSYNC_NODE_TYPE_API_USER,
            PSYNC_DEFAULT_DOMAIN,
            0,
            0,
            &node_ref );
    if( (ret != DTC_NONE) || (node_ref == PSYNC_NODE_REF_INVALID) )
    {
        printf( "failed to create PolySync node\n" );
    }

    if( ret == DTC_NONE )
    {
        ret = psync_message_get_type_by_name(
                node_ref,
                "ps_can_frame_msg",
                &can_frame_msg_type );
    }

    if( ret == DTC_NONE )
    {
        ret = psync_logfile_init( node_ref );
    }

    if( ret == DTC_NONE )
    {
        ret = psync_logfile_set_file_path(
                node_ref,
                file );
    }

    if( ret == DTC_NONE )
    {
        ps_msg_type reader_filter_list[1] = { can_frame_msg_type };
        ret = psync_logfile_set_message_type_filters(
                node_ref,
                NULL,
                0,
                reader_filter_list,
                1 );
    }

    if( ret == DTC_NONE )
    {
        ret = psync_logfile_enable_output_queue(
                node_ref,
                1 );
    }

    if( ret == DTC_NONE )
    {
        ret = psync_logfile_get_replay_msg_queue(
                node_ref,
                &msg_queue );
    }

    if( ret == DTC_NONE )
    {
        ret = psync_logfile_set_mode(
                node_ref,
                LOGFILE_MODE_READ,
                1 );
    }

    if( ret == DTC_NONE )
    {
        ret = psync_logfile_set_state(
                node_ref,
                LOGFILE_STATE_ENABLED,
                0 );
    }

    if( ret == DTC_NONE )
    {
        handle = 1;
    }

    return handle;
}


//
void can_replay_close(
        const can_handle_s handle )
{
    if( node_ref != PSYNC_NODE_REF_INVALID )
    {
        (void) psync_logfile_set_mode(
                node_ref,
                LOGFILE_MODE_OFF,
                PSYNC_RNR_SESSION_ID_INVALID );

        (void) psync_logfile_release( node_ref );

        (void) psync_release( &node_ref );
    }
}


//
int can_replay_read(
        const can_handle_s handle,
        const timestamp_ms timeout,
        can_frame_s * const frame )
{
    int ret = 1;

    if( (node_ref != PSYNC_NODE_REF_INVALID) && (msg_queue != NULL) )
    {
        // check for a message
        ps_msg_ref msg = PSYNC_MSG_REF_INVALID;

        if( timeout == 0 )
        {
            msg = g_async_queue_try_pop( msg_queue );
        }
        else
        {
            msg = g_async_queue_timeout_pop( msg_queue, (guint64) (timeout * 1000ULL) );
        }

        if( msg != PSYNC_MSG_REF_INVALID )
        {
            // get message type
            ps_msg_type msg_type = PSYNC_MSG_TYPE_INVALID;

            (void) psync_message_get_type( msg, &msg_type );

            if( msg_type == can_frame_msg_type )
            {
                const ps_can_frame_msg * const can_msg = (const ps_can_frame_msg*) msg;

                frame->rx_timestamp = (timestamp_ms) (can_msg->timestamp / 1000ULL);
                frame->rx_timestamp_mono = time_get_monotonic_timestamp();
                frame->native_rx_timestamp = (timestamp_ms) (can_msg->native_timestamp.value / 1000ULL);
                frame->id = (unsigned long) can_msg->id;
                frame->dlc  = (unsigned long) can_msg->data_buffer._length;

                if( can_msg->data_buffer._length != 0 )
                {
                    memcpy(
                            (void*) &frame->data[ 0 ],
                            (void*) can_msg->data_buffer._buffer,
                            (size_t) can_msg->data_buffer._length );
                }

                ret = 0;
            }

            (void) psync_message_free( node_ref, &msg );
        }
    }

    return ret;
}
