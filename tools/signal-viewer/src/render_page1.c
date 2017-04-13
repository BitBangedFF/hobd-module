/**
 * @file render_page1.c
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
void render_hobd_heartbeat(
        const config_s * const config,
        const hobd_heartbeat_s * const data,
        const GLdouble base_x,
        const GLdouble base_y );




// *****************************************************
// static definitions
// *****************************************************




// *****************************************************
// public definitions
// *****************************************************

//
void render_page1(
        const config_s * const config,
        st_state_s * const state )
{
    glPushMatrix();

    // get table pointers
    signal_table_s * const table0 = st_get_table_by_can_id(
            HOBD_CAN_ID_HEARTBEAT_OBD_GATEWAY,
            state );

    // render tables
    if( table0 != NULL )
    {
        render_table_base( table0, 5.0, 40.0 );
        render_hobd_heartbeat(
                config,
                &table0->heartbeat_obd_gateway,
                20.0,
                80.0 );
    }

    glPopMatrix();
}
