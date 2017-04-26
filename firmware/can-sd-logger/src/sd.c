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
#include "sd_cmd.h"
#include "sd.h"


#define SD_MSK_SPI_CONFIG_SLOW (SPI_MSB_FIRST | SPI_MASTER | SPI_DATA_MODE_0 | SPI_CLKIO_BY_128)
#define SD_MSK_SPI_CONFIG_FAST (SPI_MSB_FIRST | SPI_MASTER | SPI_DATA_MODE_0 | SPI_CLKIO_BY_2)

#define SD_RAW_SPEC_1 (0)
#define SD_RAW_SPEC_2 (1)
#define SD_RAW_SPEC_SDHC (2)

static const char FILE_NAME[] = "data.log";


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
        uint8_t status = ERR_OK;
        uint8_t card_type = 0;
        uint16_t idx;

        file = NULL;

        spi_init_ss();
        spi_disable_ss();

        // initialize SPI with lowest frequency
        // max of 400 kHz during identification mode of card
        (void) spi_init(SD_MSK_SPI_CONFIG_SLOW);
        spi_disable_ss();

        // card needs 74 cycles minimum to start up
        for(idx = 0; idx < 10; idx += 1)
        {
            (void) spi_getchar();
        }

        spi_enable_ss();

        // reset the card
        uint8_t resp = 0x00;
        for(idx = 0; resp != (1 << R1_IDLE_STATE); idx += 1)
        {
            resp = send_command(CMD_GO_IDLE_STATE, 0);

            if(idx == 0x01FF)
            {
                spi_disable_ss();
                status = ERR_SD_INIT;
                DEBUG_PUTS("failed to reset SD card\n");
            }
        }

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
                    DEBUG_PUTS("SD card operation voltage range doesn't match\n");
                }

                if(status == ERR_OK)
                {
                    if(spi_getchar() != 0xAA)
                    {
                        spi_disable_ss();
                        status = ERR_SD_INIT;
                        DEBUG_PUTS("SD card returned invalid pattern\n");
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
                DEBUG_PUTS("unsupported SD card type\n");
            }
        }

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
                    DEBUG_PUTS("failed to wait for SD card\n");
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
                    DEBUG_PUTS("failed to send SD OCR command\n");
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
                DEBUG_PUTS("failed to set block size\n");
            }
        }

        // switch to highest SPI frequency possible
        if(status == ERR_OK)
        {
            spi_disable_ss();
            (void) spi_init(SD_MSK_SPI_CONFIG_FAST);
            spi_disable_ss();
        }

        if(status == ERR_OK)
        {
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
