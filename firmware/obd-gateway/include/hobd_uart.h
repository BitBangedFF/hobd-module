/**
 * @file hobd_uart.h
 * @brief Honda OBD Uart Protocol.
 *
 * @todo hobd protocol notes.
 *
 * Data rate: 10,400 bps
 *
 * Initialization sequence:
 *   -# pull k-line low for 70 ms
 *   -# return k-line to high state
 *   -# wait 120 ms
 *   -# send wake up packet - no response is expected
 *     - send {0xFE, 0x04, 0xFF, 0xFF}
 *   -# send initialization command packet - expect a response
 *     - send {0x72, 0x05, 0x00, 0xF0, 0x99}
 *     - expect {0x02, 0x04, 0x00, 0xFA}
 *
 * Checksum:
 * @todo - checksum notes.
 *
 */




#ifndef HOBD_UART_H
#define HOBD_UART_H




#include <inttypes.h>




//
#define HOBD_PACKET_SIZE_MAX (255)


//
#define HOBD_PACKET_TYPE_INVALID (0x00)


//
#define HOBD_PACKET_TYPE_QUERY (0x72)


//
#define HOBD_PACKET_TYPE_RESPONSE (0x02)


//
#define HOBD_PACKET_TYPE_WAKE_UP (0xFE)


//
#define HOBD_PACKET_SUBTYPE_WAKE_UP (0xFF)


//
#define HOBD_PACKET_SUBTYPE_INIT_COMMAND (0x00)


//
#define HOBD_PACKET_SUBTYPE_TABLE_SUBGROUP (0x72)


//
#define HOBD_PACKET_SUBTYPE_TABLE (0x71)


//
#define HOBD_INIT_COMMAND_DATA (0xF0)


//
#define HOBD_TABLE_0 (0x00)


//
#define HOBD_TABLE_16 (0x10)


//
#define HOBD_TABLE_32 (0x20)


//
#define HOBD_TABLE_209 (0xD1)




//
typedef struct
{
    //
    //
    uint8_t type;
    //
    //
    uint8_t size;
    //
    //
    uint8_t subtype;
} hobd_packet_header_s;


//
typedef struct
{
    //
    //
    hobd_packet_header_s header;
    //
    //
    uint8_t table;
    //
    //
    uint8_t register_offset;
    //
    //
    uint8_t register_cnt;
} hobd_table_query_s;


//
typedef struct
{
    //
    //
    hobd_packet_header_s header;
    //
    //
    uint8_t table;
    //
    //
    uint8_t register_offset;
    //
    // register data follows after register_offset
} hobd_table_response_s;


//
typedef struct
{
    //
    //
    hobd_packet_header_s header;
    //
    //
    uint8_t data;
} hobd_init_command_s;


//
typedef struct
{
    //
    //
    uint16_t engine_rpm;
    //
    //
    uint8_t tps_volt;
    //
    //
    uint8_t tps_percent;
    //
    //
    uint8_t ect_volt;
    //
    //
    uint8_t ect_temp;
    //
    //
    uint8_t iat_volt;
    //
    //
    uint8_t iat_temp;
    //
    //
    uint8_t map_volt;
    //
    //
    uint8_t map_pressure;
    //
    //
    uint8_t reserved_0;
    //
    //
    uint8_t reserved_1;
    //
    //
    uint8_t battery_volt;
    //
    //
    uint8_t wheel_speed;
    //
    //
    uint16_t fuel_injectors;
} hobd_table_16_s;


//
typedef struct
{
    //
    //
    uint8_t gear;
    //
    //
    uint8_t reserved_0;
    //
    //
    uint8_t reserved_1;
    //
    //
    uint8_t reserved_2;
    //
    //
    uint8_t engine_on;
} hobd_table_209_s;




#endif  /* HOBD_UART_H */
