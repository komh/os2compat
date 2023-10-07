/*
 * setmode() test program
 *
 * Copyright (C) 2014-2023 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#include <stdio.h>
#include <io.h>
#include <fcntl.h>

#include "test.h"

int test( int fd, const char *name )
{
    setmode( fd, O_BINARY );

    TEST_EQUAL_MSG( isatty( fd )
                    && ( fcntl( fd, F_GETFL ) & O_BINARY ), 0, name );

    return 0;
}

int main( void )
{
    printf("Testing setmode( O_BINARY ) on a tty...\n");

    test( fileno( stdout ), "stdout");
    test( fileno( stderr ), "stderr");

    printf("All tests PASSED\n");

    return 0;
}
