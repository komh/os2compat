/*
 * spawn2ve() and spawn2vpe() test program
 *
 * Copyright (C) 2024 KO Myung-Hun <komh78@gmail.com>
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

#include "test.h"

#include "spawn2.h"

#define PASS_STR "!aCKNowLeDGeMenNT!"

static inline int set_cloexec( int fd )
{
    int fl = fcntl( fd, F_GETFD );

    return fl == -1 ? -1 : fcntl( fd, F_SETFD, fl | FD_CLOEXEC );
}

static int child( const char *rfdstr, const char *wfdstr,
                  const char *noinheritstr )
{
    int noinherit = atoi( noinheritstr );

    FILE *rfp = fdopen( atoi( rfdstr ), "rt" );
    FILE *wfp = fdopen( atoi( wfdstr ), "wt" );

    TEST_EQ_MSG( rfp, NULL, "fdopen(rfdstr)" );
    if( noinherit )
        TEST_EQ_MSG( wfp, NULL, "fdopen(wfdstr)" );
    else
    {
        TEST_NE_MSG( wfp, NULL, "fdopen(wfdstr)" );

        fprintf( wfp, "%s", PASS_STR );
    }

    if( wfp )
        fclose( wfp );

    if( rfp )
        fclose( rfp );

    return 0;
}

void test( const char *me, const char *msg, int flags )
{
    char str[ 64 ];
    const char *args[] = { me, "--child", str, str + 16, str + 32, NULL };
    int stdfds[] = { 0, 2, 1 }; /* activate redirection */
    int pipefds[ 2 ];
    int pid;
    int passlen = strlen( PASS_STR );

    TEST_START( msg );

    TEST_EQ( pipe( pipefds ), 0 );

    set_cloexec( pipefds[ 0 ]);
    _itoa( pipefds[ 0 ], str, 10 );
    _itoa( pipefds[ 1 ], str + 16, 10 );
    _itoa( flags & P_2_NOINHERIT ? 1 : 0, str + 32, 10 );

    pid = spawn2ve( P_NOWAIT | flags, me, args, NULL, NULL, stdfds );
    TEST_NE_MSG( pid, -1, "spawn2ve()" );

    close( pipefds[ 1 ]);

    if( flags & P_2_NOINHERIT )
        TEST_EQ( read( pipefds[ 0 ], str, sizeof( str )), 0 );
    else
    {
        TEST_EQ( read( pipefds[ 0 ], str, sizeof( str )), passlen );
        TEST_EQ( memcmp( str, PASS_STR, passlen ), 0 );
    }

    TEST_EQ( waitpid( pid, NULL, 0 ), pid );

    close( pipefds[ 0 ]);

    TEST_END();
}

int main( int argc, char *argv[])
{
    const char *me = argv[ 0 ];

    if( argc > 4 && strcmp( argv[ 1 ], "--child" ) == 0 )
        return child( argv[ 2 ], argv[ 3 ], argv[ 4 ]);

    test( me,
          "if spawn2ve(NO-THREADSAFE) passes open handles not having "
          "FD_CLOEXEC", 0 );

    test( me,
          "if spawn2ve(THREADSAFE) passes open handles not having "
          "FD_CLOEXEC", P_2_THREADSAFE );

    test( me, "if spawn2ve(NO-THREADSAFE | NO-INHERIT) passes no handles",
          P_2_NOINHERIT );

    test( me, "if spawn2ve(THREADSAFE | NO-INHERIT) passes no handles",
          P_2_THREADSAFE | P_2_NOINHERIT );

    TEST_ALL_PASSED();

    return 0;
}
