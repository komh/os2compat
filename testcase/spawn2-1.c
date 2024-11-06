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
#include <limits.h>
#include <errno.h>

#include "test.h"

#include "spawn2.h"

int main( void )
{
    const char *args[] = { "pwd", NULL, };
    char path[ PATH_MAX ];
    char *fullpath;
    char *relpath;
    int nullfd;
    int stdfds[] = { 0, 0, 0 };

    TEST_START( "a name only" );

    TEST_EQ( _path2( args[ 0 ], ".exe", path, sizeof( path )), 0 );

    nullfd = open( "/dev/null", O_WRONLY | O_NOINHERIT );
    TEST_NE_MSG( nullfd, -1, "open(/dev/null)" );

    /* make stdout and stderr silent */
    stdfds[ 1 ] = nullfd;
    stdfds[ 2 ] = nullfd;

    TEST_EQ( spawn2ve( P_WAIT, args[ 0 ], args, NULL, NULL, stdfds ), -1 );
    TEST_BOOL_MSG( errno == ENOENT, 1, "spawn2ve()" );

    TEST_EQ( spawn2vpe( P_WAIT, args[ 0 ], args, NULL, NULL, stdfds ), 0 );

    TEST_END();

    TEST_START( "a full path" );

    fullpath = realpath( path, NULL );
    TEST_NE_MSG( fullpath, NULL, "realpath" );

    args[ 0 ] = fullpath;

    TEST_EQ( spawn2ve( P_WAIT, args[ 0 ], args, NULL, NULL, stdfds ), 0 );
    TEST_EQ( spawn2vpe( P_WAIT, args[ 0 ], args, NULL, NULL, stdfds ), 0 );

    TEST_END();

    TEST_START( "cwd + a relative path" );

    relpath = strchr( fullpath + 4, '/' );
    *relpath++ = 0;
    args[ 0 ] = relpath;

    TEST_EQ( spawn2ve( P_WAIT, args[ 0 ], args, fullpath, NULL, stdfds ), 0 );

    TEST_EQ( spawn2vpe( P_WAIT, args[ 0 ], args, fullpath, NULL, stdfds ), 0 );

    TEST_END();

    free( fullpath );

    close( nullfd );

    TEST_ALL_PASSED();

    return 0;
}
