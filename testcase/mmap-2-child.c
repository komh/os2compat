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

#include "mmap.h"
#include "mmap-2.h"

#define EXIT(c) do { rc = ( c ); goto exit_close; } while( 0 )

int main( void )
{
    const char *name = MMAP_FILENAME;
    const char *msg = MMAP_MSG;

    const int pagesize = getpagesize();

    int  fd;
    char *p;

    int rc = 0;

    printf("CHILD process started.\n");

    fd = open( name, O_RDWR | O_BINARY );
    if( fd == -1 )
    {
        fprintf( stderr, "open() failed!!!\n");

        return 1;
    }

    printf("Writing to a mmaped memory...\n");

    p = mmap( NULL, pagesize * 2, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
              pagesize );
    if( p == MAP_FAILED )
    {
        fprintf( stderr, "mmap() failed\n");

        EXIT( 1 );
    }

    strcpy( p, msg );

    if( munmap( p, pagesize * 2 ) == -1 )
    {
        fprintf( stderr, "munmap() failed\n");

        EXIT( 1 );
    }

exit_close:

    close( fd );

    printf("CHILD process ended.\n");

    return rc;
}
