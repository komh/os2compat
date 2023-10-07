/*
 * mmap( MAP_SHARED ) test program
 *
 * Copyright (C) 2014-2023 KO Myung-Hun <komh@chollian.net>
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

#include "test.h"

#include "mmap.h"

#define MMAP_FILENAME   "mmap-2.test"
#define MMAP_MSG        "This is a mmap() MAP_SHARED test."

#define EXIT(c) do { rc = ( c ); goto exit_close; } while( 0 )

int child( void )
{
    const char *name = MMAP_FILENAME;
    const char *msg = MMAP_MSG;

    const int pagesize = getpagesize();

    int  fd;
    char *p;

    int rc = 0;

    fd = open( name, O_RDWR | O_BINARY );
    if( fd == -1 )
    {
        fprintf( stderr, "CHILD: open() failed!!!\n");

        return 1;
    }

    p = mmap( NULL, pagesize * 2, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
              pagesize );
    if( p == MAP_FAILED )
    {
        fprintf( stderr, "CHILD: mmap() failed\n");

        EXIT( 1 );
    }

    strcpy( p, msg );

    if( munmap( p, pagesize * 2 ) == -1 )
    {
        fprintf( stderr, "CHILD: munmap() failed\n");

        EXIT( 1 );
    }

exit_close:

    close( fd );

    return rc;
}

int main( int argc, char *argv[])
{
    const char *name = MMAP_FILENAME;
    const char *msg = MMAP_MSG;

    const int pagesize = getpagesize();

    int  fd;
    char *p;
    int  len = strlen( msg ) + 1;

    int rc = 0;

    if( argc > 1 )
        return child();

    printf("Testing mmap( MAP_SHARED )...\n");

    fd = open( name, O_RDWR | O_CREAT | O_BINARY, S_IREAD | S_IWRITE );

    if( fd == -1 )
    {
        fprintf( stderr, "open() failed!!!\n");

        return 1;
    }

    if( chsize( fd, pagesize * 3 ) == -1 )
    {
        fprintf( stderr, "chsize() failed!!!\n");

        EXIT( 1 );
    }

    p = mmap( NULL, pagesize * 2, PROT_READ, MAP_SHARED, fd, 0 );
    if( p == MAP_FAILED )
    {
        fprintf( stderr, "mmap() failed\n");

        EXIT( 1 );
    }

    if( spawnlp( P_WAIT, argv[ 0 ], argv[ 0 ], argv[ 0 ], NULL ) != 0 )
    {
        fprintf( stderr, "spawnlp() failed\n");

        munmap( p, len );

        EXIT( 1 );
    }

    TEST_EQUAL_MSG( memcmp( msg, p + pagesize, len ), 0,
                    "Compare with child");

    if( munmap( p, pagesize * 2 ) == -1 )
    {
        fprintf( stderr, "munmap() failed\n");

        EXIT( 1 );
    }

    p = calloc( 1, len );

    lseek( fd, pagesize, SEEK_SET );
    read( fd, p, len );

    TEST_EQUAL_MSG( memcmp( msg, p, len ), 0, "Compare with file");

    free( p );

    printf("All tests PASSED\n");

exit_close:

    close( fd );

    remove( name );

    return rc;
}
