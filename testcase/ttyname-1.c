/*
 * ttyname() test program
 *
 * Copyright (C) 2023 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "test.h"

struct testparam
{
    const char *devname;
    int oflags;
    const char *expected;
};

int main( void )
{
    struct testparam tests[] = {
        {"CON",     O_RDWR,     "/dev/con"},
        {"KBD$",    O_RDONLY,   "/dev/con"},
        {"SCREEN$", O_WRONLY,   "/dev/con"},
        {"NUL",     O_RDWR,     "/dev/nul"},
        {"CLOCK$",  O_RDWR,     "/dev/clock$"},
        { NULL,     0,          NULL },
    };

    struct testparam *test;
    int fd;
    int pipes[ 2 ];

    for( test = tests; test->devname; test++ )
    {
        printf("Testing [%s]...\n", test->devname );

        TEST_BOOL(( fd = open( test->devname, test->oflags )) != -1, 1 );
        TEST_EQUAL( strcmp( ttyname( fd ), test->expected ), 0 );
        close( fd );

        printf("\n");
    }

    printf("Testing pipes...\n");

    TEST_EQUAL( pipe( pipes ), 0 );
    TEST_BOOL( ttyname( pipes[ 0 ]) == NULL, 1 );
    close( pipes[ 0 ]);
    close( pipes[ 1 ]);

    printf("\n");

    printf("All test PASSED\n");
    return 0;
}
