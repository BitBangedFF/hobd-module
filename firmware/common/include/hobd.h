/**
 * @file hobd.h
 * @brief TODO.
 *
 */


#ifndef HOBD_H
#define	HOBD_H


#include <stdint.h>


// ms
#define HOBD_CAN_TX_INTERVAL_HEARTBEAT (50)
#define HOBD_CAN_TX_INTERVAL_UPTIME (1000)

#define HOBD_CAN_ID_HEARTBEAT_BASE (0x010)
#define HOBD_CAN_ID_HEARTBEAT_OBD_GATEWAY (0x015)

#define HOBD_CAN_ID_COMMAND (0x020)
#define HOBD_CAN_ID_RESPONSE (0x021)

#define HOBD_CAN_ID_OBD_TIME (0x080)
#define HOBD_CAN_ID_OBD1 (0x081)
#define HOBD_CAN_ID_OBD2 (0x082)
#define HOBD_CAN_ID_OBD3 (0x083)
#define HOBD_CAN_ID_OBD_UPTIME (0x084)

#define HOBD_HEARTBEAT_STATE_INVALID (0x00)
#define HOBD_HEARTBEAT_STATE_INIT (0x01)
#define HOBD_HEARTBEAT_STATE_OK (0x02)
#define HOBD_HEARTBEAT_STATE_ERROR (0x03)

#define HOBD_HEARTBEAT_WARN_OBDBUS_RX (1 << 1)
#define HOBD_HEARTBEAT_WARN_OBDBUS_TX (1 << 2)
#define HOBD_HEARTBEAT_WARN_OBDBUS_RX_OVERFLOW (1 << 3)
#define HOBD_HEARTBEAT_WARN_CANBUS_RX (1 << 5)
#define HOBD_HEARTBEAT_WARN_CANBUS_TX (1 << 6)

#define HOBD_HEARTBEAT_ERROR_OBDBUS (1 << 1)
#define HOBD_HEARTBEAT_ERROR_CANBUS (1 << 4)


typedef struct
{
    uint8_t hardware_version : 4;
    uint8_t firmware_version : 4;
    uint8_t node_id;
    uint8_t state;
    uint8_t counter;
    uint16_t warning_register;
    uint16_t error_register;
} hobd_heartbeat_s;


typedef struct
{
    uint8_t id;
    uint8_t key;
    uint16_t data_0;
    uint32_t data_1;
} hobd_command_s;


typedef struct
{
    uint8_t cmd_id;
    uint8_t key;
    uint16_t data_0;
    uint32_t data_1;
} hobd_response_s;


/**
 * @brief On-board diagnostics time message.
 *
 * Message size (CAN frame DLC): 8 bytes
 * CAN frame ID: \ref HOBD_CAN_ID_OBD_TIME
 * Transmit rate: TODO ms
 *
 */
typedef struct
{
    uint32_t rx_time;
    uint16_t counter_1;
    uint16_t counter_2;
} hobd_obd_time_s;


/**
 * @brief On-board diagnostics uptime message.
 *
 * Message size (CAN frame DLC): 4 bytes
 * CAN frame ID: \ref HOBD_CAN_ID_OBD_UPTIME
 * Transmit rate: TODO ms
 *
 */
typedef struct
{
    uint32_t uptime; /*!< Module uptime. [seconds] */
} hobd_obd_uptime_s;


/**
 * @brief On-board diagnostics 1 message.
 *
 * Message size (CAN frame DLC): 6 bytes
 * CAN frame ID: \ref HOBD_CAN_ID_OBD1
 * Transmit rate: TODO ms
 *
 */
typedef struct
{
    uint16_t engine_rpm;
    uint8_t wheel_speed;
    uint8_t battery_volt;
    uint8_t tps_volt;
    uint8_t tps_percent;
} hobd_obd1_s;


/**
 * @brief On-board diagnostics 2 message.
 *
 * Message size (CAN frame DLC): 8 bytes
 * CAN frame ID: \ref HOBD_CAN_ID_OBD2
 * Transmit rate: TODO ms
 *
 */
typedef struct
{
    uint8_t ect_volt;
    uint8_t ect_temp;
    uint8_t iat_volt;
    uint8_t iat_temp;
    uint8_t map_volt;
    uint8_t map_pressure;
    uint16_t fuel_injectors;
} hobd_obd2_s;


/**
 * @brief On-board diagnostics 3 message.
 *
 * Message size (CAN frame DLC): 1 bytes
 * CAN frame ID: \ref HOBD_CAN_ID_OBD3
 * Transmit rate: TODO ms
 *
 */
typedef struct
{
    uint8_t engine_on : 1;
    uint8_t gear : 4;
    uint8_t reserved : 3;
} hobd_obd3_s;


#endif	/* HOBD_H */
