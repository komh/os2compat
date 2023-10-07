/*
 * if_nameindex() family test program
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
#include <string.h>

#include <net/if.h>

#include "test.h"

#include "if_nameindex.h"

int main( void )
{
    struct if_nameindex *nis;
    char ifname[ IFNAMSIZ ];
    int i;

    nis = if_nameindex();

    if( nis )
    {
        for( i = 0; nis[ i ].if_index != 0; i++ )
        {
            printf("Index %d = [%s]\n", nis[ i ].if_index, nis[ i ].if_name );
            TEST_EQUAL( if_nametoindex( nis[ i ].if_name ),
                        nis[ i ].if_index );
            TEST_EQUAL( strcmp( if_indextoname( nis[ i ].if_index, ifname ),
                                nis[ i ].if_name ), 0 );

            printf("\n");
        }

        if_freenameindex( nis );

    }
    else
        perror("if_nameindex()");

    printf("All tests PASSED\n");

    return 0;
}

