/**
 * @file display_manager.h
 * @brief TODO.
 *
 */




#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H




#include <signal.h>
#include "gl_headers.h"
#include "time_domain.h"
#include "signal_table.h"
#include "config.h"




//
#define DM_WINDOW_ID_INVALID (-1)


// 33 ms ~= 30 FPS
#define DM_REDRAW_INTERVAL (33ULL)




/**
 * @brief Display manager context data.
 *
 */
typedef struct
{
    //
    //
    int gl_argc; /*!< GL argument count. */
    //
    //
    char **gl_argv; /*!< GL arugment list. */
    //
    //
    GLenum gl_error; /*!< GL error value. */
    //
    //
    int win_id; /*!< Window identifier.
                 * Value \ref GUI_WINDOW_ID_INVALID means invalid. */
    //
    // monotonic
    timestamp_ms last_redraw_time;
    //
    //
    st_state_s st_state;
    //
    //
    sig_atomic_t *exit_signal_ptr;
    //
    //
    config_s config;
} dm_context_s;




//
dm_context_s * dm_get_context( void );


//
int dm_init(
        const char * const window_title,
        const unsigned long window_width,
        const unsigned long window_height,
        sig_atomic_t * const exit_signal );


//
void dm_release( void );


//
void dm_update(
        timestamp_ms * const time_to_redraw );




#endif /* DISPLAY_MANAGER_H */
