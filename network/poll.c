/*
 * poll() implementation for OS/2 kLIBC
 *
 * Copyright (C) 2014-2021 KO Myung-Hun <komh@chollian.net>
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

/*
 * Dependecies: network/select.c
 */

#define INCL_DOS
#include <os2.h>

#include <types.h>
#include <unistd.h>
#include <sys/time.h>

#include <stdlib.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/stat.h>

#include "poll.h"

static int checkfd( int fd )
{
    struct stat st;
    ULONG ulState;
    int optval, optlen = sizeof( optval );

    /* invalid handle */
    if( fstat( fd, &st ) == -1 )
        return -1;

    /* files */
    if( S_ISREG( st.st_mode ))
        return 0;

    /* sockets */
    if( S_ISSOCK( st.st_mode ) &&
        getsockopt( fd, SOL_SOCKET, SO_TYPE, &optval, &optlen ) == 0 )
        return 0;

    /* named pipes */
    if( DosQueryNPHState( fd, &ulState ) == 0 )
        return 0;

    /* not supported handles */
    return -1;
}

int os2compat_poll( struct os2compat_pollfd *fds, unsigned nfds, int timeout )
{
    fd_set rdset, wrset, exset;

    int non_sockets = 0;

    struct timeval tv = { 0, 0 };

    int val = -1;

    unsigned i;

    FD_ZERO( &rdset );
    FD_ZERO( &wrset );
    FD_ZERO( &exset );
    for( i = 0; i < nfds; i++ )
    {
        int fd = fds[ i ].fd;

        fds[ i ].revents = 0;

        if( checkfd( fd ) == -1 )
        {
            if( fd >= 0 )
            {
                fds[ i ].revents = POLLNVAL;

                non_sockets++;
            }

            continue;
        }

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

    if( non_sockets > 0 )
        timeout = 0;    /* Just check pending sockets */

    /* Sockets included ? */
    if( val != -1)
    {
        struct timeval *ptv = NULL;

        if( timeout >= 0 )
        {
            div_t d    = div( timeout, 1000 );
            tv.tv_sec  = d.quot;
            tv.tv_usec = d.rem * 1000;

            ptv = &tv;
        }

        if( select( val + 1, &rdset, &wrset, &exset, ptv ) == -1 )
            return -1;
    }

    val = 0;
    for( i = 0; i < nfds; i++ )
    {
        int fd = fds[ i ].fd;

        if( fd >= 0 && fds[ i ].revents == 0)
        {
            fds[ i ].revents = ( FD_ISSET( fd, &rdset ) ? POLLIN  : 0 )
                             | ( FD_ISSET( fd, &wrset ) ? POLLOUT : 0 )
                             | ( FD_ISSET( fd, &exset ) ? POLLPRI : 0 );
        }

        if( fds[ i ].revents != 0 )
            val++;
    }

    return val;
}
