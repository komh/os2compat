/*
 * fcntl() supporting a non-block mode of a named pipe for OS/2 kLIBC
 *
 * Copyright (C) 2014-2021 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#define INCL_DOS
#include <os2.h>

#include <stdarg.h>
#include <fcntl.h>

/* alias */
int _std_fcntl( int handle, int request, ... );

/* override libc fcntl() */
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
