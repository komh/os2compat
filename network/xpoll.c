/*
 * xpoll() implementation similar to kqueue() and epoll() for OS/2 kLIBC
 *
 * Copyright (C) 2021 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

/*
 * Dependecies: network/poll.c
 */

#define INCL_DOS
#include <os2.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/select.h>
#include <sys/stat.h>

#include "xpoll.h"

struct os2compat_xpollset
{
    int nfds;
    struct pollfd fds[ FD_SETSIZE ];
};

/**
 * Binary search for a file descriptor in a xpollset instance
 *
 * @param xpset A xpollset instance
 * @param[in] fd A file descriptor to search
 * @returns Positive or zero index of @a fd if found
 * @returns Negative index if not found. -Index - 1 means the index where @a fd
            should be placed.
 */
static int binsearch( struct os2compat_xpollset *xpset, int fd )
{
    int left, right;
    int mid;
    int val;

    if( xpset->nfds == 0 )
        return -1;  /* -0 - 1 */

    left = 0;
    right = xpset->nfds - 1;

    while( left < right )
    {
        mid = ( left + right ) / 2;
        val = xpset->fds[ mid ].fd;

        if( val == fd )
            return mid;

        if( val < fd )
            left = mid + 1;
        else /* if( val > fd ) */
            right = mid - 1;
    }

    val = xpset->fds[ left ].fd;

    if( val == fd )
        return left;

    if( val < fd )
        return -( left + 1 ) - 1;

    /* val > fd */
    return -left - 1;
}

struct os2compat_xpollset *os2compat_xpoll_create( void )
{
    return calloc( sizeof( struct os2compat_xpollset ), 1 );
}

int os2compat_xpoll_destroy( struct os2compat_xpollset *xpset )
{
    free( xpset );

    return 0;
}

/**
 * Check fd is supported
 *
 * @param[in] fd A file descriptor to check
 * @return 0 if supported, otherwise -1
 */
static int check_fd( int fd )
{
    struct stat st;
    ULONG ulState;

    /* accept negative fd, but do nothing for it like poll() */
    if( fd < 0 )
        return 0;

    if( fstat( fd, &st ) == -1 )
        return -1;

    /* files or sockets */
    if( S_ISREG( st.st_mode ) || S_ISSOCK( st.st_mode ))
        return 0;

    /* named pipes */
    if( DosQueryNPHState( fd, &ulState ) == 0 )
        return 0;

    /* not supported handles */
    errno = EINVAL;

    return -1;
}

int os2compat_xpoll_add( struct os2compat_xpollset *xpset,
                         int fd, unsigned event )
{
    int pos;

    if( check_fd( fd ) == -1 )
        return -1;

    if( xpset->nfds == FD_SETSIZE )
    {
        errno = ENOMEM;
        return -1;
    }

    pos = binsearch( xpset, fd );
    if( pos < 0 )
    {
        /* Not found. */
        pos = -pos - 1;

        /* Insert empty slot. */
        memmove( &xpset->fds[ pos + 1 ], &xpset->fds[ pos ],
                 ( xpset->nfds - pos ) * sizeof( xpset->fds[ 0 ]));

        xpset->nfds++;
    }

    xpset->fds[ pos ].fd = fd;
    xpset->fds[ pos ].events = event;
    xpset->fds[ pos ].revents = 0;

    return 0;
}

int os2compat_xpoll_del( struct os2compat_xpollset *xpset, int fd )
{
    int pos;

    pos = binsearch( xpset, fd );
    if( pos < 0 )
    {
        errno = EINVAL;
        return -1;
    }

    /* Remove slot. */
    memmove( &xpset->fds[ pos ], &xpset->fds[ pos + 1 ],
             ( xpset->nfds - pos - 1 ) * sizeof( xpset->fds[ 0 ]));

    xpset->nfds--;

    return 0;
}

int os2compat_xpoll_query( struct os2compat_xpollset *xpset,
                           int fd, struct pollfd *pfd )
{
    int pos;

    pos = binsearch( xpset, fd );
    if( pos < 0 )
    {
        errno = EINVAL;
        return -1;
    }

    if( pfd )
        memcpy( pfd, &xpset->fds[ pos ], sizeof( *pfd ));

    return 0;
}

int os2compat_xpoll_wait( struct os2compat_xpollset *xpset,
                          struct pollfd *fds, int maxfds, int timeout )
{
    int nfds;

    nfds = poll( xpset->fds, xpset->nfds, timeout );
    if( nfds > 0 )
    {
        int i, j;

        for( i = 0, j = 0; j < nfds; i++ )
        {
            if( xpset->fds[ i ].revents != 0 )
            {
                if( j < maxfds )
                    fds[ j ] = xpset->fds[ i ];
                j++;

                /* Clear revents. */
                xpset->fds[ i ].revents = 0;
            }
        }

        /* Up to maxfds */
        if( nfds > maxfds )
            nfds = maxfds;
    }

    return nfds;
}
