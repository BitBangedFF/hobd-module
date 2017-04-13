/**
 * @file render_hobd_obd_time.c
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




// *****************************************************
// public definitions
// *****************************************************

//
void render_hobd_obd_time(
        const config_s * const config,
        const hobd_obd_time_s * const data,
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
            "rx_time                                         : %lu",
            (unsigned long) data->rx_time );

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
            "counter_1                                     : %lu",
            (unsigned long) data->counter_1 );

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
            "counter_2                                     : %lu",
            (unsigned long) data->counter_2 );

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
}
