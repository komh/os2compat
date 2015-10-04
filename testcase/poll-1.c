/*
 * poll() test program
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
#include <sys/socket.h>
#include <unistd.h>

#include "poll.h"

void poll_test( const char *msg, struct pollfd *fds, int n, int timeout )
{
    int i;
    int rc;

    printf("%s\n", msg );

    rc = poll( fds, n, timeout );

    printf("rc = %d\n", rc );

    for( i = 0; i < n; i++ )
    {
        printf("fd = %d, events = %x, revents = %x\n",
               fds[ i ].fd, fds[ i ].events, fds[ i ].revents );
    }

    printf("\n");
}

#define INVALID_HANDLE  100
#define NEGATIVE_HANDLE -1

int main( void )
{
    struct pollfd fds[ 2 ];
    int sv[ 2 ];

    fds[ 0 ].fd     = INVALID_HANDLE;
    fds[ 0 ].events = POLLIN;

    socketpair( AF_LOCAL, SOCK_STREAM, 0, sv );

    fds[ 1 ].fd     = sv[ 0 ];
    fds[ 1 ].events = POLLIN;

    poll_test("An invalid handle only, 0 timeout.",
              fds, 1, 0 );

    poll_test("An invalid handle only, 1000 timeout.",
              fds, 1, 1000 );

    poll_test("An invalid handle only, -1 timeout.",
              fds, 1, -1 );

    poll_test("A not-read-ready-socket only, 0 timeout.",
              fds + 1, 1, 0 );

    poll_test("A not-read-ready-socket only, 1000 timeout.",
              fds + 1, 1, 1000 );

#if 0
    poll_test("A not-read-ready-socket only, -1 timeout.",
              fds + 1, 1, -1 );
#endif

    poll_test("An invalid handle and a not-read-ready-socket, 0 timeout.",
              fds, 2, 0 );

    poll_test("An invalid handle and a not-read-ready-socket, 1000 timeout.",
              fds, 2, 1000 );

    poll_test("An invalid handle and a not-read-ready-socket, -1 timeout.",
              fds, 2, -1 );

    fds[ 0 ].fd = NEGATIVE_HANDLE;

    poll_test("A negative handle only, 0 timeout.",
              fds, 1, 0 );

    poll_test("A negative only, 1000 timeout.",
              fds, 1, 1000 );

#if 0
    poll_test("A negative handle only, -1 timeout.",
              fds, 1, -1 );
#endif

    poll_test("A negative handle and a not-read-ready-socket, 0 timeout.",
              fds, 2, 0 );

    poll_test("A negative handle and a not-read-ready-socket, 1000 timeout.",
              fds, 2, 1000 );

#if 0
    poll_test("A negative handle and a not-read-ready-socket, -1 timeout.",
              fds, 2, -1 );
#endif

    write( sv[ 1 ], "\0", 1 );

    poll_test("A read-ready-socket only, 0 timeout.",
              fds + 1, 1, 0 );

    poll_test("A read-ready-socket only, 1000 timeout.",
              fds + 1, 1, 1000 );

    poll_test("A read-ready-socket only, -1 timeout.",
              fds + 1, 1, -1 );

    fds[ 0 ].fd = INVALID_HANDLE;

    poll_test("An invalid handle and a read-ready-socket, 0 timeout.",
              fds, 2, 0 );

    poll_test("An invalid handle and a read-ready-socket, 1000 timeout.",
              fds, 2, 1000 );

    poll_test("An invalid handle and a read-ready-socket, -1 timeout.",
              fds, 2, -1 );

    fds[ 0 ].fd = NEGATIVE_HANDLE;

    poll_test("A negative handle and a read-ready-socket, 0 timeout.",
              fds, 2, 0 );

    poll_test("A negative handle and a read-ready-socket, 1000 timeout.",
              fds, 2, 1000 );

    poll_test("A negative handle and a read-ready-socket, -1 timeout.",
              fds, 2, -1 );

    close( sv[ 0 ]);
    close( sv[ 1 ]);

    return 0;
}
