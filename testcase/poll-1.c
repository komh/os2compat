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
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "poll.h"

int poll_test( const char *msg, struct pollfd *fds, int n, int timeout,
               int *res )
{
    struct timeval tv1;
    struct timeval tv2;
    int i;
    int rc;
    int passed = 1;

    printf("%s\n", msg );

    gettimeofday( &tv1, NULL );

    rc = poll( fds, n, timeout );

    gettimeofday( &tv2, NULL );

    printf("rc = %d, duration = %ld (ms)\n",
           rc, ( tv2.tv_sec * 1000 + tv2.tv_usec / 1000 ) -
               ( tv1.tv_sec * 1000 + tv1.tv_usec / 1000 ));

    for( i = 0; i < n; i++ )
    {
        printf("fd = %d, events = %x, revents = %x(%x)\n",
               fds[ i ].fd, fds[ i ].events, fds[ i ].revents, res[ i + 1]);

        passed = passed && fds[ i ].revents == res[ i + 1];
    }

    passed = passed && rc == res[ 0 ];

    printf("%s\n", passed ? "PASSED" : "FAILED");

    printf("\n");

    return passed;
}

#define INVALID_HANDLE  100
#define NEGATIVE_HANDLE -1

int main( void )
{
    struct pollfd fds[ 4 ];
    int res[ 5 ];
    int sv[ 2 ];
    int passed = 1;

    fds[ 0 ].fd     = INVALID_HANDLE;
    fds[ 0 ].events = POLLIN;

    fds[ 1 ].fd     = NEGATIVE_HANDLE;
    fds[ 1 ].events = POLLIN;

    fds[ 2 ].fd     = open("poll-1.c", O_RDONLY);
    fds[ 2 ].events = POLLIN;

    socketpair( AF_LOCAL, SOCK_STREAM, 0, sv );

    fds[ 3 ].fd     = sv[ 0 ];
    fds[ 3 ].events = POLLIN;

    res[ 0 ] = 2;
    res[ 1 ] = POLLNVAL;
    res[ 2 ] = 0;
    res[ 3 ] = POLLIN;
    res[ 4 ] = 0;
    passed = passed &&
             poll_test("Invalid, negative, regular, not-read-ready-socket, "
                       "1000 timeout.\n", fds, 4, 1000, res);

    res[ 0 ] = 0;
    res[ 1 ] = 0;
    passed = passed &&
             poll_test("Not-read-ready-socket only, 1000 timeout.\n",
                       fds + 3, 1, 1000, res );

    write( sv[ 1 ], "\0", 1 );

    res[ 0 ] = 3;
    res[ 1 ] = POLLNVAL;
    res[ 2 ] = 0;
    res[ 3 ] = POLLIN;
    res[ 4 ] = POLLIN;
    passed = passed &&
             poll_test("Invalid, negative, regular, read-ready-socket, "
                       "1000 timeout.\n", fds, 4, 1000, res );

    res[ 0 ] = 1;
    res[ 1 ] = POLLIN;
    passed = passed &&
             poll_test("Read-ready-socket only, 1000 timeout.\n",
                       fds + 3, 1, 1000, res );

    close( fds[ 2 ].fd );
    close( sv[ 0 ]);
    close( sv[ 1 ]);

    printf("%s\n", passed ? "All test PASSED" : "Some tests FAILED");

    return 0;
}
