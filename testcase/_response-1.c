/*
 * _response() test program
 *
 * Copyright (C) 2016 KO Myung-Hun <komh@chollian.net>
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

#define RESPONSE_NAME "_response.rsp"

static void spawn( const char *prog )
{
    FILE *f;
    int i;

    f = fopen( RESPONSE_NAME, "wt");

    for( i = 0; i < LINE_LENGTH - 1; i++ )
        fputc('X', f );
    fputc('0', f );
    fputc('\n', f );

    for( i = 0; i < LINE_LENGTH - 1; i++ )
        fputc('X', f );
    fputc('1', f );

    fclose( f );

    spawnlp( P_WAIT, prog, prog, "@" RESPONSE_NAME, NULL );

    remove( RESPONSE_NAME );
}

int main( int argc, char *argv[])
{
    _response( &argc, &argv );

    if( argc > 1 )
    {
        int i;

        for( i = 0; i < argc; i++ )
            printf("a length of argv[%d] = %d, the last char = %c\n",
                   i, strlen( argv[ i ]), argv[ i ][ strlen( argv[ i ]) - 1 ]);

        return 0;
    }

    spawn( argv[ 0 ]);

    return 0;
}
