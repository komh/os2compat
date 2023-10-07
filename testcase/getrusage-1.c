/*
 * getrusage() test program
 *
 * Copyright (C) 2016 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#include <sys/types.h>      /* id_t */
#include <sys/resource.h>

#include <stdio.h>
#include <stdlib.h>

#include "test.h"

#define SLEEP_MS    100

#define TV2US( tv ) (( tv ).tv_sec * 1000000 + ( tv ).tv_usec )

int main( void )
{
    struct rusage sru;
    struct rusage eru;

    printf("Testing getrusage( RUSAGE_SELF )...\n");

    TEST_EQUAL( getrusage( RUSAGE_SELF, &sru ), 0 );

    /* Consume times */
    _sleep2( SLEEP_MS );

    TEST_EQUAL( getrusage( RUSAGE_SELF, &eru ), 0 );

    TEST_BOOL_MSG( TV2US( eru.ru_utime ) - TV2US( sru.ru_utime ) >=
                   SLEEP_MS * 1000, 1, "User time");

    if( sru.ru_stime.tv_sec == 0 && sru.ru_stime.tv_usec == 0
        && eru.ru_stime.tv_sec == 0 && eru.ru_stime.tv_usec == 0 )
        printf("System time not implemented yet\n");
    else
        TEST_BOOL_MSG( TV2US( eru.ru_stime ) - TV2US( sru.ru_stime ) > 0, 1,
                       "System time");

    printf("All tests PASSED\n");

    return 0;
}
