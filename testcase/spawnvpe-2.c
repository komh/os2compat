/*
 * spawnvpe() environment strings test program
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

#define CHILD "spawnvpe-2-child.exe"

#define ARGC    16
#define ARGLEN  ( 4 * 1024 )

/* spawn*() of kLIBC will crash if ENVC * ENVLEN is about 64KiB */
#define ENVC    16
#define ENVLEN  ( 4 * 1024 )

int main( void )
{
    char *argv[ ARGC + 1 + 1 ] = { CHILD, };
    char *envp[ ENVC + 1 ];
    int i;
    int rc;

    for( i = 1; i <= ARGC; i++ )
    {
        argv[ i ] = calloc( 1, ARGLEN + 1 );
        memset( argv[ i ], '0' + i, ARGLEN );
    }
    argv[ ARGC + 1 ] = NULL;

    for( i = 0; i < ENVC; i++ )
    {
        envp[ i ] = calloc( 1, ENVLEN + 1 );
        envp[ i ][ 0 ] = 'A' + i;
        envp[ i ][ 1 ] = '=';
        memset( envp[ i ] + 2, '0' + i, ENVLEN - 2 );
    }
    envp[ ENVC ] = NULL;

    rc = spawnvpe( P_WAIT, CHILD, argv, envp );

    for( i = 1; i <= ARGC; i++ )
        free( argv[ i ]);

    for( i = 0; i < ENVC; i++ )
        free( envp[ i ]);

    printf("Total length of passing arguments to a child = %d\n",
           strlen( CHILD ) + ARGC * ARGLEN );
    printf("Total length of passing environment strings to a child = %d\n",
           ENVC * ENVLEN );
    printf("spawnvpe() = %d\n", rc );

    return rc;
}
