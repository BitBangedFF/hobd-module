/**
 * @file sd.c
 * @brief TODO.
 *
 */


#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <stdint.h>

#include "board.h"
#include "spi_drv.h"
#include "spi_lib.h"

#include "fat_filelib.h"

#include "error.h"
#include "debug.h"
#include "sd_cmd.h"
#include "sd_protocol.h"
#include "sd.h"


#define SD_MSK_SPI_CONFIG_SLOW (SPI_MSB_FIRST | SPI_MASTER | SPI_DATA_MODE_0 | ((1<<SPR1)|(1<<SPR0)))
#define SD_MSK_SPI_CONFIG_FAST (SPI_MSB_FIRST | SPI_MASTER | SPI_DATA_MODE_0 | ((0<<SPR1)|(0<<SPR0)))

#define SD_RAW_SPEC_1 (0)
#define SD_RAW_SPEC_2 (1)
#define SD_RAW_SPEC_SDHC (2)


static const char FILE_NAME[] = "/data.log";


static BOOL is_init = FALSE;
static FL_FILE *file = NULL;


static uint8_t send_command(
        const uint8_t cmd,
        const uint32_t data )
{
    uint8_t resp;

    // wait some clock cycles
    (void) spi_getchar();

    (void) spi_putchar(0x40 | cmd);
    (void) spi_putchar((data >> 24) & 0xFF);
    (void) spi_putchar((data >> 16) & 0xFF);
    (void) spi_putchar((data >> 8) & 0xFF);
    (void) spi_putchar((data >> 0) & 0xFF);

    if(cmd == CMD_GO_IDLE_STATE)
    {
        (void) spi_putchar(0x95);
    }
    else if(cmd == CMD_SEND_IF_COND)
    {
        (void) spi_putchar(0x87);
    }
    else
    {
        (void) spi_putchar(0xFF);
    }

    uint8_t idx;
    for(idx = 0, resp = 0xFF; (idx < 10) && (resp == 0xFF); idx += 1)
    {
        resp = spi_getchar();
    }

    return resp;
}


static uint8_t sd_read_block(
        const uint32_t sector,
        uint8_t * const buffer )
{
    uint8_t ret = ERR_OK;
    uint8_t resp;

    spi_enable_ss();

    resp = send_command(CMD_READ_SINGLE_BLOCK, sector);

    if(resp != 0)
    {
        spi_disable_ss();
        ret = ERR_SD_INIT;
    }

    if(ret == ERR_OK)
    {
        // wait for data block (start byte 0xFE)
        while(spi_getchar() != 0xFE);
    }

    if(ret == ERR_OK)
    {
        uint16_t idx;
        for(idx = 0; idx < FAT_SECTOR_SIZE; idx += 1)
        {
            buffer[idx] = spi_getchar();
        }

        // read crc16
        (void) spi_getchar();
        (void) spi_getchar();
    }

    spi_disable_ss();
    (void) spi_getchar();

    return ret;
}


static uint8_t sd_write_block(
        const uint32_t sector,
        const uint8_t * const buffer )
{
    uint8_t ret = ERR_OK;
    uint8_t resp;

    spi_enable_ss();

    resp = send_command(CMD_WRITE_SINGLE_BLOCK, sector);

    if(resp != 0)
    {
        spi_disable_ss();
        ret = ERR_SD_INIT;
    }

    if(ret == ERR_OK)
    {
        // send start byte
        (void) spi_putchar(0xFE);

        uint16_t idx;
        for(idx = 0; idx < FAT_SECTOR_SIZE; idx += 1)
        {
            (void) spi_putchar(buffer[idx]);
        }

        // write dummy crc16
        (void) spi_putchar(0xFF);
        (void) spi_putchar(0xFF);

        // wait while card is busy
        while(spi_getchar() != 0xFF);
    }

    spi_disable_ss();
    (void) spi_getchar();

    return ret;
}


// FAT lib read callback
static int fat_read(
        uint32_t sector,
        uint8_t *buffer,
        uint32_t sector_count )
{
    uint8_t status = ERR_OK;

    uint32_t idx;
    for(idx = 0; (idx < sector_count) && (status == ERR_OK); idx += 1)
    {
        status = sd_read_block(sector, buffer);

        sector += 1;
        buffer += FAT_SECTOR_SIZE;
    }

    return (int) (status == ERR_OK);
}


// FAT lib write callback
static int fat_write(
        uint32_t sector,
        uint8_t *buffer,
        uint32_t sector_count )
{
    uint8_t status = ERR_OK;

    uint32_t idx;
    for(idx = 0; (idx < sector_count) && (status == ERR_OK); idx += 1)
    {
        status = sd_write_block(sector, buffer);

        sector += 1;
        buffer += FAT_SECTOR_SIZE;
    }

    return (int) (status == ERR_OK);
}


