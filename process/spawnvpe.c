/*
 * spawnvpe() to support a very long command line for OS/2 kLIBC
 *
 * Copyright (C) 2014 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#include <stdlib.h>
#include <string.h>
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

static void spawnvpe_add_rsp_temp( int pid, const char *name )
{
    struct rsp_temp *rsp_temp_new;

    rsp_temp_new       = malloc( sizeof( *rsp_temp_new ));
    rsp_temp_new->pid  = pid;
    rsp_temp_new->name = strdup( name );
    rsp_temp_new->next = rsp_temp_start;

    rsp_temp_start = rsp_temp_new;
}

static void spawnvpe_remove_rsp_temp( int pid )
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

__attribute__((destructor))
static void spawnvpe_cleanup( void )
{
    spawnvpe_remove_rsp_temp( -1 );
}

/* alias */
int _std_spawnvpe( int mode, const char *name, char * const argv[],
                   char * const envp[]);

int spawnvpe( int mode, const char *name, char * const argv[],
              char * const envp[])
{
    char *rsp_argv[ 3 ];
    char  rsp_name_arg[] = "@spawnvpe-rsp-XXXXXX";
    char *rsp_name = &rsp_name_arg[ 1 ];
    int   i;
    int   rc;
    int   saved_errno;

    rc = _std_spawnvpe( mode, name, argv, envp );
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

        rsp_argv[ 0 ] = argv[ 0 ];
        rsp_argv[ 1 ] = rsp_name_arg;
        rsp_argv[ 2 ] = NULL;

        rc = _std_spawnvpe( mode, name, rsp_argv, envp );
        saved_errno = errno;

        /* make a response file list to clean up later if spawned a child
         * successfully except P_WAIT */
        if( rc >= 0 && ( mode & 0xFF ) != P_WAIT )
            spawnvpe_add_rsp_temp( rc, rsp_name );
        else                        /* failed or P_WAIT ? */
            remove( rsp_name );     /* remove immediately */
    }

    errno = saved_errno;

    return rc;
}
