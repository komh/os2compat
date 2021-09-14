/*
 * selectex() test program
 *
 * Copyright (C) 2021 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <io.h>

#include <sys/time.h>
#include <sys/socket.h>

#include "selectex.h"

#include "test.h"

struct threadargs
{
    int fd;
    int delay;
    const char *msg;
};

static void writethread( void *arg )
{
    struct threadargs args = *( struct threadargs * )arg;

    _sleep2( args.delay );
    write( args.fd, args.msg, strlen( args.msg ));
}

static void pipetest( void )
{
    struct timeval tv;
    int ph[ 2 ];
    char buf[ 80 ];
    fd_set rdset;
    fd_set wrset;
    struct threadargs args;

    fprintf( stderr, "----- Testing pipes ----\n");

    TEST_EQUAL( pipe( ph ), 0 );

    /* wait mode test */
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    TEST_EQUAL( selectex( 0, NULL, NULL, NULL, &tv ), 0 );

    /* not read-ready test */
    FD_ZERO( &rdset );
    FD_SET( ph[ 0 ], &rdset );
    TEST_EQUAL( selectex( ph[ 0 ] + 1, &rdset, NULL, NULL, &tv ), 0 );

    write( ph[ 1 ], "hello", 5 );

    /* read-ready test */
    FD_ZERO( &rdset );
    FD_SET( ph[ 0 ], &rdset );
    TEST_EQUAL( selectex( ph[ 0 ] + 1, &rdset, NULL, NULL, &tv ), 1 );
    TEST_BOOL( FD_ISSET( ph[ 0 ], &rdset ), 1 );

    /* read-ready by partial read() test */
    TEST_EQUAL( read( ph[ 0 ], buf, 3 ), 3 );
    TEST_EQUAL( selectex( ph[ 0 ] + 1, &rdset, NULL, NULL, &tv ), 1 );

    /* not read-ready by read() test */
    TEST_EQUAL( read( ph[ 0 ], buf, 2 ), 2 );
    TEST_EQUAL( selectex( ph[ 0 ] + 1, &rdset, NULL, NULL, &tv ), 0 );

    /* mis-placed fdset test */
    TEST_EQUAL( selectex( ph[ 0 ] + 1, NULL, &rdset, NULL, &tv ), -1 );

    /* write-ready test */
    FD_ZERO( &wrset );
    FD_SET( ph[ 1 ], &wrset );
    TEST_EQUAL( selectex( ph[ 1 ] + 1, NULL, &wrset, NULL, &tv ), 1 );
    TEST_BOOL( FD_ISSET( ph[ 1 ], &wrset ), 1 );

    /* mis-placed fdset test */
    TEST_EQUAL( selectex( ph[ 1 ] + 1, &wrset, NULL, NULL, &tv ), -1 );

    /* time-limit wait test */
    args.fd = ph[ 1 ];
    args.delay = 100;
    args.msg = "pipe1";
    _beginthread( writethread, NULL, 1024 * 1024, &args );

    FD_ZERO( &rdset );
    FD_SET( ph[ 0 ], &rdset );
    tv.tv_sec = 0;
    tv.tv_usec = 200 * 1000;
    TEST_EQUAL( selectex( ph[ 0 ] + 1, &rdset, NULL, NULL, &tv ), 1 );
    TEST_BOOL( FD_ISSET( ph[ 0 ], &rdset ), 1 );

    /* clear a pipe */
    TEST_EQUAL( read( ph[ 0 ], buf, sizeof( buf )), 5 );

    /* time-limit timeout test */
    args.fd = ph[ 1 ];
    args.delay = 300;
    args.msg = "pipe12";
    _beginthread( writethread, NULL, 1024 * 1024, &args );

    FD_ZERO( &rdset );
    FD_SET( ph[ 0 ], &rdset );
    tv.tv_sec = 0;
    tv.tv_usec = 200 * 1000;
    TEST_EQUAL( selectex( ph[ 0 ] + 1, &rdset, NULL, NULL, &tv ), 0 );

    /* wait for a thread to finish */
    FD_ZERO( &rdset );
    FD_SET( ph[ 0 ], &rdset );
    TEST_EQUAL( selectex( ph[ 0 ] + 1, &rdset, NULL, NULL, NULL ), 1 );
    TEST_BOOL( FD_ISSET( ph[ 0 ], &rdset ), 1 );

    /* clear a pipe */
    TEST_EQUAL( read( ph[ 0 ], buf, sizeof( buf )), 6 );

    close( ph[ 0 ]);
    close( ph[ 1 ]);
}

