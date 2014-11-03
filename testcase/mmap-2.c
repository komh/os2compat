/*
 * mmap( MAP_SHARED ) test program
 *
 * Copyright (C) 2014 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <process.h>

#include "mmap.h"
#include "mmap-2.h"

#define EXIT(c) do { rc = ( c ); goto exit_close; } while( 0 )

int main( void )
{
    const char *name = MMAP_FILENAME;
    const char *msg = MMAP_MSG;
    const char *child = "mmap-2-child.exe";

    const int pagesize = getpagesize();

    int  fd;
    char *p;
    int  len = strlen( msg ) + 1;

    int rc = 0;

    fd = open( name, O_RDWR | O_CREAT | O_SIZE | O_BINARY, S_IREAD | S_IWRITE,
               pagesize * 3 );

    if( fd == -1 )
    {
        fprintf( stderr, "open() failed!!!\n");

        return 1;
    }

    p = mmap( NULL, pagesize * 2, PROT_READ, MAP_SHARED, fd, 0 );
    if( p == MAP_FAILED )
    {
        fprintf( stderr, "mmap() failed\n");

        EXIT( 1 );
    }

    printf("Spwaning a child...\n");
    if( spawnlp( P_WAIT, child, child, NULL ) != 0 )
    {
        fprintf( stderr, "spawnlp() failed\n");

        munmap( p, len );

        EXIT( 1 );
    }

    printf("Comparing a mmaped memory... ");

    if( memcmp( msg, p + pagesize, len ))
        printf("FAILED\n");
    else
        printf("PASSED\n");

    if( munmap( p, pagesize * 2 ) == -1 )
    {
        fprintf( stderr, "munmap() failed\n");

        EXIT( 1 );
    }

    printf("Reading from a file...\n");

    p = calloc( 1, len );

    lseek( fd, pagesize, SEEK_SET );
    read( fd, p, len );

    printf("Comparing contents... ");

    if( !memcmp( msg, p, len ))
        printf("PASSED\n");
    else
        printf("FAILED\n");

    free( p );

exit_close:

    close( fd );

    remove( name );

    return rc;
}
