/**
 * @file hobd_parser.h
 * @brief TODO.
 *
 */


#ifndef HOBD_PARSER_H
#define	HOBD_PARSER_H


#include <stdint.h>

#include "hobd_protocol.h"


typedef enum
{
    HOBD_PARSER_STATE_GET_TYPE = 0,
    HOBD_PARSER_STATE_GET_SIZE,
    HOBD_PARSER_STATE_GET_SUBTYPE,
    HOBD_PARSER_STATE_GET_DATA,
    HOBD_PARSER_STATE_GET_CHECKSUM
} hobd_parser_state_kind;


typedef struct
{
    hobd_parser_state_kind state;
    uint8_t bytes_read;
    uint8_t data_size;
    hobd_msg_header_s header;
    uint8_t data[HOBD_MSG_SIZE_MAX];
    uint16_t checksum_part;
    uint8_t checksum;
} hobd_parser_s;


void hobd_parser_init(
        hobd_parser_s * const parser );


uint8_t hobd_parser_parse_byte(
        const uint8_t byte,
        hobd_parser_s * const parser );


#endif	/* HOBD_PARSER_H */
