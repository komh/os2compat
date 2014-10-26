/*
 * sock_pipe() test program
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
#include <process.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "non-block-pipe.h"

#ifndef STDOUT_FILENO
#define STDOUT_FILENO  1
#endif

int main( void )
{
    int   ph[ 2 ];
    int   saved_stdout;
    int   pid;
    FILE *fp;
    char  line[ 512 ];

    /* this program will quit without printing anything if a child program is
     * not built with kLIBC
     */
    sock_pipe( ph );

    saved_stdout = dup( STDOUT_FILENO );
    dup2( ph[ 1 ], STDOUT_FILENO );

    pid = spawnlp( P_NOWAIT, "pwd.exe", "pwd.exe", NULL );

    dup2( saved_stdout, STDOUT_FILENO );
    close( saved_stdout);

    /* If you change O_NONBLOCK to 0, then this program will not quit until
     * Ctrl-C is pressed
     */
    fcntl( ph[ 0 ], F_SETFL, O_NONBLOCK );

    fp = fdopen( ph[ 0 ], "rt");

    while( fgets( line, sizeof( line ), fp))
        fputs( line, stdout );

    waitpid( pid, NULL, 0 );

    fclose( fp );

    close( ph[ 0 ]);
    close( ph[ 1 ]);

    return 0;
}
