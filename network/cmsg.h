/*
 * Define missing CMSG_xxx macros for control messages for OS/2 kLIBC
 *
 * Copyright (C) 2016 KO Myung-Hun <komh@chollian.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://www.wtfpl.net/ for more details.
 */

#ifndef OS2COMPAT_CMSG_H
#define OS2COMPAT_CMSG_H

#include <sys/socket.h> /* struct cmsghdr */

#ifndef CMSG_ALIGN
#define CMSG_ALIGN( len ) _ALIGN( len )
#endif

#ifndef CMSG_SPACE
#define CMSG_SPACE( len ) ( CMSG_ALIGN( sizeof( struct cmsghdr )) + \
                            CMSG_ALIGN( len ))
#endif

#ifndef CMSG_LEN
#define CMSG_LEN( len ) ( CMSG_ALIGN( sizeof( struct cmsghdr )) + ( len ))
#endif

#endif
