/*
 * xpoll() test program
 *
 * Copyright (C) 2021-2023 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "test.h"

#include "xpoll.h"

int xpoll_test( const char *msg, struct xpollset *xpset,
                struct pollfd *fds, int n, int timeout, int *res )
{
    struct timeval tv1;
    struct timeval tv2;
    int i;
    int rc;

    printf("%s\n", msg );

    gettimeofday( &tv1, NULL );

    rc = xpoll_wait( xpset, fds, n, timeout );

    gettimeofday( &tv2, NULL );

    printf("rc = %d(%d), duration = %ld (ms)\n",
           rc, res[ 0 ],
           ( tv2.tv_sec * 1000 + tv2.tv_usec / 1000 ) -
           ( tv1.tv_sec * 1000 + tv1.tv_usec / 1000 ));

    for( i = 0; i < rc; i++ )
    {
        printf("fd = %d, events = %x\n", fds[ i ].fd, fds[ i ].events );

        TEST_EQUAL( fds[ i ].revents, res[ i + 1]);
    }

    printf("\n");

    return 0;
}

#define INVALID_HANDLE  100
#define NEGATIVE_HANDLE -1

int main( void )
{
    struct xpollset *xpset;
    int fd;
    int sv[ 2 ];
    struct pollfd fds[ 4 ];
    int res[ 5 ];

    printf("Testing xpoll()...\n");

    TEST_BOOL( xpset = xpoll_create(), 1 );

    TEST_EQUAL( xpoll_add( xpset, INVALID_HANDLE, POLLIN ), -1 );
    TEST_EQUAL( xpoll_add( xpset, NEGATIVE_HANDLE, POLLIN ), 0 );

    fd = open("xpoll-1.c", O_RDONLY );

    TEST_EQUAL( xpoll_add( xpset, fd, POLLIN ), 0 );

    socketpair( AF_LOCAL, SOCK_STREAM, 0, sv );

    TEST_EQUAL( xpoll_add( xpset, sv[ 0 ], POLLIN ), 0 );

    res[ 0 ] = 1;
    res[ 1 ] = POLLIN;
    res[ 2 ] = 0;
    res[ 3 ] = 0;
    res[ 4 ] = 0;
    xpoll_test("Negative, regular, not-read-ready-socket, 1000 timeout.",
               xpset, fds, 4, 1000, res);

    TEST_EQUAL( xpoll_del( xpset, NEGATIVE_HANDLE ), 0 );
    TEST_EQUAL( xpoll_del( xpset, fd ), 0 );

    res[ 0 ] = 0;
    res[ 1 ] = POLLIN;
    res[ 2 ] = 0;
    res[ 3 ] = 0;
    res[ 4 ] = 0;
    xpoll_test("Not-read-ready-socket only, 1000 timeout.",
               xpset, fds, 4, 1000, res );

    write( sv[ 1 ], "\0", 1 );

    TEST_EQUAL( xpoll_add( xpset, NEGATIVE_HANDLE, POLLIN ), 0 );
    TEST_EQUAL( xpoll_add( xpset, fd, POLLIN ), 0 );

    res[ 0 ] = 2;
    res[ 1 ] = POLLIN;
    res[ 2 ] = POLLIN;
    res[ 3 ] = 0;
    res[ 4 ] = 0;
    xpoll_test("Negative, regular, read-ready-socket, 1000 timeout.",
               xpset, fds, 4, 1000, res );

    TEST_EQUAL( xpoll_del( xpset, NEGATIVE_HANDLE ), 0 );
    TEST_EQUAL( xpoll_del( xpset, fd ), 0 );

    res[ 0 ] = 1;
    res[ 1 ] = POLLIN;
    res[ 2 ] = 0;
    res[ 3 ] = 0;
    res[ 4 ] = 0;
    xpoll_test("Read-ready-socket only, 1000 timeout.",
               xpset, fds, 4, 1000, res );

    close( fd );
    close( sv[ 0 ]);
    close( sv[ 1 ]);

    printf("All tests PASSED\n");

    return 0;
}
