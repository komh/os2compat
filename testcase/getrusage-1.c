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

int main( void )
{
    struct rusage sru;
    struct rusage eru;
    int i, j, k = 0;

    getrusage( RUSAGE_SELF, &sru );

    /* Consume times */
    for (i = 0; i < 10000; i++)
    {
        for (j = 0; j < 10000; j++)
        {
            k += 10000;
        }
    }

    getrusage( RUSAGE_SELF, &eru );

    printf("User time: from %ld.%lds to %ld.%lds\n",
           sru.ru_utime.tv_sec, sru.ru_utime.tv_usec / 1000,
           eru.ru_utime.tv_sec, eru.ru_utime.tv_usec / 1000 );

    printf("System time: from %ld.%lds to %ld.%lds\n",
           sru.ru_stime.tv_sec, sru.ru_stime.tv_usec / 1000,
           eru.ru_stime.tv_sec, eru.ru_stime.tv_usec / 1000 );

    return 0;
}
