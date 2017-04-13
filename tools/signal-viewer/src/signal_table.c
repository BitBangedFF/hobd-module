/**
 * @file signal_table.c
 * @brief TODO.
 *
 */




#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "gl_headers.h"
#include "math_util.h"
#include "time_domain.h"
#include "can_frame.h"
#include "render.h"
#include "signal_table.h"




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
void render_page1(
        const config_s * const config,
        st_state_s * const state );


//
void render_page2(
        const config_s * const config,
        st_state_s * const state );




// *****************************************************
// static definitions
// *****************************************************

//
static void render_page_header(
        const config_s * const config,
        const st_state_s * const state,
        const unsigned long page_number )
{
    char string[512];
    char date[64];
    const GLdouble base_x = 0.0;
    const GLdouble base_y = 25.0;
    const GLdouble bound_x = 1200.0;
    const GLdouble text_yoff = 15.0;
    const GLdouble date_xoff = 5.0;
    const GLdouble mstime_xoff = 200.0;
    const GLdouble monotime_xoff = 400.0;
    const GLdouble page_xoff = 600.0;

    glLineWidth( 2.0f );

    render_line(
            base_x,
            base_y,
            base_x + bound_x,
            base_y );

    snprintf(
            string,
            sizeof(string),
            "%s",
            asctime_r( time_get_localtime( state->last_update ), date ) );

    render_text_2d(
            date_xoff,
            text_yoff,
            string,
            NULL );

    snprintf(
            string,
            sizeof(string),
            "ms: %llu",
            state->last_update );

    render_text_2d(
            mstime_xoff,
            text_yoff,
            string,
            NULL );

    snprintf(
            string,
            sizeof(string),
            "ms-mono: %llu",
            state->last_update_mono );

    render_text_2d(
            monotime_xoff,
            text_yoff,
            string,
            NULL );

    snprintf(
            string,
            sizeof(string),
            "Page %lu / %lu",
            page_number,
            ST_PAGE_COUNT );

    render_text_2d(
            page_xoff,
            text_yoff,
            string,
            NULL );
}




// *****************************************************
// public definitions
// *****************************************************

//
void st_init(
        const config_s * const config,
        st_state_s * const state )
{
    unsigned long index = 0;

    {
        signal_table_s * const table = &state->signal_tables[ index++ ];

        table->can_id = HOBD_CAN_ID_HEARTBEAT_OBD_GATEWAY;
        table->can_dlc = (unsigned long) sizeof( table->heartbeat_obd_gateway );
        snprintf(
                table->table_name,
                sizeof(table->table_name),
                "OBD Heartbeat" );
    }

    {
        signal_table_s * const table = &state->signal_tables[ index++ ];

        table->can_id = HOBD_CAN_ID_OBD_TIME;
        table->can_dlc = (unsigned long) sizeof( table->obd_time );
        snprintf(
                table->table_name,
                sizeof(table->table_name),
                "OBD Time" );
    }

    {
        signal_table_s * const table = &state->signal_tables[ index++ ];

        table->can_id = HOBD_CAN_ID_OBD1;
        table->can_dlc = (unsigned long) sizeof( table->obd1 );
        snprintf(
                table->table_name,
                sizeof(table->table_name),
                "OBD 1" );
    }

    {
        signal_table_s * const table = &state->signal_tables[ index++ ];

        table->can_id = HOBD_CAN_ID_OBD2;
        table->can_dlc = (unsigned long) sizeof( table->obd2 );
        snprintf(
                table->table_name,
                sizeof(table->table_name),
                "OBD 2" );
    }

    {
        signal_table_s * const table = &state->signal_tables[ index++ ];

        table->can_id = HOBD_CAN_ID_OBD3;
        table->can_dlc = (unsigned long) sizeof( table->obd3 );
        snprintf(
                table->table_name,
                sizeof(table->table_name),
                "OBD 3" );
    }
}


//
void st_render(
        const config_s * const config,
        st_state_s * const state )
{
    glPushMatrix();

    glColor4d( 0.0, 0.0, 0.0, 1.0 );

    if( config->freeze_frame_enabled == FALSE )
    {
        state->last_update = time_get_timestamp();
        state->last_update_mono = time_get_monotonic_timestamp();
    }

    // render page header
    render_page_header( config, state, (config->active_page_index + 1) );

    // render page
    if( config->active_page_index == ST_PAGE_1 )
    {
        render_page1( config, state );
    }
    else if( config->active_page_index == ST_PAGE_2 )
    {
        render_page2( config, state );
    }

    glPopMatrix();
}


//
signal_table_s *st_get_table_by_can_id(
        const unsigned long can_id,
        st_state_s * const state )
{
    signal_table_s *table = NULL;

    unsigned long idx = 0;
    for( idx = 0; (idx < ST_SIGNAL_COUNT) && (table == NULL); idx += 1 )
    {
        if( state->signal_tables[ idx ].can_id == can_id )
        {
            table = &state->signal_tables[ idx ];
        }
    }

    return table;
}


//
void st_process_can_frame(
        const can_frame_s * const can_frame,
        const config_s * const config,
        st_state_s * const state )
{
    if( config->freeze_frame_enabled == FALSE )
    {
        // get a pointer to the data if we have a table for the frame
        signal_table_s * const table = st_get_table_by_can_id(
                can_frame->id,
                state );

        // copy new data
        if( table != NULL )
        {
            table->native_rx_time = can_frame->native_rx_timestamp;
            table->rx_time = can_frame->rx_timestamp;
            table->rx_time_mono = can_frame->rx_timestamp_mono;

            memcpy(
                    (void*) &table->buffer[ 0 ],
                    (void*) &can_frame->data[ 0 ],
                    (size_t) table->can_dlc );
        }
    }
}
