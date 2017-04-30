/**
 * @file sd_protocol.h
 * @brief TODO.
 *
 */


#ifndef SD_PROTOCOL_H
#define	SD_PROTOCOL_H


#include <stdint.h>


#define SD_MSG_HEADER_SIZE (4)

#define SD_MSG_PREAMBLE (0xAB)

#define SD_FILE_START_WORD (0xFF0101AB)

#define SD_MSG_TYPE_INVALID (0x00)
#define SD_MSG_TYPE_FILE_START (0x01)
#define SD_MSG_TYPE_CAN_FRAME (0x05)


typedef struct
{
    uint8_t preamble;
    uint8_t type;
    uint8_t size;
} sd_msg_header_s;

typedef struct
{
    sd_msg_header_s header;
    uint8_t data[0];
} sd_msg_s;

typedef struct
{
    uint32_t rx_timestamp;
    uint16_t id;
    uint8_t dlc;
    uint8_t data[8];
} sd_can_frame_s;

typedef struct
{
    sd_msg_header_s header;
    sd_can_frame_s frame;
} sd_msg_can_frame_s;


#endif	/* SD_PROTOCOL_H */
