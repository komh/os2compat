/*
 * if_nameindex() family test program
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

#include <net/if.h>

#include "if_nameindex.h"

int main( void )
{
    struct if_nameindex *nis;
    char ifname[ IFNAMSIZ ];
    int i;

    nis = if_nameindex();

    if( nis )
    {
        printf("----- List of interfaces -----\n");
        for( i = 0; nis[ i ].if_index != 0; i++ )
            printf("Index %d = [%s]\n", nis[ i ].if_index, nis[ i ].if_name );

        printf("----- End of list -----\n");
        if_freenameindex( nis );

    }
    else
        perror("if_nameindex()");

    i = if_nametoindex("lan0");
    printf("Index of lan0 = %d\n", i );
    printf("name of index %d = [%s]\n", i, if_indextoname( i, ifname ));

    i = if_nametoindex("lo");
    printf("Index of lo = %d\n", i );
    printf("name of index %d = [%s]\n", i, if_indextoname( i, ifname ));

    return 0;
}

