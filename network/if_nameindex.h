/*
 * if_nameindex() family implementation for OS/2 kLIBC
 *
 * Copyright (C) 2016-2021 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#ifndef OS2COMPAT_IF_NAMEINDEX_H
#define OS2COMPAT_IF_NAMEINDEX_H

#include <net/if.h>

#ifdef __cplusplus
extern "C" {
#endif

struct os2compat_if_nameindex
{
    /* Note: if_index may be different from ifmib.iftable.iftIndex */
    unsigned int if_index;
    char        *if_name;
};

struct os2compat_if_nameindex *os2compat_if_nameindex( void );
void os2compat_if_freenameindex( struct os2compat_if_nameindex *ptr );
char *os2compat_if_indextoname( unsigned ifindex, char *ifname );
unsigned os2compat_if_nametoindex( const char *ifname );

#define if_nameindex        os2compat_if_nameindex
#define if_freenameindex    os2compat_if_freenameindex
#define if_indextoname      os2compat_if_indextoname
#define if_nametoindex      os2compat_if_nametoindex

#ifdef __cplusplus
}
#endif

#endif
