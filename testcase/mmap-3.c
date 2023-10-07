/*
 * mmap( MAP_ANON | MAP_SHARED ) test program
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
#include <unistd.h>

#include <sys/wait.h>

#include "test.h"

#include "mmap.h"

#define P1  10
#define P2  20

#define EXIT(c) do { rc = (c); goto exit_munmap; } while( 0 )

int main( void )
{
    int *p1;
    int *p2;
    int  len = sizeof( *p1 );
    int  p2c[ 2 ];
    int  c2p[ 2 ];
    int  sig;
    int  pid;
    const char *me;
    int status;

    int rc = 0;

    printf("Testing mmap( MAP_ANON | MAP_SHARED )...\n");

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

    if( pipe( p2c ) == -1 )
    {
        fprintf( stderr, "pipe() failed\n");

        EXIT( 1 );
    }

    if( pipe( c2p ) == -1 )
    {
        fprintf( stderr, "pipe() failed\n");

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
            close( p2c[ 1 ]);
            close( c2p[ 0 ]);

            me = "CHILD ";
            printf("%s: *p2 = %d\n", me, *p2 );
            *p2 = P2;
            printf("%s: Set *p2 to %d\n", me, *p2 );

            read( p2c[ 0 ], &sig, sizeof( sig ));
            write( c2p[ 1 ], &sig, sizeof( sig ));

            close( p2c[ 0 ]);
            close( c2p[ 1 ]);
            break;

        default :
            close( p2c[ 0 ]);
            close( c2p[ 1 ]);

            me = "PARENT";
            printf("%s: *p1 = %d\n", me, *p1 );
            *p1 = P1;
            printf("%s: Set *p1 to %d\n", me, *p1 );

            write( p2c[ 1 ], &sig, sizeof( sig ));
            read( c2p[ 0 ], &sig, sizeof( sig ));

            close( p2c[ 1 ]);
            close( c2p[ 0 ]);
            break;
    }

    TEST_EQUAL_MSG( *p1, P1, me );
    TEST_EQUAL_MSG( *p2, P2, me );

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

    if( rc == 0 && pid != 0 && waitpid( pid, &status, 0 ) == pid
        && WIFEXITED( status ) && WEXITSTATUS( status ) == 0 )
        printf("All tests PASSED\n");

    return rc;
}
