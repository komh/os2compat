/*
 * semaphore test program
 *
 * Copyright (C) 2016-2023 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#define INCL_DOS
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>

#include "test.h"

#include "semaphore.h"

void thread( void *arg )
{
    sem_t *sem = arg;

    printf("Thread %d started!!!\n", _gettid());

    TEST_EQUAL( sem_wait( sem ), 0 );

    printf("Thread %d ended!!!\n", _gettid());
}

int main( void )
{
    sem_t sem;

    TID t1, t2;

    printf("Testing semaphores...\n");

    TEST_EQUAL( sem_init( &sem, 0, 0 ), 0 );

    TEST_EQUAL( sem_post( &sem ), 0 );
    TEST_EQUAL( sem_post( &sem ), 0 );

    t1 = _beginthread( thread, NULL, 1024 * 1024, &sem );
    t2 = _beginthread( thread, NULL, 1024 * 1024, &sem );

    DosWaitThread( &t1, DCWW_WAIT );
    DosWaitThread( &t2, DCWW_WAIT );

    TEST_EQUAL( sem_destroy( &sem ), 0 );

    printf("All tests PASSED\n");

    return 0;
}
