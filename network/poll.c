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

#include <sys/socket.h>

#include "poll.h"

int poll( struct pollfd *fds, unsigned nfds, int timeout )
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
        if( getsockopt( fd, SOL_SOCKET, SO_TYPE,
                        &( int ){ 0 }, &( int ){ sizeof( int )}) == -1 &&
            ( errno == ENOTSOCK || errno == EBADF ))
        {
            if (fd >= 0)
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

    /* Sockets included ? */
    if( val != -1 )
    {
        fd_set saved_rdset = rdset;
        fd_set saved_wrset = wrset;
        fd_set saved_exset = exset;

        /* Check pending sockets */
        switch( select( val + 1, &rdset, &wrset, &exset, &tv ))
        {
            case -1 :   /* Error */
                return -1;

            case 0 :    /* Timeout */
                /* Socket only ? */
                if( non_sockets == 0 )
                {
                    struct timeval *ptv = NULL;

                    if( timeout >= 0 )
                    {
                        div_t d    = div( timeout, 1000 );
                        tv.tv_sec  = d.quot;
                        tv.tv_usec = d.rem * 1000;

                        ptv = &tv;
                    }

                    rdset = saved_rdset;
                    wrset = saved_wrset;
                    exset = saved_exset;

                    if( select( val + 1, &rdset, &wrset, &exset, ptv )
                            == -1 )
                        return -1;
                }
                break;

            default:    /* Ready */
                break;
        }
    }

    val = 0;
    for( i = 0; i < nfds; i++ )
    {
        int fd = fds[ i ].fd;

        if( fd >= 0 && fds[ i ].revents == 0)
        {
            int fd = fds[ i ].fd;
            fds[ i ].revents = ( FD_ISSET( fd, &rdset ) ? POLLIN  : 0 )
                             | ( FD_ISSET( fd, &wrset ) ? POLLOUT : 0 )
                             | ( FD_ISSET( fd, &exset ) ? POLLPRI : 0 );
        }

        if( fds[ i ].revents != 0 )
            val++;
    }

    return val;
}
