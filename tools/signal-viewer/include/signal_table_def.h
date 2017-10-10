/**
 * @file signal_table_def.h
 * @brief TODO.
 *
 */




#ifndef SIGNAL_TABLE_DEF_H
#define SIGNAL_TABLE_DEF_H




#include "time_domain.h"




// enforce 1 byte alignment so we can do linear packing
#pragma pack(push)
#pragma pack(1)


#include "hobd.h"




//
typedef struct
{
    //
    //
    timestamp_ms native_rx_time;
    //
    //
    timestamp_ms rx_time;
    //
    //
    timestamp_ms rx_time_mono;
    //
    //
    unsigned long can_id;
    //
    //
    unsigned long can_dlc;
    //
    //
    char table_name[ 256 ];
    //
    //
    union
    {
        //
        //
        unsigned char buffer[ 8 ];
        //
        //
        hobd_heartbeat_s heartbeat_obd_gateway;
        //
        //
        hobd_obd_time_s obd_time;
        hobd_obd_uptime_s obd_uptime;
        //
        //
        hobd_obd1_s obd1;
        //
        //
        hobd_obd2_s obd2;
        //
        //
        hobd_obd3_s obd3;
    };
} signal_table_s;


// restore alignment
#pragma pack(pop)




#endif /* SIGNAL_TABLE_DEF_H */
