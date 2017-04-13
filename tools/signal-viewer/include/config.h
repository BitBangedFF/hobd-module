/**
 * @file config.h
 * @brief TODO.
 *
 */




#ifndef CONFIG_H
#define	CONFIG_H




#include <stdbool.h>




//
#ifndef TRUE
#define TRUE true
#endif
#ifndef FALSE
#define FALSE false
#endif




//
typedef struct
{
    //
    //
    unsigned long win_width; /*!< Window width. [pixels] */
    //
    //
    unsigned long win_height; /*!< Window height. [pixels] */
    //
    //
    char win_title[ 1024 ]; /*!< Window title string. */
    //
    //
    bool freeze_frame_enabled;
    //
    //
    unsigned long active_page_index;
} config_s;




#endif	/* CONFIG_H */
