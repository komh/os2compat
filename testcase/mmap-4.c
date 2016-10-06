/*
 * mmap( MAP_FIXED ) test program for already allocated case
 *
 * Copyright (C) 2016 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#include <stdio.h>
#include <stdlib.h>

#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mmap.h"

#define EXIT(c, label) do { rc = (c); goto exit_##label; } while( 0 )

#define TESTFILE "mmap-4.test"

int main( void )
{
    char *data;
    char *p1;
    char *p2;
    int pagesize;
    int i;
    int fd;

    int rc = 0;

    pagesize = getpagesize();

    data = malloc( pagesize );
    if( !data )
        return 1;

    for( i = 0; i < pagesize; i++ )
        data[ i ] = i * i;

    fd = open(TESTFILE, O_CREAT | O_RDWR, S_IREAD | S_IWRITE );
    if( fd == -1 )
        EXIT( 1, free );

    if( write( fd, data, pagesize ) != pagesize )
        EXIT( 1, close );

    p1 = mmap( NULL, pagesize, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED,
               -1, 0 );
    if( p1 == MAP_FAILED )
    {
        fprintf( stderr, "mmap(p1) failed\n");

        EXIT( 1, close );
    }

    p2 = mmap( p1, pagesize, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_PRIVATE,
               fd, 0 );
    if( p2 == MAP_FAILED )
    {
        fprintf( stderr, "mmap(p2) failed\n");

        EXIT( 1, munmap );
    }

    printf("p1 = %p, p2 = %p\n", p1, p2 );

    for( i = 0; i < pagesize; i++ )
    {
        if( data[ i ] != p1[ i ])
        {
            fprintf(stderr, "Mismatched at %d offset\n", i );

            EXIT( 1, munmap );
        }
    }

    printf("PASSED!!!\n");

exit_munmap:
    if( munmap( p1, pagesize ))
    {
        fprintf( stderr, "munmap(p1) failed\n");

        rc = 1;
    }

    if( munmap( p2, pagesize ))
    {
        fprintf( stderr, "munmap(p2) failed\n");

        rc = 1;
    }

exit_close:
    close( fd );

    remove( TESTFILE );

exit_free:
    free( data );

    return rc;
}
