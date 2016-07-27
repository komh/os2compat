/*
 * scandir() and alphasort() test program
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
#include <stdlib.h>
#include <string.h>

#include "scandir.h"

static void test( int ( *sel )(/* const */ struct dirent * ),
                  int ( *compare )( const /* struct dirent ** */ void *,
                                    const /* struct dirent ** */ void * ))
{
    struct dirent **namelist = 0;
    int n;

    n = scandir(".", &namelist, sel, compare );
    if( n != -1 )
    {
        int i;

        for( i = 0; i < n; i++ )
            printf("%d(th): %s\n", i, namelist[ i ]->d_name );

        free( namelist );
    }

    printf("Entries = %d\n", n );
}

static int filter(/* const */ struct dirent *d )
{
    return strcmp( d->d_name, ".") && strcmp( d->d_name, "..");
}

int main( void )
{
    printf("Without filter, Without compare\n");
    test( NULL, NULL );

    printf("With filter, Without compare\n");
    test( filter, NULL );

    printf("Without filter, With compare\n");
    test( NULL, alphasort );

    printf("With filter, With compare\n");
    test( filter, alphasort );

    return 0;
}
