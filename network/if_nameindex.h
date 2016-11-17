/*
 * if_nameindex() family implementation for OS/2 kLIBC
 *
 * Copyright (C) 2016 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#ifndef OS2COMPAT_IF_NAMEINDEX_H
#define OS2COMPAT_IF_NAMEINDEX_H

#ifdef __cplusplus
extern "C" {
#endif

struct if_nameindex
{
    /* Note: if_index may be different from ifmib.iftable.iftIndex */
    unsigned int if_index;
    char        *if_name;
};

struct if_nameindex *if_nameindex( void );
void if_freenameindex( struct if_nameindex *ptr );
char *if_indextoname( unsigned ifindex, char *ifname );
unsigned if_nametoindex( const char *ifname );

#ifdef __cplusplus
}
#endif

#endif
