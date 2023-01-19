/*
 * getifaddrs() and freeifaddrs() test program
 *
 * Copyright (C) 2023 KO Myung-Hun <komh@chollian.net>
 *
 * Modified from:
 *   https://man7.org/linux/man-pages/man3/getifaddrs.3.html
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#include "getifaddrs.h"

#include <stdio.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_dl.h>

#define INET_NTOA( sa ) inet_ntoa(((( struct sockaddr_in * )( sa ))->\
                                     sin_addr ))

int main( void )
{
    struct ifaddrs *ifaddr;
    int family;

    if( getifaddrs( &ifaddr ) == -1 )
    {
        perror("getifaddrs");
        return 1;
    }

    /* Walk through linked list, maintaining head pointer so we
       can free list later. */

    for( struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next )
    {
        if( ifa->ifa_addr == NULL )
            continue;

        family = ifa->ifa_addr->sa_family;

        /* Display interface name and family (including symbolic
           form of the latter for the common families). */

        printf("%-8s %s (%d) %08x\n",
               ifa->ifa_name,
               ( family == AF_INET) ? "AF_INET" :
               ( family == AF_LINK) ? "AF_LINK" : "???",
               family, ifa->ifa_flags);

        /* For an AF_INET* interface address, display the address. */

        if( family == AF_INET )
        {
            printf("\t\taddress: <%s>\n", INET_NTOA( ifa->ifa_addr ));

            if( ifa->ifa_netmask )
                printf("\t\tnetmask: <%s>\n", INET_NTOA( ifa->ifa_netmask ));

            if( ifa->ifa_dstaddr )
            {
                if( ifa->ifa_flags & IFF_POINTOPOINT )
                    printf("\t\tdestination: <%s>\n",
                           INET_NTOA( ifa->ifa_dstaddr ));
                else
                    printf("\t\tbroadcast: <%s>\n",
                           INET_NTOA( ifa->ifa_broadaddr ));
            }

            printf("\t\tdata: <%p>\n", ifa->ifa_data );
        }
        else if( family == AF_LINK )
        {
            char *mac = LLADDR(( struct sockaddr_dl * )ifa->ifa_addr );

            printf("\t\tMAC address: <%02X:%02X:%02X:%02X:%02X:%02X>\n",
                   mac[ 0 ], mac[ 1 ], mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ]);
        }

        printf("\n");
    }

    freeifaddrs( ifaddr );

    return 0;
}