static void sockettest( void )
{
    struct timeval tv;
    int socks[ 2 ];
    char buf[ 80 ];
    fd_set rdset;
    fd_set wrset;
    struct threadargs args;

    fprintf( stderr, "----- Testing sockets ----\n");

    TEST_EQUAL( socketpair( AF_LOCAL, SOCK_STREAM, 0, socks ), 0 );

    /* not read-ready test */
    FD_ZERO( &rdset );
    FD_SET( socks[ 0 ], &rdset );
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    TEST_EQUAL( selectex( socks[ 0 ] + 1, &rdset, NULL, NULL, &tv ), 0 );

    write( socks[ 1 ], "hello", 5 );

    /* read-ready test */
    FD_ZERO( &rdset );
    FD_SET( socks[ 0 ], &rdset );
    TEST_EQUAL( selectex( socks[ 0 ] + 1, &rdset, NULL, NULL, &tv ), 1 );
    TEST_BOOL( FD_ISSET( socks[ 0 ], &rdset ), 1 );

    /* read-ready by partial read() test */
    TEST_EQUAL( read( socks[ 0 ], buf, 3 ), 3 );
    TEST_EQUAL( selectex( socks[ 0 ] + 1, &rdset, NULL, NULL, &tv ), 1 );

    /* not read-ready by read() test */
    TEST_EQUAL( read( socks[ 0 ], buf, 2 ), 2 );
    TEST_EQUAL( selectex( socks[ 0 ] + 1, &rdset, NULL, NULL, &tv ), 0 );

    /* write-ready test */
    FD_ZERO( &wrset );
    FD_SET( socks[ 1 ], &wrset );
    TEST_EQUAL( selectex( socks[ 1 ] + 1, NULL, &wrset, NULL, &tv ), 1 );
    TEST_BOOL( FD_ISSET( socks[ 1 ], &wrset ), 1 );

    /* time-limit wait test */
    args.fd = socks[ 1 ];
    args.delay = 100;
    args.msg = "socket1";
    _beginthread( writethread, NULL, 1024 * 1024, &args );

    FD_ZERO( &rdset );
    FD_SET( socks[ 0 ], &rdset );
    tv.tv_sec = 0;
    tv.tv_usec = 200 * 1000;
    TEST_EQUAL( selectex( socks[ 0 ] + 1, &rdset, NULL, NULL, &tv ), 1 );
    TEST_BOOL( FD_ISSET( socks[ 0 ], &rdset ), 1 );

    /* clear a socket */
    TEST_EQUAL( read( socks[ 0 ], buf, sizeof( buf )), 7 );

    /* time-limit timeout test */
    args.fd = socks[ 1 ];
    args.delay = 300;
    args.msg = "socket12";
    _beginthread( writethread, NULL, 1024 * 1024, &args );

    FD_ZERO( &rdset );
    FD_SET( socks[ 0 ], &rdset );
    tv.tv_sec = 0;
    tv.tv_usec = 200 * 1000;
    TEST_EQUAL( selectex( socks[ 0 ] + 1, &rdset, NULL, NULL, &tv ), 0 );

    /* wait for a thread to finish */
    FD_ZERO( &rdset );
    FD_SET( socks[ 0 ], &rdset );
    TEST_EQUAL( selectex( socks[ 0 ] + 1, &rdset, NULL, NULL, NULL ), 1 );
    TEST_BOOL( FD_ISSET( socks[ 0 ], &rdset ), 1 );

    /* clear a socket */
    TEST_EQUAL( read( socks[ 0 ], buf, sizeof( buf )), 8 );

    close( socks[ 0 ]);
    close( socks[ 1 ]);
}

