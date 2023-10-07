/*
 * execvpe() test program
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
#include <process.h>

#include "test.h"

#define CHILD_MAGIC "EXEC-CHILD"

#define ARGC    16
#define ARGLEN  ( 4 * 1024 )

static int child( int argc, char *argv[])
{
    int arglen;
    int i;

    printf("Testing execvpe() for a very long command line...\n");

    _response( &argc, &argv );

    arglen = 0;
    for( i = 2; i < argc; i++ )
        arglen += strlen( argv[ i ]);

    TEST_EQUAL_MSG( arglen, ARGC * ARGLEN, "CHILD");

    printf("All tests PASSED\n");

    return 0;
}

int main( int argc, char *argv[])
{
    char *args[ ARGC + 1/* argv[ 0 ] */ + 1/* CHILD_MAGIC */ + 1/* NULL */] =
        { argv[ 0 ], CHILD_MAGIC, };
    int i;
    int rc;

    _response( &argc, &argv );

    if( argc > 1 && !strcmp( argv[ 1 ], CHILD_MAGIC ))
        return child( argc, argv );

    for( i = 0; i < ARGC; i++ )
    {
        args[ i + 2 ] = calloc( 1, ARGLEN + 1/* NUL */);
        memset( args[ i + 2 ], '0' + i, ARGLEN );
    }
    args[ ARGC + 2 ] = NULL;

    rc = execvpe( args[ 0 ], args, NULL );

    /* should not be run after here */

    TEST_EQUAL_MSG( rc, 0, "PARENT");

    return rc;
}
