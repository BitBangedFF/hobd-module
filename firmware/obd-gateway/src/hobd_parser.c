/**
 * @file hobd_parser.c
 * @brief TODO.
 *
 */


#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <stdint.h>

#include "board.h"

#include "debug.h"
#include "error.h"
#include "time.h"
#include "hobd_protocol.h"
#include "hobd_parser.h"



static uint8_t valid_type(
        const uint8_t type )
{
    uint8_t valid_type;

    if( type == HOBD_MSG_TYPE_QUERY )
    {
        valid_type = type;
    }
    else if( type == HOBD_MSG_TYPE_RESPONSE )
    {
        valid_type = type;
    }
    else if( type == HOBD_MSG_TYPE_WAKE_UP )
    {
        valid_type = type;
    }
    else
    {
        valid_type = HOBD_MSG_TYPE_INVALID;
    }

    return valid_type;
}


static uint8_t valid_subtype(
        const uint8_t subtype )
{
    uint8_t valid_subtype;

    if( subtype == HOBD_MSG_SUBTYPE_WAKE_UP )
    {
        valid_subtype = subtype;
    }
    else if( subtype == HOBD_MSG_SUBTYPE_INIT )
    {
        valid_subtype = subtype;
    }
    else if( subtype == HOBD_MSG_SUBTYPE_TABLE_SUBGROUP )
    {
        valid_subtype = subtype;
    }
    else if( subtype == HOBD_MSG_SUBTYPE_TABLE )
    {
        valid_subtype = subtype;
    }
    else
    {
        valid_subtype = HOBD_MSG_SUBTYPE_INVALID;
    }

    return valid_subtype;
}


void hobd_parser_init(
        hobd_parser_s * const parser )
{
    parser->state = HOBD_PARSER_STATE_GET_TYPE;
    parser->header.type = HOBD_MSG_TYPE_INVALID;
    parser->header.size = 0;
    parser->bytes_read = 0;
    parser->data_size = 0;
    parser->checksum = 0;
}


uint8_t hobd_parser_parse_byte(
        const uint8_t byte,
        hobd_parser_s * const parser )
{
    uint8_t ret = ERR_NO_DATA;

    if( parser->state == HOBD_PARSER_STATE_GET_TYPE )
    {
        const uint8_t type = valid_type( byte );

        if( type != HOBD_MSG_TYPE_INVALID )
        {
            parser->checksum = 0;
            parser->data_size = 0;
            parser->header.type = type;
            parser->state = HOBD_PARSER_STATE_GET_SIZE;
        }
    }
    else if( parser->state == HOBD_PARSER_STATE_GET_SIZE )
    {
        if( byte >= HOBD_MSG_SIZE_MIN )
        {
            parser->header.size = byte;
            parser->data_size = (byte - HOBD_MSG_SIZE_MIN);
            parser->state = HOBD_PARSER_STATE_GET_SUBTYPE;
        }
    }
    else if( parser->state == HOBD_PARSER_STATE_GET_SUBTYPE )
    {
        const uint8_t subtype = valid_subtype( byte );

        if( subtype != HOBD_MSG_SUBTYPE_INVALID )
        {
            parser->header.subtype = subtype;

            if( parser->header.size > HOBD_MSG_SIZE_MIN )
            {
                parser->bytes_read = 0;
                parser->state = HOBD_PARSER_STATE_GET_DATA;
            }
            else
            {
                parser->bytes_read = 0;
                parser->state = HOBD_PARSER_STATE_GET_CHECKSUM;
            }
        }
        else
        {
            parser->state = HOBD_PARSER_STATE_GET_TYPE;
        }
    }
    else if( parser->state == HOBD_PARSER_STATE_GET_DATA )
    {
        if( parser->data_size != 0 )
        {
            parser->data[ parser->bytes_read ] = byte;
            parser->bytes_read += 1;

            if( parser->bytes_read == parser->data_size )
            {
                parser->state = HOBD_PARSER_STATE_GET_CHECKSUM;
            }
        }
    }
    else if( parser->state == HOBD_PARSER_STATE_GET_CHECKSUM )
    {
#warning "TODO - checksum calculation and state recovery"

        parser->state = HOBD_PARSER_STATE_GET_TYPE;
    }

    return ret;
}