static void mixedtest( void )
{
    struct timeval tv;
    FILE *fp;
    int fd;
    int ph [ 2];
    int socks[ 2 ];
    char buf[ 80 ];
    fd_set rdset;
    fd_set wrset;
    struct threadargs args;

    fprintf( stderr, "----- Testing mixed mode ----\n");

    TEST_BOOL( fp = tmpfile(), 1 );
    TEST_EQUAL( pipe( ph ), 0 );
    TEST_EQUAL( socketpair( AF_LOCAL, SOCK_STREAM, 0, socks ), 0 );

    fd = fileno( fp );

    /* read-ready for a file test */
    FD_ZERO( &rdset );
    FD_SET( fd, &rdset );
    FD_SET( ph[ 0 ], &rdset );
    FD_SET( socks[ 0 ], &rdset );
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    TEST_EQUAL( selectex( socks[ 0 ] + 1, &rdset, NULL, NULL, &tv ), 1 );
    TEST_BOOL( FD_ISSET( fd, &rdset ), 1 );

    write( socks[ 1 ], "hello", 5 );

    /* not read-ready for a pipe and read-ready for a socket test */
    FD_ZERO( &rdset );
    FD_SET( ph[ 0 ], &rdset );
    FD_SET( socks[ 0 ], &rdset );
    TEST_EQUAL( selectex( socks[ 0 ] + 1, &rdset, NULL, NULL, &tv ), 1 );
    TEST_BOOL( FD_ISSET( ph[ 0 ], &rdset ), 0 );
    TEST_BOOL( FD_ISSET( socks[ 0 ], &rdset ), 1 );

    write( ph[ 1 ], "world", 5 );

    /* read-ready for a pipe and a socket */
    FD_ZERO( &rdset );
    FD_SET( ph[ 0 ], &rdset );
    FD_SET( socks[ 0 ], &rdset );
    TEST_EQUAL( selectex( socks[ 0 ] + 1, &rdset, NULL, NULL, &tv ), 2 );
    TEST_BOOL( FD_ISSET( ph[ 0 ], &rdset ), 1 );
    TEST_BOOL( FD_ISSET( socks[ 0 ], &rdset ), 1 );

    /* clear a socket */
    TEST_EQUAL( read( socks[ 0 ], buf, sizeof( buf )), 5 );

    /* read-ready for a pipe and not read-ready for a socket */
    FD_ZERO( &rdset );
    FD_SET( ph[ 0 ], &rdset );
    FD_SET( socks[ 0 ], &rdset );
    TEST_EQUAL( selectex( socks[ 0 ] + 1, &rdset, NULL, NULL, &tv ), 1 );
    TEST_BOOL( FD_ISSET( ph[ 0 ], &rdset ), 1 );
    TEST_BOOL( FD_ISSET( socks[ 0 ], &rdset ), 0 );

    /* clear a pipe */
    TEST_EQUAL( read( ph[ 0 ], buf, sizeof( buf )), 5 );

    /* not read-ready for a pipe and a socket test */
    FD_ZERO( &rdset );
    FD_SET( ph[ 0 ], &rdset );
    FD_SET( socks[ 0 ], &rdset );
    TEST_EQUAL( selectex( socks[ 0 ] + 1, &rdset, NULL, NULL, &tv ), 0 );

    /* write-ready for a file, a pipe and a socket test */
    FD_ZERO( &wrset );
    FD_SET( fd, &wrset );
    FD_SET( ph[ 1 ], &wrset );
    FD_SET( socks[ 1 ], &wrset );
    TEST_EQUAL( selectex( socks[ 1 ] + 1, NULL, &wrset, NULL, &tv ), 3 );
    TEST_BOOL( FD_ISSET( fd, &wrset ), 1 );
    TEST_BOOL( FD_ISSET( ph[ 1 ], &wrset ), 1 );
    TEST_BOOL( FD_ISSET( socks[ 1 ], &wrset ), 1 );

    /* time-limit wait for not a read-ready pipe and
     * a read-ready socket test  */
    args.fd = socks[ 1 ];
    args.delay = 100;
    args.msg = "socket123";
    _beginthread( writethread, NULL, 1024 * 1024, &args );

    FD_ZERO( &rdset );
    FD_SET( ph[ 0 ], &rdset );
    FD_SET( socks[ 0 ], &rdset );
    tv.tv_sec = 0;
    tv.tv_usec = 200 * 1000;
    TEST_EQUAL( selectex( socks[ 0 ] + 1, &rdset, NULL, NULL, &tv ), 1 );
    TEST_BOOL( FD_ISSET( ph[ 0 ], &rdset ), 0 );
    TEST_BOOL( FD_ISSET( socks[ 0 ], &rdset ), 1 );

    /* clear a socket */
    TEST_EQUAL( read( socks[ 0 ], buf, sizeof( buf )), 9 );

    /* time-limit timeout for a pipe and a socket test */
    args.fd = socks[ 1 ];
    args.delay = 300;
    args.msg = "socket1234";
    _beginthread( writethread, NULL, 1024 * 1024, &args );

    FD_ZERO( &rdset );
    FD_SET( ph[ 0 ], &rdset );
    FD_SET( socks[ 0 ], &rdset );
    tv.tv_sec = 0;
    tv.tv_usec = 200 * 1000;
    TEST_EQUAL( selectex( socks[ 0 ] + 1, &rdset, NULL, NULL, &tv ), 0 );

    /* wait for a thread to finish */
    FD_ZERO( &rdset );
    FD_SET( ph[ 0 ], &rdset );
    FD_SET( socks[ 0 ], &rdset );
    TEST_EQUAL( selectex( socks[ 0 ] + 1, &rdset, NULL, NULL, NULL ), 1 );
    TEST_BOOL( FD_ISSET( ph[ 0 ], &rdset ), 0 );
    TEST_BOOL( FD_ISSET( socks[ 0 ], &rdset ), 1 );

    /* clear a socket */
    TEST_EQUAL( read( socks[ 0 ], buf, sizeof( buf )), 10 );

    /* time-limit wait for a read-ready pipe and
     * not a read-ready socket test */
    args.fd = ph[ 1 ];
    args.delay = 100;
    args.msg = "pipe1234567";
    _beginthread( writethread, NULL, 1024 * 1024, &args );

    FD_ZERO( &rdset );
    FD_SET( ph[ 0 ], &rdset );
    FD_SET( socks[ 0 ], &rdset );
    tv.tv_sec = 0;
    tv.tv_usec = 200 * 1000;
    TEST_EQUAL( selectex( socks[ 0 ] + 1, &rdset, NULL, NULL, &tv ), 1 );
    TEST_BOOL( FD_ISSET( ph[ 0 ], &rdset ), 1 );
    TEST_BOOL( FD_ISSET( socks[ 0 ], &rdset ), 0 );

    /* clear a pipe */
    TEST_EQUAL( read( ph[ 0 ], buf, sizeof( buf )), 11 );

    /* time-limit timeout fo a pipe and a socket */
    args.fd = ph[ 1 ];
    args.delay = 300;
    args.msg = "pipe12345678";
    _beginthread( writethread, NULL, 1024 * 1024, &args );

    FD_ZERO( &rdset );
    FD_SET( ph[ 0 ], &rdset );
    FD_SET( socks[ 0 ], &rdset );
    tv.tv_sec = 0;
    tv.tv_usec = 200 * 1000;
    TEST_EQUAL( selectex( socks[ 0 ] + 1, &rdset, NULL, NULL, &tv ), 0 );

    /* wait for a thread to finish */
    FD_ZERO( &rdset );
    FD_SET( ph[ 0 ], &rdset );
    FD_SET( socks[ 0 ], &rdset );
    TEST_EQUAL( selectex( ph[ 0 ] + 1, &rdset, NULL, NULL, NULL ), 1 );
    TEST_BOOL( FD_ISSET( ph[ 0 ], &rdset ), 1 );
    TEST_BOOL( FD_ISSET( socks[ 0 ], &rdset ), 0 );

    /* clear a pipe */
    TEST_EQUAL( read( ph[ 0 ], buf, sizeof( buf )), 12 );

    fclose( fp );

    close( socks[ 0 ]);
    close( socks[ 1 ]);

    close( ph[ 0 ]);
    close( ph[ 1 ]);
}

int main( void )
{
    pipetest();

    sockettest();

    mixedtest();

    return 0;
}
