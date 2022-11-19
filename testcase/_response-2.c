/*
 * _response() environment strings test program
 *
 * Copyright (C) 2022 KO Myung-Hun <komh@chollian.net>
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

#define LINE_LENGTH ( 1000 * 1000 )

#define CHILD_MAGIC "SPAWN-CHILD"

#define RESPONSE_NAME "_response-env.rsp"

static void spawn( const char *prog )
{
    char *envp[ 2 ];
    FILE *f;
    int i;

    f = fopen( RESPONSE_NAME, "wt");

    fprintf( f, "KEY1=");
    for( i = 0; i < LINE_LENGTH - 1; i++ )
        fputc('X', f );
    fputc('0', f );
    fputc('\n', f );

    fprintf( f, "KEY2=");
    for( i = 0; i < LINE_LENGTH - 1; i++ )
        fputc('X', f );
    fputc('1', f );

    fclose( f );

    envp[ 0 ] = "@__KLIBC_ENV_RESPONSE__@=@" RESPONSE_NAME;
    envp[ 1 ] = NULL;

    spawnlpe( P_WAIT, prog, prog, CHILD_MAGIC, NULL, envp );

    remove( RESPONSE_NAME );
}

int main( int argc, char *argv[])
{
    _response( &argc, &argv );

    if( argc > 1 && !strcmp( argv[ 1 ], CHILD_MAGIC))
    {
        int i;

        for( i = 0; environ[ i ]; i++ )
            printf("a length of environ[%d] = %d, the last char = %c\n",
                   i, strlen( environ[ i ]),
                   environ[ i ][ strlen( environ[ i ]) - 1 ]);

        return 0;
    }

    spawn( argv[ 0 ]);

    return 0;
}

