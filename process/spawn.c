/*
 * spawn*() to support a very long command line for OS/2 kLIBC
 *
 * Copyright (C) 2014-2022 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#define INCL_DOS
#include <os2.h>

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <io.h>
#include <process.h>
#include <errno.h>

struct rsp_temp
{
    int    pid;
    char  *name;
    struct rsp_temp *next;
};

static struct rsp_temp *rsp_temp_start = NULL;

static void spawn_add_rsp_temp( int pid, const char *name )
{
    struct rsp_temp *rsp_temp_new;

    rsp_temp_new       = malloc( sizeof( *rsp_temp_new ));
    rsp_temp_new->pid  = pid;
    rsp_temp_new->name = strdup( name );
    rsp_temp_new->next = rsp_temp_start;

    rsp_temp_start = rsp_temp_new;
}

static void spawn_remove_rsp_temp( int pid )
{
    struct rsp_temp *rsp_temp;
    struct rsp_temp *rsp_temp_prev = NULL;
    struct rsp_temp *rsp_temp_next = NULL;

    for( rsp_temp = rsp_temp_start; rsp_temp; rsp_temp = rsp_temp_next )
    {
        rsp_temp_next = rsp_temp->next;

        if( pid == -1 || rsp_temp->pid == pid )
        {
            if (rsp_temp_start == rsp_temp)
                rsp_temp_start = rsp_temp_next;
            else    /* rsp_temp_prev must not be NULL */
              rsp_temp_prev->next = rsp_temp_next;

            remove( rsp_temp->name );
            free( rsp_temp->name );
            free( rsp_temp );

            if( pid != -1 )
                break;
        }

        rsp_temp_prev = rsp_temp;
    }
}

static VOID APIENTRY spawn_cleanup( VOID )
{
    spawn_remove_rsp_temp( -1 );

    DosExitList( EXLST_EXIT, NULL );
}

__attribute__((constructor))
static void spawn_startup( void )
{
    /*
     * Workaround the bug of kLIBC v0.6.6 which is to fail to execute the
     * program when a directory with the same name in the current directory.
     * For example, some commands such as `git push' fails if a directory named
     * `git' in the current directory.
     * This has effects on codes using _path2() such as spawnvpe().
     */
     if (!getenv("EMXPATH"))
        putenv("EMXPATH=");

    /* add spawn_cleanup() to a exit list to clean response files up later */
     DosExitList( EXLST_ADD | 0x00007A00, ( PFNEXITLIST )spawn_cleanup );
}

/* alias */
int _std_spawnve( int mode, const char *name, char * const argv[],
                  char * const envp[]);

int spawnve( int mode, const char *name, char * const argv[],
             char * const envp[])
{
    char *rsp_argv[ 3 ];
    char  rsp_name_arg[] = "@spawnvpe-rsp-XXXXXX";
    char *rsp_name = &rsp_name_arg[ 1 ];
    int   i;
    int   rc;
    int   saved_errno;

    rc = _std_spawnve( mode, name, argv, envp );
    saved_errno = errno;

    /* arguments too long? */
    if( rc == -1 && errno == EINVAL )
    {
        /* use a response file */
        int fd;

        if(( fd = mkstemp( rsp_name )) == -1 )
        {
            errno = saved_errno;

            return -1;
        }

        /* write all the arguments except a 0th program name */
        for( i = 1; argv[ i ]; i++ )
        {
            write( fd, argv[ i ], strlen( argv[ i ]));
            write( fd, "\n", 1 );
        }

        close( fd );

        /* add a response file to a list before spawning a child because
         * spawning with P_OVERLAY does not return */
        if(( mode & 0xFF ) == P_OVERLAY )
            spawn_add_rsp_temp( 0, rsp_name );

        rsp_argv[ 0 ] = argv[ 0 ];
        rsp_argv[ 1 ] = rsp_name_arg;
        rsp_argv[ 2 ] = NULL;

        rc = _std_spawnve( mode, name, rsp_argv, envp );
        saved_errno = errno;

        /* make a response file list to clean up later if spawned a child
         * successfully except P_WAIT */
        if( rc >= 0 && ( mode & 0xFF ) != P_WAIT )
            spawn_add_rsp_temp( rc, rsp_name );
        else                        /* failed or P_WAIT ? */
            remove( rsp_name );     /* remove immediately */
    }

    errno = saved_errno;

    return rc;
}

int spawnv( int mode, const char *name, char * const argv[])
{
    return spawnve( mode, name, argv, NULL );
}

int spawnvp( int mode, const char *name, char * const argv[])
{

    return spawnvpe( mode, name, argv, NULL );
}

int spawnvpe( int mode, const char *name, char * const argv[],
              char * const envp[])
{
    char path[ _MAX_PATH ];

    if( _path2( name, ".exe", path, sizeof( path )) == 0)
        return spawnve( mode, path, argv, envp );

    return -1;
}

int spawnl( int mode, const char *name, const char *arg0, ... )
{
    /* as OS/2 kLIBC does */
    return spawnv( mode, name,  ( char * const * )&arg0 );
}

int spawnle( int mode, const char *name, const char *arg0, ... )
{
    /* as OS/2 kLIBC does */
    va_list argp;
    char * const *envp;

    va_start( argp, arg0 );
    while( va_arg( argp, const char * ) != NULL )
        /* nothing */;

    envp = va_arg( argp, char * const * );
    va_end( argp );

    return spawnve( mode, name, ( char * const * )&arg0, envp );
}

int spawnlp( int mode, const char *name, const char *arg0, ... )
{
    /* as OS/2 kLIBC does */
    return spawnvp( mode, name,  ( char * const * )&arg0 );
}

int spawnlpe( int mode, const char *name, const char *arg0, ... )
{
    /* as OS/2 kLIBC does */
    va_list argp;
    char * const *envp;

    va_start( argp, arg0 );
    while( va_arg( argp, const char * ) != NULL )
        /* nothing */;

    envp = va_arg( argp, char * const * );
    va_end( argp );

    return spawnvpe( mode, name, ( char * const * )&arg0, envp );
}
