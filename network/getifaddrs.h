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

#ifndef OS2COMPAT_GETIFADDRS_H
#define OS2COMPAT_GETIFADDRS_H

#ifdef __cplusplus
extern "C" {
#endif

struct os2compat_ifaddrs
{
    struct os2compat_ifaddrs   *ifa_next;    /* Next item in list */
    char                       *ifa_name;    /* Name of interface */
    unsigned int                ifa_flags;   /* Flags from SIOCGIFFLAGS */
    struct sockaddr            *ifa_addr;    /* Address of interface */
    struct sockaddr            *ifa_netmask; /* Netmask of interface */
    union
    {
        struct sockaddr *ifu_broadaddr;
                         /* Broadcast address of interface */
        struct sockaddr *ifu_dstaddr;
                         /* Point-to-point destination address */
    } ifa_ifu;
#define ifa_broadaddr   ifa_ifu.ifu_broadaddr
#define ifa_dstaddr     ifa_ifu.ifu_dstaddr
    void                       *ifa_data;    /* Address-specific data */
};

#define ifaddrs os2compat_ifaddrs

int  os2compat_getifaddrs( struct os2compat_ifaddrs **ifap );
void os2compat_freeifaddrs( struct os2compat_ifaddrs *ifa );

#define getifaddrs  os2compat_getifaddrs
#define freeifaddrs os2compat_freeifaddrs

#ifdef __cplusplus
}
#endif

#endif
