/**
 * @file sd.c
 * @brief TODO.
 *
 */


#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <stdint.h>

#include "board.h"
#include "spi_drv.h"
#include "spi_lib.h"

#include "fat_filelib.h"

#include "error.h"
#include "debug.h"
#include "sd.h"


static const char FILE_NAME[] = "data.log";


static BOOL is_init = FALSE;
static FL_FILE *file = NULL;


// FAT lib read callback
static int fat_read(
        uint32_t sector,
        uint8_t *buffer,
        uint32_t sector_count )
{
    return 0;
}


// FAT lib write callback
static int fat_write(
        uint32_t sector,
        uint8_t *buffer,
        uint32_t sector_count )
{
    return 0;
}


void sd_init( void )
{
    if(is_init == FALSE)
    {
        file = NULL;
        fl_init();

        if(fl_attach_media(fat_read, fat_write) != FAT_INIT_OK)
        {
            DEBUG_PUTS("failed to attach FAT IO\n");
            fl_shutdown();
        }
        else
        {
            is_init = TRUE;
        }
    }
}


void sd_open( void )
{
    if(is_init == TRUE)
    {
        file = fl_fopen(FILE_NAME, "wb");

        if(file == NULL)
        {
            DEBUG_PUTS("failed to create log file\n");
        }
    }
}


void sd_close( void )
{
    if(file != NULL)
    {
        (void) fl_fflush(file);
        fl_fclose(file);

        file = NULL;
    }
}


void sd_flush( void )
{
    if(file != NULL)
    {
        (void) fl_fflush(file);
    }
}


void sd_write(
        const uint8_t * const data,
        const uint16_t size,
        const uint16_t count )
{
    if(is_init == TRUE)
    {
        if(file != NULL)
        {
            const int bytes_written = fl_fwrite(
                    (const void*) data,
                    (const int) size,
                    (const int) count,
                    file);

#warning "TODO - handle returns"
            if( bytes_written <= 0 )
            {
                DEBUG_PUTS("failed to write SD data\n");
            }
        }
    }
}
