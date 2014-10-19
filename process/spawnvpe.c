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

static void spawnvpe_cleanup( void )
{
    spawnvpe_remove_rsp_temp( -1 );
}

/* alias */
int _std_spawnvpe( int mode, const char *name, char * const argv[],
                   char * const envp[]);

/* OS/2 can process a command line up to 32K */
#define MAX_CMD_LINE_LEN 32768

int spawnvpe( int mode, const char *name, char * const argv[],
              char * const envp[])
{
    static int registered = 0;

    char *rsp_argv[ 3 ];
    char  rsp_name_arg[] = "@spawnvpe-rsp-XXXXXX";
    char *rsp_name = &rsp_name_arg[ 1 ];
    int   arg_len = 0;
    int   i;
    int   rc;

    /* register cleanup entry */
    if( !registered )
    {
      if( !atexit( spawnvpe_cleanup ))
        registered = 1;
    }

    for( i = 0; argv[ i ]; i++ )
        arg_len += strlen( argv[ i ]) + 1;

    /* if a length of command line is longer than MAX_CMD_LINE_LEN, then use
     * a response file. OS/2 cannot process a command line longer than 32K.
     * Of course, a response file cannot be recognized by a normal OS/2
     * program, that is, neither non-EMX or non-kLIBC. But it cannot accept
     * a command line longer than 32K in itself. So using a response file
     * in this case, is an acceptable solution */
    if( arg_len > MAX_CMD_LINE_LEN )
    {
        int fd;

        if(( fd = mkstemp( rsp_name )) == -1 )
            return -1;

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

        argv = rsp_argv;
    }

    rc = _std_spawnvpe( mode, name, argv, envp );

    /* a response file was generated ? */
    if( argv == rsp_argv )
    {
        /* make a response file list to clean up later if spawned a child
         * successfully except P_WAIT */
        if( rc >= 0 && ( mode & 0xFF ) != P_WAIT )
            spawnvpe_add_rsp_temp( rc, rsp_name );
        else                        /* failed or P_WAIT ? */
            remove( rsp_name );     /* remove immediately */
    }

    return rc;
}
