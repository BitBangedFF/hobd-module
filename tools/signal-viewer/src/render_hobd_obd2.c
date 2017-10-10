/**
 * @file render_hobd_obd2.c
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
void render_hobd_obd2(
        const config_s * const config,
        const hobd_obd2_s * const data,
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
            "ect_volt                                       : %lu",
            (unsigned long) data->ect_volt );

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
            "ect_temp                                     : %lu",
            (unsigned long) data->ect_temp );

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
            "iat_volt                                        : %lu",
            (unsigned long) data->iat_volt );

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
            "iat_temp                                      : %lu",
            (unsigned long) data->iat_temp );

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
            "map_volt                                     : %lu",
            (unsigned long) data->map_volt );

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
            "map_pressure                              : %lu",
            (unsigned long) data->map_pressure );

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
            "fuel_injectors                               : %lu",
            (unsigned long) data->fuel_injectors );

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
