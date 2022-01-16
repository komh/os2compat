/*
 * execvpe() test program
 *
 * Copyright (C) 2014 KO Myung-Hun <komh@chollian.net>
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

#define CHILD "spawnvpe-1-child.exe"

#define ARGC    16
#define ARGLEN  ( 4 * 1024 )

int main( void )
{
    char *argv[ ARGC + 1 + 1 ] = { CHILD, };
    int i;
    int rc;

    for( i = 1; i <= ARGC; i++ )
    {
        argv[ i ] = calloc( 1, ARGLEN + 1 );
        memset( argv[ i ], '0' + i, ARGLEN );
    }
    argv[ ARGC + 1 ] = NULL;

    rc = execvpe( CHILD, argv, NULL );

    /* should not be run after here */

    for( i = 1; i <= ARGC; i++ )
        free( argv[ i ]);

    printf("Total length of passing arguments to a child = %d\n",
           strlen( CHILD ) + ARGC * ARGLEN );
    printf("spawnvpe() = %d\n", rc );

    return rc;
}
