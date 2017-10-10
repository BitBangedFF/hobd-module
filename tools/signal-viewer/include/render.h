/**
 * @file render.h
 * @brief TODO.
 *
 */




#ifndef RENDER_H
#define RENDER_H




#include "gl_headers.h"
#include "signal_table_def.h"




//
void render_text_2d(
        const GLdouble cx,
        const GLdouble cy,
        const char * const text,
        const void * const font );


//
void render_line(
        const GLdouble x1,
        const GLdouble y1,
        const GLdouble x2,
        const GLdouble y2 );


//
void render_table_base(
        const signal_table_s * const table,
        const GLdouble base_x,
        const GLdouble base_y );




#endif /* RENDER_H */
