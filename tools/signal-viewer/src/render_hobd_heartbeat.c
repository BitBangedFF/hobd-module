/**
 * @file render_hobd_heartbeat.c
 * @brief TODO.
 *
 */




#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>

#include "gl_headers.h"
#include "math_util.h"
#include "time_domain.h"
#include "render.h"
#include "config.h"
#include "signal_table_def.h"




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

//
static GLdouble render_errors(
        const uint16_t errors,
        const GLdouble base_x,
        const GLdouble base_y )
{
    char string[512];
    GLdouble delta_y = 0.0;
    const void * const font = GLUT_BITMAP_HELVETICA_10;

    if( (errors & HOBD_HEARTBEAT_ERROR_OBDBUS) != 0 )
    {
        snprintf(
            string,
            sizeof(string),
            "- ERROR OBD BUS" );

        render_text_2d(
                base_x,
                base_y + delta_y,
                string,
                font );

        delta_y += 15.0;
    }

    if( (errors & HOBD_HEARTBEAT_ERROR_CANBUS) != 0 )
    {
        snprintf(
            string,
            sizeof(string),
            "- ERROR CAN BUS" );

        render_text_2d(
                base_x,
                base_y + delta_y,
                string,
                font );

        delta_y += 15.0;
    }

    return delta_y;
}


//
static GLdouble render_warnings(
        const uint16_t warnings,
        const GLdouble base_x,
        const GLdouble base_y )
{
    char string[512];
    GLdouble delta_y = 0.0;
    const void * const font = GLUT_BITMAP_HELVETICA_10;

    if( (warnings & HOBD_HEARTBEAT_WARN_OBDBUS_RX) != 0 )
    {
        snprintf(
            string,
            sizeof(string),
            "- WARN OBD BUS RX" );

        render_text_2d(
                base_x,
                base_y + delta_y,
                string,
                font );

        delta_y += 15.0;
    }

    if( (warnings & HOBD_HEARTBEAT_WARN_OBDBUS_TX) != 0 )
    {
        snprintf(
            string,
            sizeof(string),
            "- WARN OBD BUS TX" );

        render_text_2d(
                base_x,
                base_y + delta_y,
                string,
                font );

        delta_y += 15.0;
    }

    if( (warnings & HOBD_HEARTBEAT_WARN_OBDBUS_RX_OVERFLOW) != 0 )
    {
        snprintf(
            string,
            sizeof(string),
            "- WARN OBD BUS RX OVERFLOW" );

        render_text_2d(
                base_x,
                base_y + delta_y,
                string,
                font );

        delta_y += 15.0;
    }

    if( (warnings & HOBD_HEARTBEAT_WARN_CANBUS_RX) != 0 )
    {
        snprintf(
            string,
            sizeof(string),
            "- WARN CAN BUS RX" );

        render_text_2d(
                base_x,
                base_y + delta_y,
                string,
                font );

        delta_y += 15.0;
    }

    if( (warnings & HOBD_HEARTBEAT_WARN_CANBUS_TX) != 0 )
    {
        snprintf(
            string,
            sizeof(string),
            "- WARN CAN BUS TX" );

        render_text_2d(
                base_x,
                base_y + delta_y,
                string,
                font );

        delta_y += 15.0;
    }

    return delta_y;
}




// *****************************************************
// public definitions
// *****************************************************

//
void render_hobd_heartbeat(
        const config_s * const config,
        const hobd_heartbeat_s * const data,
        const GLdouble base_x,
        const GLdouble base_y )
{
    char string[512];
    GLdouble delta_y = 5.0;
    const GLdouble bound_x = 355.0;
    const GLdouble text_yoff = 15.0;
    const GLdouble text_xoff = 5.0;

    render_line(
            base_x,
            base_y,
            base_x + bound_x,
            base_y );

    snprintf(
            string,
            sizeof(string),
            "hardware_version                         : %lu",
            (unsigned long) data->hardware_version );

    render_text_2d(
            base_x + text_xoff,
            base_y + text_yoff,
            string,
            NULL );

    render_line(
            base_x,
            base_y + text_yoff + delta_y,
            base_x + bound_x,
            base_y + text_yoff + delta_y );

    delta_y += 15.0;

    snprintf(
            string,
            sizeof(string),
            "firmware_version                           : %lu",
            (unsigned long) data->firmware_version );

    render_text_2d(
            base_x + text_xoff,
            base_y + text_yoff + delta_y,
            string,
            NULL );

    delta_y += 5.0;

    render_line(
            base_x,
            base_y + text_yoff + delta_y,
            base_x + bound_x,
            base_y + text_yoff + delta_y );

    delta_y += 15.0;

    snprintf(
            string,
            sizeof(string),
            "node_id                                       : 0x%02lX",
            (unsigned long) data->node_id );

    render_text_2d(
            base_x + text_xoff,
            base_y + text_yoff + delta_y,
            string,
            NULL );

    delta_y += 5.0;

    render_line(
            base_x,
            base_y + text_yoff + delta_y,
            base_x + bound_x,
            base_y + text_yoff + delta_y );

    delta_y += 15.0;

    snprintf(
            string,
            sizeof(string),
            "state                                            : %lu",
            (unsigned long) data->state );

    render_text_2d(
            base_x + text_xoff,
            base_y + text_yoff + delta_y,
            string,
            NULL );

    delta_y += 5.0;

    render_line(
            base_x,
            base_y + text_yoff + delta_y,
            base_x + bound_x,
            base_y + text_yoff + delta_y );

    delta_y += 15.0;

    snprintf(
            string,
            sizeof(string),
            "counter                                        : %lu",
            (unsigned long) data->counter );

    render_text_2d(
            base_x + text_xoff,
            base_y + text_yoff + delta_y,
            string,
            NULL );

    delta_y += 5.0;

    render_line(
            base_x,
            base_y + text_yoff + delta_y,
            base_x + bound_x,
            base_y + text_yoff + delta_y );

    delta_y += 15.0;

    snprintf(
            string,
            sizeof(string),
            "warning_register                           : 0x%04lX",
            (unsigned long) data->warning_register );

    render_text_2d(
            base_x + text_xoff,
            base_y + text_yoff + delta_y,
            string,
            NULL );

    delta_y += 5.0;

    render_line(
            base_x,
            base_y + text_yoff + delta_y,
            base_x + bound_x,
            base_y + text_yoff + delta_y );

    delta_y += 15.0;

    snprintf(
            string,
            sizeof(string),
            "error_register                                : 0x%04lX",
            (unsigned long) data->error_register );

    render_text_2d(
            base_x + text_xoff,
            base_y + text_yoff + delta_y,
            string,
            NULL );

    delta_y += 5.0;

    render_line(
            base_x,
            base_y + text_yoff + delta_y,
            base_x + bound_x,
            base_y + text_yoff + delta_y );

    delta_y += 20.0;

    delta_y += render_errors(
            data->error_register,
            base_x + 10.0,
            base_y + text_yoff + delta_y );

    delta_y += 10.0;

    delta_y += render_warnings(
            data->warning_register,
            base_x + 10.0,
            base_y + text_yoff + delta_y );
}
