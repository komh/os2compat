/*
 * pipe() with a non-blocking mode for OS/2
 *
 * Copyright (C) 2014 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#define INCL_DOS
#include <os2.h>

#include <stdio.h>          /* for snprintf()   */
#include <stdarg.h>         /* for va_list      */
#include <fcntl.h>          /* for fcntl()      */
#include <process.h>        /* for getpid()     */
#include <io.h>             /* for _imphandle() */
#include <sys/socket.h>     /* for socketpair() */

#define PIPE_NAME_BASE "\\PIPE\\NON-BLOCK-PIPE"

int _std_fcntl( int handle, int request, ... );

int fcntl( int handle, int request, ... )
{
    va_list argPtr;
    int     arg;
    ULONG   ulState;

    va_start( argPtr, request );
    arg = va_arg( argPtr, int );
    va_end( argPtr );

    /* named pipe */
    if( !DosQueryNPHState( handle, &ulState ))
    {
        if( request == F_GETFL )
            return ( ulState & NP_NOWAIT ) ? O_NONBLOCK : 0;

        if( request == F_SETFL )
        {
            if( arg == O_NONBLOCK )
                ulState |= NP_NOWAIT;
            else
                ulState &= ~NP_NOWAIT;

            /* extract relevant flags */
            ulState &= NP_NOWAIT | NP_READMODE_MESSAGE;

            return DosSetNPHState( handle, ulState ) ? -1 : 0;
        }
    }

    return _std_fcntl( handle, request, arg );
}

int named_pipe( int *ph )
{
    static volatile ULONG ulPipes = 0;

    HPIPE hpipe;
    HFILE hpipeWrite;
    ULONG ulAction;

    char szPipeName[ 80 ];

    ulPipes++;
    snprintf( szPipeName, sizeof( szPipeName ),
              "%s\\%x\\%lx", PIPE_NAME_BASE, ( unsigned )getpid(), ulPipes );

    /* NP_NOWAIT should be specified, otherwise DosConnectNPipe() blocks.
     * If you want to change pipes to blocking-mode, then use DosSetNPHState()
     * after DosConnectNPipe()
     */
    DosCreateNPipe( szPipeName,
                    &hpipe,
                    NP_ACCESS_DUPLEX,
                    NP_NOWAIT | NP_TYPE_BYTE | NP_READMODE_BYTE | 1,
                    32768, 32768, 0 );

    DosConnectNPipe( hpipe );

    DosOpen( szPipeName, &hpipeWrite, &ulAction, 0, FILE_NORMAL,
             OPEN_ACTION_OPEN_IF_EXISTS,
             OPEN_SHARE_DENYREADWRITE | OPEN_ACCESS_WRITEONLY,
             NULL );

    /* _imphandle() is not required specifically, because kLIBC imports
     * native handles automatically if needed. But here use _imphandle()
     * specifically.
     */
    ph[ 0 ] = _imphandle( hpipe );
    ph[ 1 ] = _imphandle( hpipeWrite );

    /* Default to blocking mode */
    DosSetNPHState( ph[ 0 ], NP_WAIT | NP_READMODE_BYTE );
    DosSetNPHState( ph[ 1 ], NP_WAIT | NP_READMODE_BYTE );

    return 0;

}

int sock_pipe( int *sv )
{
    return socketpair( AF_LOCAL, SOCK_STREAM, 0, sv );
}

