/**
 * @file main.c
 * @brief TODO.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "sd_protocol.h"


#define ERR_NONE (0)
#define ERR_FREAD (1)


typedef struct
{
    unsigned long iter;
    unsigned long valid_msg_cnt;
    unsigned long invalid_msg_cnt;
    unsigned long out_of_seq_cnt;
//    unsigned long missing_id_cnt_array[6];
} stats_s;


static int read_header(
        FILE * const stream,
        sd_msg_header_s * const header )
{
    int ret = ERR_NONE;

    const int status = fread(
            (void*) header,
            sizeof(*header),
            1,
            stream);

    if(status != 1)
    {
        ret = ERR_FREAD;
    }

    return ret;
}


static int read_payload(
        FILE * const stream,
        const uint8_t size,
        uint8_t * const buffer )
{
    int ret = ERR_NONE;

    const int status = fread(
            (void*) buffer,
            sizeof(*buffer),
            (size_t) size,
            stream);

    if(status != (int) size)
    {
        ret = ERR_FREAD;
    }

    return ret;
}


static void print_header(
        const sd_msg_header_s * const header )
{
    printf("preamble '0x%02X'\n", (unsigned int) header->preamble);
    printf("type '0x%02X'\n", (unsigned int) header->type);
    printf("size '%u'\n", (unsigned int) header->size);
}


static void print_payload(
        const sd_msg_s * const msg )
{
    if(msg->header.type == SD_MSG_TYPE_INVALID)
    {
        printf("  INVALID TYPE\n");
    }
    else if(msg->header.type == SD_MSG_TYPE_FILE_START)
    {
        printf("  FILE START - 0x%02X\n",
                (unsigned int) msg->data[0]);
    }
    else if(msg->header.type == SD_MSG_TYPE_CAN_FRAME)
    {
        printf("  CAN FRAME\n");

        const sd_can_frame_s * const can_frame = (const sd_can_frame_s*) msg->data;

        printf("    rx_timestamp '%lu'\n",
                (unsigned long) can_frame->rx_timestamp);
        printf("    id '0x%04X (%u)'\n",
                (unsigned int) can_frame->id, (unsigned int) can_frame->id);
        printf("    dlc '%u'\n",
                (unsigned int) can_frame->dlc);

        if(can_frame->dlc != 0)
        {
            printf("    ");

            uint8_t idx;
            for(idx = 0; idx < can_frame->dlc; idx += 1)
            {
                printf("%02X ",
                        (unsigned int) can_frame->data[can_frame->dlc - idx - 1]);
            }

            printf("\n");
        }
    }
    else
    {
        printf("  UNKNOWN TYPE\n");
    }
}


static void print_stats( const stats_s * const stats )
{
    printf("\n");
    printf("stats\n");

    printf("iterations '%lu'\n", stats->iter);
    printf("valid messages '%lu'\n", stats->valid_msg_cnt);
    printf("invalid messages '%lu'\n", stats->invalid_msg_cnt);
    printf("out-of-sequence messages '%lu'\n", stats->out_of_seq_cnt);

    printf("\n");
}


int main( int argc, char **argv )
{
    stats_s stats;
    char file_path[512];

    (void) memset(&stats, 0, sizeof(stats));

    if((argc > 1) && (strlen(argv[1]) > 0))
    {
        (void) strncpy(file_path, argv[1], sizeof(file_path));
    }
    else
    {
        (void) strncpy(file_path, "data.log", sizeof(file_path));
    }

    FILE * const file = fopen(file_path, "rb");
    if(file == NULL)
    {
        printf("error - fopen failed '%s'\n", file_path);
        return EXIT_FAILURE;
    }

    unsigned int do_loop = 1;
    while(do_loop != 0)
    {
        uint8_t msg_buffer[SD_MSG_SIZE_MAX];

        (void) memset(msg_buffer, 0, sizeof(msg_buffer));

        sd_msg_s * const msg = (sd_msg_s*) msg_buffer;

        const int header_status = read_header(
                file,
                &msg->header);
        if(header_status != ERR_NONE)
        {
            printf("error - read_header failed '%d'\n", header_status);
            do_loop = 0;

            if(feof(file) == 0)
            {
                stats.invalid_msg_cnt += 1;
            }
        }
        else
        {
            if(msg->header.preamble != SD_MSG_PREAMBLE)
            {
                printf("error - invalid preamble 0x%02X\n",
                        (unsigned int) msg->header.preamble);
                do_loop = 0;
                stats.invalid_msg_cnt += 1;
            }

            print_header(&msg->header);
        }

        if((header_status == ERR_NONE) && (do_loop != 0))
        {
            const int payload_status = read_payload(
                    file,
                    msg->header.size,
                    msg->data);
            if(payload_status != ERR_NONE)
            {
                printf("error - read_payload failed '%d'\n", payload_status);
                do_loop = 0;
                stats.invalid_msg_cnt += 1;
            }
            else
            {
                print_payload(msg);
                stats.valid_msg_cnt += 1;
            }
        }

        printf("\n");

        if(feof(file) != 0)
        {
            printf("\n*** EOF ***\n\n");
            do_loop = 0;
        }

        if(stats.iter == 0)
        {
            const uint32_t expected = SD_FILE_START_WORD;
    
            if(memcmp(&expected, msg_buffer, sizeof(expected)) != 0)
            {
                printf("error - missing expect start-of-file\n");
                do_loop = 0;
                stats.invalid_msg_cnt += 1;
            }
        }

        stats.iter += 1;
    }

    if(file != NULL)
    {
        (void) fclose(file);
    }

    print_stats(&stats);

    return EXIT_SUCCESS;
}