void sd_init( void )
{
    if(is_init == FALSE)
    {
        uint8_t status = ERR_OK;
        uint8_t card_type = 0;
        uint16_t idx;

        file = NULL;

        spi_init_ss();
        spi_disable_ss();

        // initialize SPI with lowest frequency
        // max of 400 kHz during identification mode of card
        Spi_init_bus( ((uint8_t) (SD_MSK_SPI_CONFIG_SLOW & MSK_SPI_MASTER)));
        SPCR = SD_MSK_SPI_CONFIG_SLOW;
        SPSR &= ~(1 << SPI2X);
        spi_enable();
        spi_disable_ss();
        wdt_reset();

        // card needs 74 cycles minimum to start up
        for(idx = 0; idx < 10; idx += 1)
        {
            (void) spi_getchar();
        }

        spi_enable_ss();
        wdt_reset();

        // reset the card
        uint8_t resp = 0x00;
        for(idx = 0; resp != (1 << R1_IDLE_STATE); idx += 1)
        {
            resp = send_command(CMD_GO_IDLE_STATE, 0);

            if(idx == 0x01FF)
            {
                spi_disable_ss();
                status = ERR_SD_INIT;
            }
        }

        wdt_reset();

        if(status == ERR_OK)
        {
            // check for version of SD card specification
            resp = send_command(CMD_SEND_IF_COND, 0x100 | 0xAA);

            if((resp & (1 << R1_ILL_COMMAND)) == 0)
            {
                (void) spi_getchar();
                (void) spi_getchar();

                if((spi_getchar() & 0x01) == 0)
                {
                    spi_disable_ss();
                    status = ERR_SD_INIT;
                }

                if(status == ERR_OK)
                {
                    if(spi_getchar() != 0xAA)
                    {
                        spi_disable_ss();
                        status = ERR_SD_INIT;
                    }
                }

                if(status == ERR_OK)
                {
                    card_type |= (1 << SD_RAW_SPEC_2);
                }
            }
            else
            {
                spi_disable_ss();
                status = ERR_SD_INIT;
            }
        }

        wdt_reset();

        if(status == ERR_OK)
        {
            // wait for card to get ready
            for(idx = 0, resp = 0x00; status == ERR_OK; idx += 1)
            {
                (void) send_command(CMD_APP, 0);
                resp = send_command(CMD_SD_SEND_OP_COND, 0x40000000);

                if((resp & (1 << R1_IDLE_STATE)) == 0)
                {
                    break;
                }

                if(idx == 0x7FFF)
                {
                    spi_disable_ss();
                    status = ERR_SD_INIT;
                }
            }
        }

        if(status == ERR_OK)
        {
            if((card_type & (1 << SD_RAW_SPEC_2)) != 0)
            {
                if(send_command(CMD_READ_OCR, 0) != 0)
                {
                    spi_disable_ss();
                    status = ERR_SD_INIT;
                }
                else
                {
                    if((spi_getchar() & 0x40) != 0)
                    {
                        card_type |= (1 << SD_RAW_SPEC_SDHC);
                    }

                    (void) spi_getchar();
                    (void) spi_getchar();
                    (void) spi_getchar();
                }
            }
        }

        // set block size to 512 bytes
        if(status == ERR_OK)
        {
            if(send_command(CMD_SET_BLOCKLEN, 512) != 0)
            {
                spi_disable_ss();
                status = ERR_SD_INIT;
            }
        }

        // switch to highest SPI frequency possible
        if(status == ERR_OK)
        {
            spi_disable_ss();
            SPCR = SD_MSK_SPI_CONFIG_FAST;
            SPSR |= (1 << SPI2X);
            spi_enable();
        }

        spi_disable_ss();
        wdt_reset();

        if(status == ERR_OK)
        {
            fl_init();

            if(fl_attach_media(fat_read, fat_write) != FAT_INIT_OK)
            {
                DEBUG_PUTS(PSTR("failed to attach FAT IO\n"));
                status = ERR_SD_FS;
                fl_shutdown();
            }
            else
            {
                is_init = TRUE;
            }
        }
        else
        {
            DEBUG_PUTS(PSTR("failed to init SD card\n"));
        }
    }
}


void sd_open( void )
{
    if(is_init == TRUE)
    {
        file = fl_fopen(FILE_NAME, "ab");

        if(file == NULL)
        {
            DEBUG_PUTS(PSTR("failed to create log file\n"));
        }
        else
        {
            const uint32_t file_start_word = SD_FILE_START_WORD;

            sd_write(
                    (const uint8_t*) &file_start_word,
                    sizeof(file_start_word),
                    1);
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
                DEBUG_PUTS(PSTR("failed to write SD data\n"));
            }
        }
    }
}


void sd_puts(
        const char * const data )
{
    if(is_init == TRUE)
    {
        if(file != NULL)
        {
            const int bytes_written = fl_fputs(data, file);

#warning "TODO - handle returns"
            if( bytes_written <= 0 )
            {
                DEBUG_PUTS(PSTR("failed to write SD data\n"));
            }
        }
    }
}
