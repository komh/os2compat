/*
 * mmap( MAP_SHARED without MAP_ANON ) test program
 *
 * Copyright (C) 2016 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#include <stdio.h>
#include <unistd.h>

#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

#include "mmap.h"

#define EXIT(c) do { rc = (c); goto exit_cleanup; } while( 0 )

#define TESTFILE1 "mmap-6-1.test"
#define TESTFILE2 "mmap-6-2.test"

#define KEY_PARENT 0x10
#define KEY_CHILD  0x20

int main( void )
{
    int  fd1 = -1;
    int  fd2 = -1;
    int *p1 = MAP_FAILED;
    int *p2 = MAP_FAILED;
    int  len1 = sizeof( *p1 );
    int  len2 = sizeof( *p2 );
    int  pagesize = getpagesize();
    int  pid;
    const char *parent = "PARENT";
    const char *child = "CHILD";
    const char *me;

    int i;
    int rc = 0;

    fd1 = open( TESTFILE1, O_CREAT | O_RDWR | O_BINARY, S_IREAD | S_IWRITE );
    if( fd1 == -1 )
    {
        fprintf( stderr, "open( %s ) failed\n", TESTFILE1 );

        EXIT( 1 );
    }

    for( i = 0; i < pagesize; i++ )
    {
        if( write( fd1, &i, 1 ) != 1 )
        {
            fprintf( stderr, "write( fd1 ) failed\n");

            EXIT( 1 );
        }
    }

    fd2 = open( TESTFILE2, O_CREAT | O_RDWR | O_BINARY, S_IREAD | S_IWRITE );
    if( fd2 == -1 )
    {
        fprintf( stderr, "open( %s ) failed\n", TESTFILE2 );

        EXIT( 1 );
    }

    for( i = 0; i < pagesize; i++ )
    {
        if( write( fd2, &i, 1 ) != 1 )
        {
            fprintf( stderr, "write( fd2 ) failed\n");

            EXIT( 1 );
        }
    }

    p1 = mmap( NULL, len1, PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0 );
    if( p1 == MAP_FAILED )
    {
        fprintf( stderr, "mmap( p1 ) failed\n");

        EXIT( 1 );
    }

    p2 = mmap( NULL, len2, PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0 );
    if( p2 == MAP_FAILED )
    {
        fprintf( stderr, "mmap( p2 ) failed\n");

        EXIT( 1 );
    }

    *p1 = KEY_CHILD; *p2 = KEY_PARENT;

    pid = fork();
    switch( pid )
    {
        case -1 :
            fprintf( stderr, "fork() failed\n");

            EXIT( 1 );
            break;

        case 0 :
            me = child;
            printf("%s: %s: *p2 = %d(%d)\n",
                   me, *p2 == KEY_PARENT ? "PASSED" : "FAILED",
                   *p2, KEY_PARENT );

            *p2 = KEY_CHILD;
            printf("%s: Set *p2 to %d\n", me, *p2 );
            break;

        default :
            me = parent;
            printf("%s: %s: *p1 = %d(%d)\n",
                   me, *p1 == KEY_CHILD ? "PASSED" : "FAILED",
                   *p1, KEY_CHILD );

            *p1 = KEY_PARENT;
            printf("%s: Set *p1 to %d\n", me, *p1 );
            break;
    }

exit_cleanup:
    if( p2 != MAP_FAILED && munmap( p2, len2 ))
    {
        fprintf( stderr, "%s: munmap( p2 ) failed\n", me);

        rc = 1;
    }

    if( p1 != MAP_FAILED && munmap( p1, len1 ))
    {
        fprintf( stderr, "%s: munmap( p1 ) failed\n", me );

        rc = 1;
    }

    if( me == parent )
    {
        int i1 = -1;
        int i2 = -1;

        if( waitpid( pid, NULL, 0 ) != pid )
        {
            fprintf( stderr, "%s: waitpid( pid ) failed\n", me );

            rc = 1;
        }
        else if( lseek( fd1, 0, SEEK_SET ) == -1
                 || read( fd1, &i1, sizeof( i1 )) != sizeof( i1 )
                 || lseek( fd2, 0, SEEK_SET ) == -1
                 || read( fd2, &i2, sizeof( i2 )) != sizeof( i2 )
                 || i1 != KEY_PARENT || i2 != KEY_CHILD )
                 rc = 1;

        printf("ALL: %s: i1 = %d(%d), i2 = %d(%d)\n",
               rc ? "FAILED" : "PASSED", i1, KEY_PARENT, i2, KEY_CHILD );
    }

    if( fd2 != -1 && close( fd2 ))
    {
        fprintf( stderr, "%s: close( fd2 ) failed\n", me );

        rc = 1;
    }

    if( fd1 != -1 && close( fd1 ))
    {
        fprintf( stderr, "%s: close( fd1 ) failed\n", me );

        rc = 1;
    }

    if( me == parent )
    {
        remove( TESTFILE1 );
        remove( TESTFILE2 );
    }

    return rc;
}
