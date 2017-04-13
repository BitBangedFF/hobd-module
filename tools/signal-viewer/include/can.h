/**
 * @file can.h
 * @brief TODO.
 *
 */




#ifndef CAN_H
#define CAN_H




#include "time_domain.h"
#include "can_frame.h"




//
#define CAN_HANDLE_INVALID (-1)




//
typedef int can_handle_s;




//
can_handle_s can_open(
        const unsigned long system_id );


//
void can_close(
        const can_handle_s handle );


//
int can_read(
        const can_handle_s handle,
        const timestamp_ms timeout,
        can_frame_s * const frame );


//
can_handle_s can_replay_open(
        const char * const file );


//
void can_replay_close(
        const can_handle_s handle );


//
int can_replay_read(
        const can_handle_s handle,
        const timestamp_ms timeout,
        can_frame_s * const frame );




#endif /* CAN_H */
