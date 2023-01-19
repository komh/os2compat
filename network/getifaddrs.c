/*
 * getifaddrs() and freeifaddrs() implementation for OS/2 kLIBc
 *
 * Copyright (C) 2023 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#include "getifaddrs.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <types.h>
#include <sys/socket.h> /* socket() */
#include <io.h>         /* close() */
#include <sys/ioctl.h>  /* ioctl(), SIOCGIFADDR */
#include <net/if.h>     /* struct ifconf, struct ifreq */

#define ADDRSIZE( addr ) (( addr )->sa_len <  sizeof( struct sockaddr ) ? \
                          sizeof( struct sockaddr ) : ( addr )->sa_len )

int os2compat_getifaddrs( struct os2compat_ifaddrs **ifap )
{
    struct os2compat_ifaddrs *ifa_head = NULL;
    struct ifconf ifconf;
    struct ifreq iflist[ IFMIB_ENTRIES ];
    int s;
    int count;
    int i;
    int ret = -1;

    s = socket( AF_INET, SOCK_RAW, 0 );
    if( s == -1 )
        return -1;

    memset( iflist, 0, sizeof( iflist ));

    ifconf.ifc_len = sizeof( iflist );
    ifconf.ifc_req = iflist;
    ret = ioctl( s, SIOCGIFCONF, &ifconf );
    if( ret == -1)
        goto out;

    ret = -1;

    count = ifconf.ifc_len / sizeof( struct ifreq );
    for( i = 0; i < count; i++ )
    {
        struct ifreq *ifr = iflist + i;

        if( ifr->ifr_addr.sa_family == AF_LINK )
        {
            struct os2compat_ifaddrs *ifa_new;

            /* AF_LINK */
            ifa_new = calloc( sizeof( *ifa_new ), 1 );
            if( ifa_new == NULL )
                goto out_of_memory;

            /* get name of interface */
            ifa_new->ifa_name = strdup( ifr->ifr_name );
            if( ifa_new->ifa_name == NULL )
            {
                os2compat_freeifaddrs( ifa_new );

                goto out_of_memory;
            }

            /* get address of interface */
            ifa_new->ifa_addr = malloc( ADDRSIZE( &ifr->ifr_addr ));
            if( ifa_new->ifa_addr == NULL )
            {
                os2compat_freeifaddrs( ifa_new );

                goto out_of_memory;
            }

            memcpy( ifa_new->ifa_addr, &ifr->ifr_addr,
                    ADDRSIZE( &ifr->ifr_addr ));

            /* get flags */
            if( ioctl( s, SIOCGIFFLAGS, ifr ) == 0 )
                ifa_new->ifa_flags = ifr->ifr_flags;

            /* add to list */
            ifa_new->ifa_next = ifa_head;
            ifa_head = ifa_new;

            /* AF_INET  */
            ifa_new = calloc( sizeof( *ifa_new ), 1 );
            if( ifa_new == NULL )
                goto out_of_memory;

            /* get name of interface */
            ifa_new->ifa_name = strdup( ifa_head->ifa_name );
            if( ifa_new->ifa_name == NULL )
            {
                os2compat_freeifaddrs( ifa_new );

                goto out_of_memory;
            }

            /* get flags */
            ifa_new->ifa_flags = ifa_head->ifa_flags;

            /* get address of interface */
            if( ioctl( s, SIOCGIFADDR, ifr ) == 0 )
            {
                ifa_new->ifa_addr = malloc( ADDRSIZE( &ifr->ifr_addr ));
                if( ifa_new->ifa_addr == NULL )
                {
                    os2compat_freeifaddrs( ifa_new );

                    goto out_of_memory;
                }

                memcpy( ifa_new->ifa_addr, &ifr->ifr_addr,
                        ADDRSIZE( &ifr->ifr_addr ));
            }

            /* get netmask of interface */
            if( ioctl(s, SIOCGIFNETMASK, ifr ) == 0 )
            {
                ifa_new->ifa_netmask = malloc( ADDRSIZE( &ifr->ifr_dstaddr ));
                if( ifa_new->ifa_netmask == NULL )
                {
                    os2compat_freeifaddrs( ifa_new );

                    goto out_of_memory;
                }

                memcpy( ifa_new->ifa_netmask, &ifr->ifr_dstaddr,
                        ADDRSIZE( &ifr->ifr_dstaddr ));
            }

            /* get broadcast address of interface */
            if( ifa_new->ifa_flags & IFF_POINTOPOINT )
            {
                if( ioctl(s, SIOCGIFDSTADDR, ifr ) == 0 )
                {
                    ifa_new->ifa_dstaddr =
                        malloc( ADDRSIZE( &ifr->ifr_dstaddr ));
                    if( ifa_new->ifa_dstaddr == NULL )
                    {
                        os2compat_freeifaddrs( ifa_new );

                        goto out_of_memory;
                    }

                    memcpy( ifa_new->ifa_dstaddr, &ifr->ifr_dstaddr,
                            ADDRSIZE( &ifr->ifr_dstaddr ));
                }
            }
            else /*if( ifa_new->ifa_flags & IFF_BROADCAST )*/
            {
                if( ioctl(s, SIOCGIFBRDADDR, ifr ) == 0 )
                {
                    ifa_new->ifa_broadaddr =
                        malloc( ADDRSIZE( &ifr->ifr_broadaddr ));

                    if( ifa_new->ifa_broadaddr == NULL )
                    {
                        os2compat_freeifaddrs( ifa_new );

                        goto out_of_memory;
                    }

                    memcpy( ifa_new->ifa_broadaddr, &ifr->ifr_broadaddr,
                            ADDRSIZE( &ifr->ifr_broadaddr ));
                }
            }

            /* ifa_data is not supported */
            ifa_new->ifa_data = NULL;

            /* add to list */
            ifa_new->ifa_next = ifa_head;
            ifa_head = ifa_new;
        }
    }

    *ifap = ifa_head;

    ret = 0;

out_of_memory:
    if( ret == -1 )
    {
        os2compat_freeifaddrs( ifa_head );

        errno = ENOMEM;
    }

out:
    close( s );

    return ret;
}

void os2compat_freeifaddrs( struct os2compat_ifaddrs *ifa )
{
    while( ifa )
    {
        struct os2compat_ifaddrs *next = ifa->ifa_next;

        free( ifa->ifa_name );
        free( ifa->ifa_addr );
        free( ifa->ifa_netmask );
        free( ifa->ifa_broadaddr );
        free( ifa->ifa_data );
        free( ifa );

        ifa = next;
    }
}

