/*
 * spawn*() to support very large arguments and/or environment strings
 * for OS/2 kLIBC
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

#include <sys/builtin.h>

/* This should be same as one in _response.c */
#define RSP_ENV_FILE_KEY    "@__KLIBC_ENV_RESPONSE__@"

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

/*
 * On OS/2, a total size of arguments is supported up to 32KiB. This applies
 * to environment strings, too. For the safety, however, use a few lesser
 * value than 32KiB.
 */
#define ARG_SIZE_MAX    ( 32 * 1024 - 256 )
#define ENV_SIZE_MAX    ( 32 * 1024 - 256 )

/*
 * Max size of an environment string to determine if insane.
 * For example, `make check' of automake v1.16.1 passes an environment string
 * whose length is about 80 * 400, insane!
 * I don't think that a normal environment string fills up a text screen.
 */
#define ENV_STR_MAX     ( 80 * 25 ) /* excluding NUL */

int spawnve( int mode, const char *name, char * const argv[],
             char * const envp[])
{
    int rc;
    int saved_errno;

    char *rsp_argv[ 3 ];
    char  rsp_name_arg[] = "@spawn-arg-rsp-XXXXXX";
    char *rsp_name = &rsp_name_arg[ 1 ];

    char **rsp_envp;
    char  rsp_name_env_str[] = RSP_ENV_FILE_KEY "=@spawn-env-rsp-XXXXXX";
    char *rsp_name_env = NULL;

    int fd;

    int arg_big = 0, env_big = 0;
    int env_count;
    int n;

    char * const *p;

    /* check size of arguments */
    n = 1;      /* an additional byte for zero at end */
    for( p = argv; *p; p++ )
        n += strlen( *p ) + 1;

    arg_big = n > ARG_SIZE_MAX;

    if( arg_big )
    {
        /* create a response file for arguments */
        if(( fd = mkstemp( rsp_name )) == -1 )
            return -1;

        /* write all the arguments except the 0th program name */
        for( p = argv + 1; *p; p++ )
        {
            write( fd, *p, strlen( *p ));
            write( fd, "\n", 1 );
        }

        close( fd );

        rsp_argv[ 0 ] = argv[ 0 ];
        rsp_argv[ 1 ] = rsp_name_arg;
        rsp_argv[ 2 ] = NULL;

        argv = rsp_argv;
    }

    /* check size and count of environment strings */
    n = 1;      /* an additional byte for zero at end */
    for( env_count = 0, p = envp ? envp : environ; *p; env_count++, p++ )
        n += strlen( *p ) + 1;

    env_big = n > ENV_SIZE_MAX;

    if( env_big )
    {
        /* create a response file for environment strings */
        int i;

        rsp_name_env = strrchr( rsp_name_env_str, '@') + 1;

        if(( fd = mkstemp( rsp_name_env )) == -1 )
        {
            saved_errno = errno;

            if( arg_big )
                remove( rsp_name );

            errno = saved_errno;

            return -1;
        }

        rsp_envp = alloca( sizeof( *rsp_envp ) *
                           ( env_count + 1 /* for NULL */));

        n = 1; /* an additional byte for zero at end */
        /* write the environment strings with maintaining the compatibility
         * with normal programs as much as possible */
        for(i = 0, p = envp ? envp : environ; *p; p++ )
        {
            int len = strlen( *p );

            /* pass insane environment string or environment strings after
             * a total size of environment strings is bigger than ENV_SIZE_MAX
             * via a response file */
            if( len > ENV_STR_MAX || ( n += len + 1 ) > ENV_SIZE_MAX )
            {
                write( fd, *p, len );
                write( fd, "\n", 1 );
            }
            else
                rsp_envp[ i++ ] = *p;
        }

        close( fd );

        /* ENV_MAX_SIZE has a enough margin for this */
        rsp_envp[ i ] = rsp_name_env_str;
        rsp_envp[ i + 1 ] = NULL;

        envp = rsp_envp;
    }

    /* add a response file to a list before spawning a child because
     * spawning with P_OVERLAY does not return */
    if(( mode & 0xFF ) == P_OVERLAY )
    {
        if( arg_big )
            spawn_add_rsp_temp( 0, rsp_name );

        if( env_big )
            spawn_add_rsp_temp( 0, rsp_name_env );
    }

    rc = _std_spawnve( mode, name, argv, envp );
    saved_errno = errno;

    /* make a response file list to clean up later if spawned a child
     * successfully except P_WAIT */
    if( rc >= 0 && ( mode & 0xFF ) != P_WAIT )
    {
        if( arg_big )
            spawn_add_rsp_temp( rc, rsp_name );

        if( env_big )
            spawn_add_rsp_temp( rc, rsp_name_env );
    }
    else                        /* failed or P_WAIT ? */
    {                           /* remove immediately */
        if( arg_big )
            remove( rsp_name );

        if( env_big )
            remove( rsp_name_env );
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
