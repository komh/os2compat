/*
 * mmap( MAP_ANON | MAP_SHARED ) test program
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
#include <unistd.h>

#include "mmap.h"

#define EXIT(c) do { rc = (c); goto exit_munmap; } while( 0 )

int main( void )
{
    int *p1;
    int *p2;
    int  len = sizeof( *p1 );
    int  pid;
    const char *me;

    int rc = 0;

    p1 = mmap( NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1,
               0 );

    if( p1 == MAP_FAILED )
    {
        fprintf( stderr, "mmap() failed\n");

        return 1;
    }

    p2 = mmap( NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1,
               0 );

    if( p2 == MAP_FAILED )
    {
        fprintf( stderr, "mmap() failed\n");

        EXIT( 1 );
    }

    pid = fork();
    switch( pid )
    {
        case -1 :
            fprintf( stderr, "fork() failed\n");

            EXIT( 1 );
            break;

        case 0 :
            me = "CHILD ";
            *p2 = 20;
            printf("%s: Set *p2 to %d\n", me, *p2 );
            break;

        default :
            me = "PARENT";
            *p1 = 10;
            printf("%s: Set *p1 to %d\n", me, *p1 );
            break;
    }

    /* @todo synchornization */

    printf("%s: p1 = %p, *p1 = %d, p2 = %p, *p2 = %d\n",
           me, p1, *p1, p2, *p2 );

exit_munmap:

    if( munmap( p1, len ))
    {
        fprintf( stderr, "%s: munmap() failed\n", me );

        rc = 1;
    }

    if( munmap( p2, len ))
    {
        fprintf( stderr, "%s: munmap() failed\n", me);

        rc = 1;
    }

    return rc;
}
