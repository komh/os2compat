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

#include <sys/time.h>

#include "test.h"

#define TV2MS( tv ) (( tv ).tv_sec * 1000 + ( tv ).tv_usec / 1000 )
#define TVDIFFMS( etv, stv ) ( TV2MS( etv ) - TV2MS( stv ))

int main( void )
{
    struct rusage sru;
    struct rusage eru;
    struct timeval stv;
    struct timeval etv;
    int i;

    printf("Testing getrusage( RUSAGE_SELF )...\n");

    TEST_EQUAL( getrusage( RUSAGE_SELF, &sru ), 0 );

    gettimeofday( &stv, NULL );

    /* Consume times */
    for( i = 0; i < 10000 * 5; i++ )
        printf("Consuming times... %dth loop\r", i );

    gettimeofday( &etv, NULL );

    TEST_EQUAL( getrusage( RUSAGE_SELF, &eru ), 0 );

    printf("Elapsed time: %5ldms\n", TVDIFFMS( etv, stv ));
    printf("   User time: %5ldms\n", TVDIFFMS( eru.ru_utime, sru.ru_utime ));
    printf(" System time: %5ldms\n", TVDIFFMS( eru.ru_stime, sru.ru_stime ));

    printf("All tests PASSED\n");

    return 0;
}
