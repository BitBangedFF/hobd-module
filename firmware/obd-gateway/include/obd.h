/**
 * @file obd.h
 * @brief TODO.
 *
 */




#ifndef OBD_H
#define	OBD_H




#include <inttypes.h>

#include "hobd.h"




//
#define OBD_GROUP_NONE_READY (0x0000)
#define OBD_GROUP_A_READY (1 << 0)
#define OBD_GROUP_B_READY (1 << 1)


// ms
#define OBD_RX_WARN_TIMEOUT (5000UL)




// OBD message data group
typedef struct
{
    //
    //
    uint16_t ready_groups;
    //
    // OBD_GROUP_A_READY
    struct
    {
        //
        //
        hobd_obd_time_s time;
        //
        //
        hobd_obd1_s obd1;
        //
        //
        hobd_obd2_s obd2;
    } group_a;
    //
    // OBD_GROUP_B_READY
    struct
    {
        //
        //
        hobd_obd_time_s time;
        //
        //
        hobd_obd3_s obd3;
    } group_b;
} obd_data_s;




//
uint8_t obd_init( void );


//
void obd_disable( void );


//
void obd_enable( void );


//
void obd_set_group_ready(
        const uint16_t group );


//
void obd_clear_group_ready(
        const uint16_t group );


//
void obd_clear_all_group_ready( void );


//
uint8_t obd_is_group_ready(
        const uint16_t group );


//
uint8_t obd_update( void );




#endif	/* OBD_H */
