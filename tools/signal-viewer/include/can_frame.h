/**
 * @file can_frame.h
 * @brief TODO.
 *
 */




#ifndef CAN_FRAME_H
#define CAN_FRAME_H




#include "time_domain.h"




//
typedef struct
{
    //
    //
    unsigned long id;
    //
    //
    unsigned long dlc;
    //
    //
    timestamp_ms native_rx_timestamp;
    //
    //
    timestamp_ms rx_timestamp;
    //
    //
    timestamp_ms rx_timestamp_mono;
    //
    //
    unsigned char data[8];
} can_frame_s;




#endif /* CAN_FRAME_H */
