/**
 * @file main.c
 * @brief TODO.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

#include "math_util.h"
#include "can.h"
#include "display_manager.h"


// *****************************************************
// static global types/macros
// *****************************************************

#define WINDOW_WIDTH (800UL)
#define WINDOW_HEIGHT (600UL)
#define WINDOW_TITLE "HOBD CAN Signal Viewer"




// *****************************************************
// static global data
// *****************************************************

/**
 * @brief Flag indicating exit signal was caught.
 *
 */
static sig_atomic_t global_exit_signal = 0;




// *****************************************************
// static declarations
// *****************************************************


/**
 * @brief Signal handler.
 *
 * @param [in] signal Signal value to handle.
 *
 */
static void sig_handler( int signal );




// *****************************************************
// static definitions
// *****************************************************

//
static void sig_handler( int sig )
{
    if( sig == SIGINT )
    {
        global_exit_signal = 1;
    }
}




// *****************************************************
// public definitions
// *****************************************************

//
int main( int argc, char **argv )
{
    // CAN bus interface handle
    can_handle_s can_handle = CAN_HANDLE_INVALID;
    unsigned int can_is_replay = 0;
    char title[256];

    // hook up the control-c signal handler, sets exit signaled flag
    (void) signal( SIGINT, sig_handler );

    // allow signals to interrupt
    (void) siginterrupt( SIGINT, 1 );

    // format base title string
    snprintf(
            title,
            sizeof(title),
            "%s",
            WINDOW_TITLE );

    // check if CAN channel system ID or replay file path was provided
    if( (argc == 2) && (strlen(argv[1]) > 0) )
    {
        if( isdigit(argv[1][0]) != 0 )
        {
            const long arg_val = atol( argv[1] );

            if( arg_val >= 0 )
            {
                printf( "connected to CAN channel with system index %lu\n", (unsigned long) arg_val );

                can_handle = can_open( (unsigned long) arg_val );

                strncat(
                        title,
                        " - Live Mode",
                        sizeof(title) );
            }
        }
//        else if( strstr(argv[1], "plog" ) != NULL )
//        {
//            printf( "opening replay file: '%s'\n", argv[1] );
//
//            can_is_replay = 1;
//
//            can_handle = can_replay_open( argv[1] );
//
//            strncat(
//                    title,
//                    " - Replay Mode",
//                    sizeof(title) );
//        }
    }
    else
    {
        strncat(
                title,
                " - Test Mode",
                sizeof(title) );
    }

    // init display manager
    const int dm_init_status = dm_init(
            title,
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            &global_exit_signal );
    if( dm_init_status != 0 )
    {
        printf( "dm_init return %d\n", dm_init_status );
        global_exit_signal = 1;
    }

    // wait for user to control-c
    while( global_exit_signal == 0 )
    {
        // update display manager
        timestamp_ms time_to_redraw = 0;
        dm_update( &time_to_redraw );

        // poll for CAN frames if enabled
        if( can_handle != CAN_HANDLE_INVALID )
        {
            // check for any ready CAN frames to process
            can_frame_s rx_frame;
            int can_status = 1;

            if( can_is_replay == 0 )
            {
                can_status = can_read(
                        can_handle,
                        m_min(time_to_redraw, 5),
                        &rx_frame );
            }
//            else
//            {
//                can_status = can_replay_read(
//                        can_handle,
//                        m_min(time_to_redraw, 5),
//                        &rx_frame );
//            }

            // if data ready
            if( can_status == 0 )
            {
                dm_context_s * const dm_context = dm_get_context();

                st_process_can_frame(
                        &rx_frame,
                        &dm_context->config,
                        &dm_context->st_state );
            }
        }
        else
        {
            // otherwise just sleep
            if( time_to_redraw > 0 )
            {
                time_sleep_ms( m_min(time_to_redraw,5) );
            }
        }
    }

    if( can_is_replay == 0 )
    {
        can_close( can_handle );
    }
//    else
//    {
//        can_replay_close( can_handle );
//    }

    dm_release();

	return EXIT_SUCCESS;
}
