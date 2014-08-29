/*
 * poll() implementation for OS/2 kLIBC
 *
 * Copyright (C) 2014 KO Myung-Hun <komh@chollian.net>
 *
 * This file was excerpted and modifed from src/os2/thread.c of VLC.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 */

#include <types.h>
#include <unistd.h>
#include <sys/time.h>

#include <stdlib.h>
#include <errno.h>

#include "poll.h"

int poll( struct pollfd *fds, unsigned nfds, int timeout )
{
    fd_set rdset, wrset, exset;

    struct timeval tv = { 0, 0 };

    unsigned i;

    int val = -1;

    FD_ZERO( &rdset );
    FD_ZERO( &wrset );
    FD_ZERO( &exset );
    for( i = 0; i < nfds; i++ )
    {
        int fd = fds[ i ].fd;
        if( val < fd )
            val = fd;

        if(( unsigned )fd >= FD_SETSIZE )
        {
            errno = EINVAL;
            return -1;
        }

        if( fds[ i ].events & POLLIN )
            FD_SET( fd, &rdset );
        if( fds[ i ].events & POLLOUT )
            FD_SET( fd, &wrset );
        if( fds[ i ].events & POLLPRI )
            FD_SET( fd, &exset );
    }

    if( timeout >= 0 )
    {
        div_t d    = div( timeout, 1000 );
        tv.tv_sec  = d.quot;
        tv.tv_usec = d.rem * 1000;
    }

    val = select( val + 1, &rdset, &wrset, &exset,
                      ( timeout >= 0 ) ? &tv : NULL );
    if( val == -1 )
        return -1;

    for( i = 0; i < nfds; i++ )
    {
        int fd = fds[ i ].fd;
        fds[ i ].revents = ( FD_ISSET( fd, &rdset ) ? POLLIN  : 0 )
                         | ( FD_ISSET( fd, &wrset ) ? POLLOUT : 0 )
                         | ( FD_ISSET( fd, &exset ) ? POLLPRI : 0 );
    }

    return val;
}
