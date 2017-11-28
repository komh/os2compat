/*
 * mmap( MAP_PRIVATE ) and fork() test program
 *
 * Copyright (C) 2017 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#include "mmap.h"

#define EXIT(c) do { rc = (c); goto exit_munmap; } while( 0 )

int main( void )
{
    int *p1;
    int *p2;
    int *p3;
    int *p4;
    int  len = sizeof( *p1 );
    int  pid;
    const char *me;

    int rc = 0;

    p1 = mmap( NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1,
               0 );

    if( p1 == MAP_FAILED )
    {
        fprintf( stderr, "mmap( p1 ) failed\n");

        return 1;
    }

    p2 = mmap( NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1,
               0 );

    if( p2 == MAP_FAILED )
    {
        fprintf( stderr, "mmap( p2 ) failed\n");

        EXIT( 1 );
    }

    if( munmap( p2, len ) == -1 )
    {
        fprintf( stderr, "munmap( p2 ) failed\n");

        EXIT( 1 );
    }

    p2 = mmap( p2, len, PROT_READ | PROT_WRITE,
               MAP_FIXED | MAP_PRIVATE | MAP_ANON, -1, 0 );

    if( p2 == MAP_FAILED )
    {
        fprintf( stderr, "mmap( p2, MAP_FIXED ) failed\n");

        EXIT( 1 );
    }

    p3 = mmap( NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1,
               0 );

    if( p3 == MAP_FAILED )
    {
        fprintf( stderr, "mmap( p3 ) failed\n");

        EXIT( 1 );
    }

    p4 = mmap( NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1,
               0 );

    if( p4 == MAP_FAILED )
    {
        fprintf( stderr, "mmap( p3 ) failed\n");

        EXIT( 1 );
    }

    *p1 = 10;
    printf("ALL: Set *p1 to %d\n", *p1 );

    *p2 = 20;
    printf("ALL: Set *p2 to %d\n", *p2 );

    pid = fork();
    switch( pid )
    {
        case -1 :
            fprintf( stderr, "fork() failed\n");

            EXIT( 1 );
            break;

        case 0 :
            me = "CHILD ";
            *p1 = 30;
            printf("%s: Set *p1 to %d\n", me, *p1 );
            *p3 = *p1;

            *p2 = 40;
            printf("%s: Set *p2 to %d\n", me, *p2 );
            *p4 = *p2;

            break;

        default :
            waitpid( pid, NULL, 0 );
            me = "PARENT";
            printf("%s: *p1 = %d, *p2 = %d, *p3 = %d, *p4 = %d\n",
                   me, *p1, *p2, *p3, *p4 );
            if( *p1 == *p3 || *p2 == *p4 )
                printf("FAILED.\n");
            else
                printf("PASSED.\n");
            break;
    }

exit_munmap:

    if( munmap( p1, len ))
    {
        fprintf( stderr, "%s: munmap( p1 ) failed\n", me );

        rc = 1;
    }

    if( munmap( p2, len ))
    {
        fprintf( stderr, "%s: munmap( p2 ) failed\n", me);

        rc = 1;
    }

    if( munmap( p3, len ))
    {
        fprintf( stderr, "%s: munmap( p3 ) failed\n", me);

        rc = 1;
    }

    if( munmap( p4, len ))
    {
        fprintf( stderr, "%s: munmap( p4 ) failed\n", me);

        rc = 1;
    }

    return rc;
}
