/*
 * _response() test program
 *
 * Copyright (C) 2016-2023 KO Myung-Hun <komh@chollian.net>
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

#define LINE_LENGTH ( 1000 * 1000 )

#define CHILD_MAGIC "SPAWN-CHILD"

#define RESPONSE_NAME "_response.rsp"

static char lastChars[] = {'0', '1'};

static void spawn( const char *prog )
{
    FILE *f;
    int i;

    f = fopen( RESPONSE_NAME, "wt");

    for( i = 0; i < LINE_LENGTH - 1; i++ )
        fputc('X', f );
    fputc( lastChars[ 0 ], f );
    fputc('\n', f );

    for( i = 0; i < LINE_LENGTH - 1; i++ )
        fputc('X', f );
    fputc( lastChars[ 1 ], f );

    fclose( f );

    printf("Testing _response() for a very long line in a response file...\n");

    TEST_EQUAL_MSG( spawnlp( P_WAIT, prog, prog, CHILD_MAGIC,
                             "@" RESPONSE_NAME, NULL ), 0, "PARENT");

    remove( RESPONSE_NAME );

    printf("All tests PASSED\n");
}

int main( int argc, char *argv[])
{
    if( argc > 1 && !strcmp( argv[ 1 ], CHILD_MAGIC ))
    {
        int i;

        _response( &argc, &argv );

        for( i = 2; i < argc; i++ )
        {
            int len = strlen( argv[ i ]);
            char ch = argv[ i ][ len - 1 ];

            printf("Testing a length of argv[%d]...\n", i );
            TEST_EQUAL_MSG( len, LINE_LENGTH, "CHILD" );

            printf("Testing the last char of argv[%d]...\n", i );
            TEST_EQUAL_MSG( ch, lastChars[ i - 2 ], "CHILD");

            printf("\n");
        }

        return 0;
    }

    spawn( argv[ 0 ]);

    return 0;
}
