/*
 * _response() environment strings test program
 *
 * Copyright (C) 2022-2023 KO Myung-Hun <komh@chollian.net>
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

#define RESPONSE_NAME "_response-env.rsp"

static const char *keys[] = {"KEY1=", "KEY2="};
static char lastChars[] = {'0', '1'};

static void spawn( const char *prog )
{
    char *envp[ 2 ];
    FILE *f;
    int i, j;

    f = fopen( RESPONSE_NAME, "wt");

    for( i = 0; i < sizeof( keys ) / sizeof( keys[ 0 ]); i++ )
    {
        fprintf( f, keys[ i ]);
        for( j = 0; j < LINE_LENGTH - 1; j++ )
            fputc('X', f );
        fputc( lastChars[ i ], f );
        fputc('\n', f );
    }

    fclose( f );

    envp[ 0 ] = "@__KLIBC_ENV_RESPONSE__@=@" RESPONSE_NAME;
    envp[ 1 ] = NULL;

    printf("Testing _response() for a very large environment...\n");

    TEST_EQUAL_MSG( spawnlpe( P_WAIT, prog, prog, CHILD_MAGIC, NULL, envp ),
                    0, "PARENT");

    remove( RESPONSE_NAME );

    printf("All tests PASSED\n");
}

int main( int argc, char *argv[])
{
    if( argc > 1 && !strcmp( argv[ 1 ], CHILD_MAGIC ))
    {
        int i;

        _response( &argc, &argv );

        for( i = 0; environ[ i ]; i++ )
        {
            int keyLen = strlen( keys[ i ]);
            int valLen = strlen( environ[ i ] + keyLen );
            char ch = environ[ i ][ keyLen + valLen - 1 ];

            printf("Testing a key part of environ[%d]\n", i );
            TEST_EQUAL_MSG( strncmp( keys[ i ], environ[ i ], keyLen ), 0,
                            "CHILD");

            printf("Testing a length of a value part of environ[%d]\n", i );
            TEST_EQUAL_MSG( valLen, LINE_LENGTH, "CHILD" );

            printf("Testing the last char of environ[%d]\n", i );
            TEST_EQUAL_MSG( ch, lastChars[ i ], "CHILD");

            printf("\n");
        }

        return 0;
    }

    spawn( argv[ 0 ]);

    return 0;
}

