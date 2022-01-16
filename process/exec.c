/*
 * exec*() to support a very long command line for OS/2 kLIBC
 *
 * Copyright (C) 2022 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

/*
 * Dependencies: process/spawn.c
 */

#include <stdlib.h>
#include <stdarg.h>
#include <process.h>

int execv( const char *name, char * const argv[])
{
    return spawnv( P_OVERLAY, name, argv );
}

int execve( const char *name, char * const argv[], char * const envp[])
{
    return spawnve( P_OVERLAY, name, argv, envp );
}

int execvp( const char *name, char * const argv[])
{

    return spawnvp( P_OVERLAY, name, argv );
}

int execvpe( const char *name, char * const argv[], char * const envp[])
{
    return spawnvpe( P_OVERLAY, name, argv, envp );
}

int execl( const char *name, const char *arg0, ... )
{
    /* as OS/2 kLIBC does */
    return spawnv( P_OVERLAY, name,  ( char * const * )&arg0 );
}

int execle( const char *name, const char *arg0, ... )
{
    /* as OS/2 kLIBC does */
    va_list argp;
    char * const *envp;

    va_start( argp, arg0 );
    while( va_arg( argp, const char * ) != NULL )
        /* nothing */;

    envp = va_arg( argp, char * const * );
    va_end( argp );

    return spawnve( P_OVERLAY, name, ( char * const * )&arg0, envp );
}

int execlp( const char *name, const char *arg0, ... )
{
    /* as OS/2 kLIBC does */
    return spawnvp( P_OVERLAY, name,  ( char * const * )&arg0 );
}

int execlpe( const char *name, const char *arg0, ... )
{
    /* as OS/2 kLIBC does */
    va_list argp;
    char * const *envp;

    va_start( argp, arg0 );
    while( va_arg( argp, const char * ) != NULL )
        /* nothing */;

    envp = va_arg( argp, char * const * );
    va_end( argp );

    return spawnvpe( P_OVERLAY, name, ( char * const * )&arg0, envp );
}
