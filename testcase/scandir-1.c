/*
 * scandir() and alphasort() test program
 *
 * Copyright (C) 2016-2023 KO Myung-Hun <komh@chollian.net>
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
#include <fcntl.h>

#include "test.h"

#include "scandir.h"

#define TEST_DIR    "scandir-test.dir"
#define TEST_FILE1  "scandir-test1.file"
#define TEST_FILE2  "scandir-test2.file"
#define TEST_FILE3  "scandir-test3.file"

#define EXIT(c) do { rc = ( c ); goto exit_close; } while( 0 )

static void test( int ( *sel )(/* const */ struct dirent * ),
                  int ( *compare )( const /* struct dirent ** */ void *,
                                    const /* struct dirent ** */ void * ),
                  int nfiles, const char *filelist[])
{
    struct dirent **namelist = 0;
    int n;

    n = scandir(".", &namelist, sel, compare );
    if( n != -1 )
    {
        int i;


        for( i = 0; i < n; i++ )
            if( filelist != NULL
                && stricmp( namelist[ i ]->d_name, filelist[ i ] ) != 0 )
                break;

        TEST_EQUAL( i, n );

        free( namelist );
    }

    TEST_EQUAL( n, nfiles );
    printf("\n");
}

static int filter(/* const */ struct dirent *d )
{
    return strcmp( d->d_name, ".") && strcmp( d->d_name, "..");
}

static int compare( const /* struct dirent **d1 */ void *p1,
                    const /* struct dirent **d2 */ void *p2)
{
    return alphasort( p1, p2 ) * -1;
}

int main( void )
{
    const char *filelist1[] = { TEST_FILE3, TEST_FILE2, TEST_FILE1,
                                "..", "." };
    int nfiles1 = sizeof( filelist1 ) / sizeof( filelist1[ 0 ]);
    const char *filelist2[] = { TEST_FILE3, TEST_FILE2, TEST_FILE1 };
    int nfiles2 = sizeof( filelist2 ) / sizeof( filelist2[ 0 ]);
    int fd1 = -1, fd2 = -1, fd3 = -1;
    int rc = 0;


    printf("Testing scandir() and alphasort()...\n");

    if( mkdir( TEST_DIR, 0755 ) == -1 )
    {
        fprintf( stderr, "mkdir() failed!!!\n");

        return 1;
    }

    if( chdir( TEST_DIR ) == -1 )
    {
        fprintf( stderr, "chdir() failed!!!\n");

        rmdir( TEST_DIR );

        return 1;
    }

    fd1 = open( TEST_FILE1, O_RDWR | O_CREAT, S_IREAD | S_IWRITE );
    fd2 = open( TEST_FILE2, O_RDWR | O_CREAT, S_IREAD | S_IWRITE );
    fd3 = open( TEST_FILE3, O_RDWR | O_CREAT, S_IREAD | S_IWRITE );

    if( fd1 == -1 || fd2 == -1 || fd3 == -1 )
    {
        fprintf( stderr, "open() failed!!!\n");

        rc = 1;

        goto exit_close;
    }

    printf("Testing without filter, without compare...\n");
    test( NULL, NULL, nfiles1, NULL );

    printf("Testing with filter, without compare...\n");
    test( filter, NULL, nfiles2, NULL );

    printf("Testing without filter, with compare...\n");
    test( NULL, compare, nfiles1, filelist1 );

    printf("Testing with filter, with compare...\n");
    test( filter, compare, nfiles2, filelist2 );

    printf("All tests PASSED\n");

exit_close:
    close( fd1 );
    close( fd2 );
    close( fd3 );

    remove( TEST_FILE1 );
    remove( TEST_FILE2 );
    remove( TEST_FILE3 );

    chdir("..");
    rmdir( TEST_DIR );

    return rc;
}
