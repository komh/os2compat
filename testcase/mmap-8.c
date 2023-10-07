/*
 * mmap( MAP_PRIVATE ) and fork() test program
 *
 * Copyright (C) 2017-2023 KO Myung-Hun <komh@chollian.net>
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

#include "test.h"

#include "mmap.h"

#define P1  10
#define P2  20
#define P3  30
#define P4  40

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

    printf("Testing mmap( MAP_PRIVATE ) and fork()...\n");

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

    *p1 = P1;
    *p2 = P2;

    printf("ALL: *p1 = %d, *p2 = %d, *p3 = %d, *p4 = %d\n",
           *p1, *p2, *p3, *p4 );

    pid = fork();
    switch( pid )
    {
        case -1 :
            fprintf( stderr, "fork() failed\n");

            EXIT( 1 );
            break;

        case 0 :
            me = "CHILD ";
            *p3 = *p1 = P3;
            *p4 = *p2 = P4;
            printf("%s: Set *p1 = %d, *p2 = %d, *p3 = %d, *p4 = %d\n",
                   me, *p1, *p2, *p3, *p4 );
            break;

        default :
        {
            int status;

            me = "PARENT";

            if( waitpid( pid, &status, 0 ) != pid
                && !WIFEXITED( status ) && WEXITSTATUS( status ) != 0 )
            {
                fprintf( stderr, "%s : waitpid( pid ) failed!!!\n", me );

                EXIT( 1 );
            }

            TEST_EQUAL_MSG( *p1, P1, me );
            TEST_EQUAL_MSG( *p2, P2, me );
            TEST_EQUAL_MSG( *p3, P3, me );
            TEST_EQUAL_MSG( *p4, P4, me );
            break;
        }
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

    if( rc == 0 && pid != 0 )
        printf("All tests PASSED\n");

    return rc;
}
