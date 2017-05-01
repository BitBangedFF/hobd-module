/**
 * @file main.c
 * @brief TODO.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sd_protocol.h"

int main( int argc, char **argv )
{
    char file_path[512];

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

    //

    if(file != NULL)
    {
        (void) fclose(file);
    }

    return EXIT_SUCCESS;
}
