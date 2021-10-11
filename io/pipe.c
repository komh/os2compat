/*
 * pipe() using a named pipe for OS/2 kLIBC
 *
 * Copyright (C) 2014-2021 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

/*
 * Dependencies: io/fcntl.c
 */

#define INCL_DOS
#include <os2.h>

#include <stdio.h>
#include <process.h>
#include <io.h>

#define PIPE_NAME_BASE "\\PIPE\\OS2COMPAT\\IO\\PIPE"

/* override libc pipe() */
int pipe( int *ph )
{
    static volatile unsigned pipes = 0;

    HPIPE hpipe;
    HFILE hpipeWrite;
    ULONG ulAction;

    char szPipeName[ 80 ];

    __atomic_add_fetch( &pipes, 1, __ATOMIC_RELAXED );
    snprintf( szPipeName, sizeof( szPipeName ),
              "%s\\%x\\%x", PIPE_NAME_BASE, ( unsigned )getpid(), pipes );

    /* NP_NOWAIT should be specified, otherwise DosConnectNPipe() blocks.
     * If you want to change pipes to blocking-mode, then use DosSetNPHState()
     * after DosConnectNPipe()
     */
    DosCreateNPipe( szPipeName,
                    &hpipe,
                    NP_ACCESS_INBOUND | NP_INHERIT,
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

    /* default to blocking mode */
    DosSetNPHState( ph[ 0 ], NP_WAIT | NP_READMODE_BYTE );
    DosSetNPHState( ph[ 1 ], NP_WAIT | NP_READMODE_BYTE );

    return 0;
}
